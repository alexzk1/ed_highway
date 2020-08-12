#include "calcstab.h"
#include "ui_calcstab.h"
#include "delayedsignal.h"
#include <math.h>
#include <QSpinBox>
#include "config_ui/globalsettings.h"

constexpr static int max_carrier_cargo = 25000;
constexpr static int carrier_tank_size = 1000;

const static QString settingsGroup = "CalcsTabSettings";

CalcsTab::CalcsTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalcsTab),
    delayedStart(new DelayedSignal(this))
{
    constexpr static int delay_ms = 2000;
    ui->setupUi(this);
    connect(delayedStart, &DelayedSignal::delayedSignal, this, &CalcsTab::calcCarrierFuel);

    const auto setup_spin = [this](QSpinBox * ptr)
    {
        ptr->setMaximum(max_carrier_cargo);
        connect(ptr, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int)
        {
            delayedStart->sourceSignal(delay_ms);
        });
    };

    const auto setup_radio = [this](QRadioButton * rb)
    {
        connect(rb, &QRadioButton::toggled, this, [this](bool)
        {
            delayedStart->sourceSignal(delay_ms / 4);
        });
    };

    setup_spin(ui->sbCargo);
    setup_spin(ui->sbModules);
    setup_spin(ui->sbFuel);

    setup_radio(ui->rbOnEmpty);
    setup_radio(ui->rbTankFull);

    loadSettings();
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

    settings.endGroup();
}

void CalcsTab::loadSettings()
{
    setTritiumStepping();
    QSettings settings;
    settings.beginGroup(settingsGroup);


    const auto read_mass = [this, &settings](const QString & v, QSpinBox * s)
    {
        const int def = s->value();
        const int val = settings.value(v, def).toInt();
        s->setValue(val);
    };
    read_mass(QStringLiteral("mass_mods"), ui->sbModules);
    read_mass(QStringLiteral("mass_cargo"), ui->sbCargo);
    read_mass(QStringLiteral("mass_fuel"), ui->sbFuel);

    settings.endGroup();
}

void CalcsTab::calcCarrierFuel()
{
    const auto mods = ui->sbModules->value();
    const auto carg = ui->sbCargo->value();
    const auto fuel = ui->sbFuel->value();

    const bool keep_full    = ui->rbTankFull->isChecked();
    const bool refuel_empty = ui->rbOnEmpty->isChecked();

    setTritiumStepping();

    ui->lblMass->setText(tr("Non-fuel mass of carrier: %1(t). This should be same as (total mass - tritium mass).").arg(mods + carg));

    if (mods + carg + fuel > max_carrier_cargo)
        ui->lblResult->setText(tr("Total mass is bigger then maximum cargo %1(t).").arg(max_carrier_cargo));
    else
    {
        constexpr static int jump_distance = 500;
        constexpr static float max_cargo = static_cast<float>(max_carrier_cargo);
        constexpr static float jd_mul = jump_distance / 4.f;

        int distance = 0;

        const auto jump = [&distance]()
        {
            distance += jump_distance;
        };

        for (int current_fuel = fuel, current_used = 0, tank = carrier_tank_size; current_fuel + tank > current_used;)
        {
            for (int r = 0; r < 2; ++r)
            {
                const int total = current_fuel + tank;
                if (total < carrier_tank_size)
                {
                    current_fuel = 0;
                    tank = total;
                }
                current_used = round(10 + jd_mul * (1 + (current_fuel + carg + mods) / max_cargo));

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
                        const int delta = std::min(carrier_tank_size - tank, current_fuel);
                        tank += delta;
                        current_fuel -= delta;
                    }
                    else
                    {
                        tank -= current_used;
                        jump();
                        break;
                    }
                }
            }
        }

        ui->lblResult->setText(tr("Max distance: %1 (ly). With return same way: %2 (ly).").arg(distance).arg(distance / 2));
    }
}

void CalcsTab::setTritiumStepping()
{
    const int val = StaticSettingsMap::getGlobalSetts().readInt("04_Int_tritiumstep");
    ui->sbFuel->setSingleStep(val);
}
