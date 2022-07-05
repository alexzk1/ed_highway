#include "calcstab.h"
#include "ui_calcstab.h"
#include "delayedsignal.h"
#include <math.h>
#include <random>
#include <iostream>
#include <QSpinBox>
#include "config_ui/globalsettings.h"
#include "spanshsyssuggest.h"
#include "edsmwrapper.h"
#include "point.h"
#include "utils/exec_exit.h"
#include "carriers_info.h"
#include "widget_helpers.h"
#include "carriermodulesdialog.h"

constexpr static int delay_ms = 2000;
const static QString settingsGroup = "CalcsTabSettings";

template <class T = float>
inline T uniformRandom(T low = static_cast<T>(0.), T hi = static_cast<T>(1.))
{
    static_assert (std::is_floating_point<T>::value, "T must be floating point one.");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<T> dis(low, hi);
    return dis(gen);
}

CalcsTab::CalcsTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalcsTab),
    delayedStart(new DelayedSignal(this))
{
    ui->setupUi(this);
    connect(delayedStart, &DelayedSignal::delayedSignal, this, &CalcsTab::calcCarrierFuel);

    //attaching change event to spin boxes
    {
        const auto setup_spin = [this](QSpinBox * ptr)
        {
            ptr->setMaximum(max_carrier_cargo());
            connect(ptr, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int)
            {
                updateCargoToMax();
                delayedStart->sourceSignal(delay_ms);
            });
        };

        const std::vector<QSpinBox*> spins =
        {
            ui->sbCargo,
            ui->sbModules,
            ui->sbFuel,
            ui->sbEachNth,
            ui->sbTonnes,
        };

        for (const auto& s : spins)
            setup_spin(s);
    }

    //attaching change event to radios
    {
        const auto setup_radio = [this](QRadioButton * rb)
        {
            connect(rb, &QRadioButton::toggled, this, [this](bool)
            {
                delayedStart->sourceSignal(delay_ms / 4);
            });
        };

        const std::vector<QRadioButton*> radios =
        {
            ui->rbOnEmpty,
            ui->rbTankFull,
            ui->rbRandom,

            ui->rbD470,
            ui->rbD495,
            ui->rbD500,
        };

        for (const auto& r : radios)
            setup_radio(r);
    }

    new SpanshSysSuggest(ui->leSys1);
    new SpanshSysSuggest(ui->leSys2);

    loadSettings();
    on_leSys1_textChanged("");
}

CalcsTab::~CalcsTab()
{
    saveSettings();
    delete ui;
}

void CalcsTab::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void CalcsTab::saveSettings()
{
    QSettings settings;
    settings.beginGroup(settingsGroup);
    settings.setValue(QStringLiteral("mass_mods"), ui->sbModules->value());
    settings.setValue(QStringLiteral("mass_cargo"), ui->sbCargo->value());
    settings.setValue(QStringLiteral("mass_fuel"), ui->sbFuel->value());
    settings.setValue(QStringLiteral("mass_nth"), ui->sbEachNth->value());
    settings.setValue(QStringLiteral("mass_tonnes"), ui->sbTonnes->value());

    settings.setValue(QStringLiteral("sys_dist1"), ui->leSys1->text());
    settings.setValue(QStringLiteral("sys_dist2"), ui->leSys2->text());

    settings.endGroup();
}

void CalcsTab::loadSettings()
{
    setTritiumStepping();
    QSettings settings;
    settings.beginGroup(settingsGroup);


    const auto read_mass = [ &settings](const QString & v, QSpinBox * s)
    {
        const int def = s->value();
        const int val = settings.value(v, def).toInt();
        s->setValue(val);
    };
    read_mass(QStringLiteral("mass_mods"), ui->sbModules);
    read_mass(QStringLiteral("mass_cargo"), ui->sbCargo);
    read_mass(QStringLiteral("mass_fuel"), ui->sbFuel);
    read_mass(QStringLiteral("mass_nth"), ui->sbEachNth);
    read_mass(QStringLiteral("mass_tonnes"), ui->sbTonnes);

    ui->leSys1->setText(settings.value(QStringLiteral("sys_dist1"), "Sol").toString());
    ui->leSys2->setText(settings.value(QStringLiteral("sys_dist2"), "").toString());

    settings.endGroup();
}

void CalcsTab::updateCargoToMax()
{
    if (ui->cbKeepCargo->isChecked())
    {
        const auto used = ui->sbModules->value() + ui->sbFuel->value();
        ui->sbCargo->setValue(max_carrier_cargo() - used);
    }
}

void CalcsTab::calcCarrierFuel()
{
    const auto mods = ui->sbModules->value();
    const auto carg = ui->sbCargo->value();
    const auto fuel = ui->sbFuel->value();
    const auto refuel_each_nth = ui->sbEachNth->value();
    const auto refuel_random_mine = ui->sbTonnes->value();

    const bool random_mine  = ui->rbRandom->isChecked();
    const bool keep_full    = random_mine || ui->rbTankFull->isChecked();
    const bool refuel_empty = ui->rbOnEmpty->isChecked();

    setTritiumStepping();

    ui->lblMass->setText(tr("Non-fuel mass of carrier: %1(t). This should be same as (total mass - tritium mass).").arg(mods + carg));

    if (mods + carg + fuel > max_carrier_cargo())
        ui->lblResult->setText(tr("Total mass is bigger then maximum cargo %1(t).").arg(max_carrier_cargo()));
    else
    {
        constexpr static int consider_infinite_travel_with_jumps  = 20000;

        constexpr static float max_cargo = static_cast<float>(max_carrier_cargo());
        constexpr static float minimum_jump_cost = 5.f;
        const static float jd_mul = 1.f / 8.f;

        int jumps_till_recharge = 0;
        bool infinite = false;

        //rbD500 default value
        float jump_distance = carrier_max_jump();

        const auto update_distance_for_range = [&jump_distance](float range)
        {
            jump_distance = std::fmax(carrier_max_jump(), myrnd::uniformRandom<float>(0.f, carrier_max_jump() - range) + range);
        };

        const auto update_distance = [this, &update_distance_for_range]()
        {
            if (ui->rbD470->isChecked())
                update_distance_for_range(470.f);

            if (ui->rbD495->isChecked())
                update_distance_for_range(495.f);

        };

        float distance = 0;
        const auto jump = [&distance, &update_distance, &jump_distance]()
        {
            distance += jump_distance;
            update_distance();
        };


        bool jumps_till_recharge_once = true;

        update_distance();
        for (int current_fuel = fuel, current_used = 0, tank = carrier_tank_size(), njump = 1;
                current_fuel + tank > current_used; ++njump)
        {
            if (njump > consider_infinite_travel_with_jumps)
            {
                infinite = true;
                break;
            }

            for (int r = 0; r < 2; ++r)
            {
                const int total = current_fuel + tank;
                if (total < carrier_tank_size())
                {
                    current_fuel = 0;
                    tank = total;
                }
                current_used = round(minimum_jump_cost + jump_distance * jd_mul * (1.f + static_cast<float>(current_fuel + carg + mods) / max_cargo));

                if (random_mine && (njump % refuel_each_nth == 0))
                {
                    if (current_used >= refuel_random_mine)
                        current_used -= refuel_random_mine;
                    else
                    {
                        const auto extra = refuel_random_mine - current_used;
                        current_used = 0;
                        const auto target = mods + carg + current_fuel + extra;
                        if (target > max_carrier_cargo())
                            current_fuel = max_carrier_cargo() - mods - carg;
                        else
                            current_fuel += extra;
                    }
                }
                if (keep_full)
                {

                    if (current_fuel > current_used)
                    {
                        current_fuel -= current_used;
                        jump();
                    }
                    else
                    {
                        if (tank > current_used)
                        {
                            tank -= current_used;
                            jump();
                        }
                    }
                    break;
                }

                if (refuel_empty)
                {
                    if (current_used > total)
                        break;

                    if (tank < current_used)
                    {
                        const int delta = std::min((int)carrier_tank_size() - tank, current_fuel);
                        tank += delta;
                        current_fuel -= delta;
                        jumps_till_recharge_once = false;
                    }
                    else
                    {
                        tank -= current_used;
                        if (jumps_till_recharge_once)
                            ++jumps_till_recharge;
                        jump();
                        break;
                    }
                }
            }
        }

        if (infinite)
            ui->lblResult->setText(tr("Infinite travel."));
        else
        {
            //todo: make setting for this
            //if true - it will be always spaces, if false - it will try to use separator by current locale like comma and add spaces only if none
            constexpr static bool force_always_spaces = true;

            if (refuel_empty)
                ui->lblResult->setText(tr("Max distance: %1 (ly). With return same way: %2 (ly). Jumps till refuel: %3")
                                       .arg(spaced_1000s(distance, force_always_spaces))
                                       .arg(spaced_1000s(distance / 2.f, force_always_spaces))
                                       .arg(spaced_1000s(jumps_till_recharge, force_always_spaces)));
            else
                ui->lblResult->setText(tr("Max distance: %1 (ly). With return same way: %2 (ly).")
                                       .arg(spaced_1000s(distance, force_always_spaces))
                                       .arg(spaced_1000s(distance / 2.f, force_always_spaces)));
        }
    }
}

void CalcsTab::setTritiumStepping()
{
    const int val = StaticSettingsMap::getGlobalSetts().readInt("04_Int_tritiumstep");
    ui->sbFuel->setSingleStep(val);
}

void CalcsTab::on_distCalc_clicked()
{
    if (ui->leSys1->text().isEmpty() || ui->leSys2->text().isEmpty())
        ui->lblDistRes->setText(tr("Both systems must be non-empty."));
    else
    {
        exec_onexit ex([this]()
        {
            ui->distCalc->setEnabled(true);
        });
        (void)ex;
        ui->distCalc->setEnabled(false);

        try
        {
            const auto j1 = EDSMWrapper::requestSysInfo(ui->leSys1->text());
            const auto j2 = EDSMWrapper::requestSysInfo(ui->leSys2->text());
            const auto dist = Point::fromJson(j1).distance(Point::fromJson(j2));
            ui->lblDistRes->setText(tr("Distance between systems is %1 LY.").arg(dist));
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            ui->lblDistRes->setText(tr("Can't calculate distance. Possible unknown system."));
        }
    }
}

void CalcsTab::on_leSys1_textChanged(const QString &arg1)
{
    (void)arg1;
    ui->lblDistRes->setText(tr("Press button to calc."));
}

void CalcsTab::on_leSys2_textChanged(const QString &arg1)
{
    on_leSys1_textChanged(arg1);
}

void CalcsTab::on_cbKeepCargo_stateChanged(int arg1)
{
    const bool checked = arg1;
    ui->sbCargo->setEnabled(!checked);
    updateCargoToMax();
    delayedStart->sourceSignal(delay_ms);
}

void CalcsTab::on_btnCarMods_clicked()
{
    CarrierModulesDialog dlg(false, this);
    if (QDialog::DialogCode::Accepted == dlg.exec())
        ui->sbModules->setValue(dlg.getTotal().cargo_use);
}

