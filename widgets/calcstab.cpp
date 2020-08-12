#include "calcstab.h"
#include "ui_calcstab.h"
#include "delayedsignal.h"
#include <math.h>
#include <QSpinBox>

constexpr static int max_carrier_cargo = 25000;
const static QString settingsGroup = "CalcsTabSettings";

CalcsTab::CalcsTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalcsTab),
    delayedStart(new DelayedSignal(this))
{
    ui->setupUi(this);
    connect(delayedStart, &DelayedSignal::delayedSignal, this, &CalcsTab::calcCarrierFuel);

    const auto setup_spin = [this](QSpinBox * ptr)
    {
        ptr->setMaximum(max_carrier_cargo);
        connect(ptr, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int)
        {
            delayedStart->sourceSignal(2000);
        });
    };

    setup_spin(ui->sbCargo);
    setup_spin(ui->sbModules);
    setup_spin(ui->sbFuel);

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
    ui->lblMass->setText(tr("Non-fuel mass of carrier: %1(t)").arg(mods + carg));

    if (mods + carg + fuel > max_carrier_cargo)
        ui->lblResult->setText(tr("Total mass is bigger then maximum cargo %1(t).").arg(max_carrier_cargo));
    else
    {
        constexpr static int jump_distance = 500;
        constexpr static float max_cargo = static_cast<float>(max_carrier_cargo);
        constexpr static float jd_mul = jump_distance / 4.f;

        int distance = 0;
        for (int current_fuel = fuel, current_used = 0; current_fuel > current_used; current_fuel -= current_used)
        {
            //=ROUND(10 + 500 * (1 + (D2 + $C$2)/25000) / 4)
            current_used = round(10 + jd_mul * (1 + (current_fuel + carg + mods) / max_cargo));
            distance += jump_distance;
        }

        ui->lblResult->setText(tr("Max distance: %1 (ly). With return same way: %2 (ly).").arg(distance).arg(distance / 2));
    }
}
