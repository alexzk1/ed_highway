#include "calcstab.h"

#include "carriermodulesdialog.h"
#include "carriers_info.h"
#include "config_ui/globalsettings.h"
#include "delayedsignal.h"
#include "edsmwrapper.h"
#include "point.h"
#include "spanshsyssuggest.h"
#include "utils/exec_exit.h"
#include "widget_helpers.h"

#include "ui_calcstab.h"

#include <QButtonGroup>
#include <QOverload>
#include <QPointer>
#include <QRadioButton>
#include <QSpinBox>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

constexpr static int delay_ms = 2000;
const static QString settingsGroup = "CalcsTabSettings";

template <class T = float>
inline T uniformRandom(T low = static_cast<T>(0.), T hi = static_cast<T>(1.))
{
    static_assert(std::is_floating_point<T>::value, "T must be floating point one.");
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return std::uniform_real_distribution<T>(low, hi)(gen);
}

CalcsTab::CalcsTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalcsTab),
    delayedStart(new DelayedSignal(this))
{
    ui->setupUi(this);
    connect(delayedStart, &DelayedSignal::delayedSignal, this, &CalcsTab::calcCarrierFuel);

    // Attaching change event to spin boxes.
    {
        const auto setup_spin = [this](QSpinBox *ptr) {
            ptr->setMaximum(carrierSelected.get_carrier_mass_limit());
            connect(ptr, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
                updateCargoToMax();
                delayedStart->sourceSignal(delay_ms);
            });
        };

        const std::vector<QSpinBox *> spins = {
          ui->sbCargo, ui->sbModules, ui->sbFuel, ui->sbEachNth, ui->sbTonnes,
        };

        for (const auto &s : spins)
        {
            setup_spin(s);
        }
    }

    // Attaching change event to radios.
    {
        const auto setup_radio = [this](QRadioButton *rb) {
            connect(rb, &QRadioButton::toggled, this, [this](bool enabled) {
                if (enabled)
                {
                    delayedStart->sourceSignal(delay_ms / 4);
                }
            });
        };

        const std::vector<QRadioButton *> radios = {
          ui->rbOnEmpty, ui->rbTankFull, ui->rbRandom,

          ui->rbD470,    ui->rbD495,     ui->rbD500,
        };

        for (const auto &r : radios)
        {
            setup_radio(r);
        }
    }

    // Attaching carrier selection.
    {
        auto group = new QButtonGroup(this);
        group->addButton(ui->rbPersonal);
        group->addButton(ui->rbSquadron);

        const auto setup_radio = [this](const QPointer<QRadioButton> &rb) {
            connect(rb, &QRadioButton::toggled, this, [this, rb](bool enabled) {
                if (enabled && rb)
                {
                    if (rb == ui->rbPersonal)
                    {
                        carrierSelected = CarrierJumpCalculator(ECarrierType::PersonalCarrier);
                    }
                    else
                    {
                        carrierSelected = CarrierJumpCalculator(ECarrierType::SquadronCarrier);
                    }

                    const auto maxConsume = carrierSelected.compute_fuel_use(
                      carrierSelected.get_carrier_mass_limit(), carrierSelected.carrier_max_jump());

                    ui->lblMaxFuel->setText(
                      QString(tr("Max fuel per jump: %1")).arg(maxConsume.value_or(0)));

                    delayedStart->sourceSignal(delay_ms / 4);
                }
            });
        };

        setup_radio(ui->rbPersonal);
        setup_radio(ui->rbSquadron);

        ui->rbPersonal->setChecked(true);
    }

    new SpanshSysSuggest(ui->leSys1);
    new SpanshSysSuggest(ui->leSys2);

    loadSettings();
    on_leSys1_textChanged("");

    // Update maximal cargo.
    const auto maxInput =
      std::max(CarrierJumpCalculator(ECarrierType::PersonalCarrier).get_carrier_mass_limit(),
               CarrierJumpCalculator(ECarrierType::SquadronCarrier).get_carrier_mass_limit());
    ui->sbCargo->setMaximum(maxInput);
    ui->sbFuel->setMaximum(maxInput);
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

    const auto read_mass = [&settings](const QString &v, QSpinBox *s) {
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
        const auto used = static_cast<std::uint32_t>(ui->sbModules->value() + ui->sbFuel->value());
        ui->sbCargo->setValue(carrierSelected.get_carrier_mass_limit() - used);
    }
}

void CalcsTab::calcCarrierFuel()
{
    constexpr static int kConsider_infinite_travel_with_jumps = 20000;

    const std::uint32_t mods = ui->sbModules->value();
    const std::uint32_t carg = ui->sbCargo->value();
    const std::uint32_t fuel = ui->sbFuel->value();
    const std::uint32_t refuel_each_nth = ui->sbEachNth->value();
    const auto refuel_random_mine = ui->sbTonnes->value();

    const bool random_mine = ui->rbRandom->isChecked();
    const bool keep_full = random_mine || ui->rbTankFull->isChecked();
    const bool refuel_empty = ui->rbOnEmpty->isChecked();

    setTritiumStepping();

    ui->lblMass->setText(
      tr("Non-fuel mass of carrier: %1(t). This should be same as (total mass - tritium mass).")
        .arg(mods + carg));

    const auto mass_limit = carrierSelected.get_carrier_mass_limit();
    if (mods + carg + fuel > mass_limit)
    {
        ui->lblResult->setText(
          tr("Total mass is bigger then maximum cargo %1(t).").arg(mass_limit));
    }
    else
    {
        int jumps_till_recharge = 0;
        bool infinite = false;
        bool error_in_fuel_compute = false;

        // rbD500 default value
        const auto maxJump = carrierSelected.carrier_max_jump();
        float jump_distance = maxJump;

        const auto update_distance_for_range = [&jump_distance, maxJump](const float range) {
            jump_distance =
              std::fmin(maxJump, myrnd::uniformRandom<float>(0.f, maxJump - range) + range);
        };

        const auto update_distance = [this, &update_distance_for_range]() {
            if (ui->rbD470->isChecked())
            {
                update_distance_for_range(470.f);
            }

            if (ui->rbD495->isChecked())
            {
                update_distance_for_range(495.f);
            }
        };

        float distance = 0;
        const auto jump = [&distance, &update_distance, &jump_distance]() {
            distance += jump_distance;
            update_distance();
        };

        bool jumps_till_recharge_once = true;

        update_distance();
        for (std::uint32_t current_fuel = fuel, current_used = 0u,
                           tank = carrierSelected.carrier_tank_size(), njump = 1u;
             current_fuel + tank > current_used; ++njump)
        {
            if (error_in_fuel_compute)
            {
                break;
            }
            if (njump > kConsider_infinite_travel_with_jumps)
            {
                infinite = true;
                break;
            }

            for (std::uint32_t r = 0; r < 2; ++r)
            {
                const std::uint32_t total = current_fuel + tank;
                if (total < carrierSelected.carrier_tank_size())
                {
                    current_fuel = 0;
                    tank = total;
                }
                const auto computed_use =
                  carrierSelected.compute_fuel_use(current_fuel + carg + mods, jump_distance);
                if (!computed_use.has_value())
                {
                    error_in_fuel_compute = true;
                    break;
                }
                current_used = *computed_use;

                if (random_mine && (njump % refuel_each_nth == 0))
                {
                    if (current_used >= refuel_random_mine)
                    {
                        current_used -= refuel_random_mine;
                    }
                    else
                    {
                        const auto extra = refuel_random_mine - current_used;
                        current_used = 0;
                        const auto target = mods + carg + current_fuel + extra;

                        if (target > mass_limit)
                        {
                            current_fuel = mass_limit - mods - carg;
                        }
                        else
                        {
                            current_fuel += extra;
                        }
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
                    {
                        break;
                    }

                    if (tank < current_used)
                    {
                        const auto delta =
                          std::min(carrierSelected.carrier_tank_size() - tank, current_fuel);
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
        {
            ui->lblResult->setText(tr("Infinite travel."));
        }
        else if (error_in_fuel_compute)
        {
            ui->lblResult->setText(tr("Error happened computing jump distances."));
        }
        else
        {
            // todo: make setting for this
            // if true - it will be always spaces, if false - it will try to use separator by
            // current locale like comma and add spaces only if none
            constexpr static bool force_always_spaces = true;

            if (refuel_empty)
            {
                ui->lblResult->setText(
                  tr("Max distance: %1 (ly). With return same way: %2 (ly). Jumps till refuel: %3")
                    .arg(spaced_1000s(distance, force_always_spaces))
                    .arg(spaced_1000s(distance / 2.f, force_always_spaces))
                    .arg(spaced_1000s(jumps_till_recharge, force_always_spaces)));
            }
            else
            {
                ui->lblResult->setText(tr("Max distance: %1 (ly). With return same way: %2 (ly).")
                                         .arg(spaced_1000s(distance, force_always_spaces))
                                         .arg(spaced_1000s(distance / 2.f, force_always_spaces)));
            }
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
    {
        ui->lblDistRes->setText(tr("Both systems must be non-empty."));
    }
    else
    {
        exec_on_exit ex([this]() {
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
        catch (std::exception &e)
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
    {
        ui->sbModules->setValue(dlg.getTotal().cargo_use);
    }
}
