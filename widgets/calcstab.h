#pragma once

#include "carriers_info.h"
#include "saveable_widget.h"
#include "utils/cm_ctors.h"

#include <QObject>
#include <QPointer>
#include <QWidget>

#include <cstdint>

namespace Ui {
class CalcsTab;
}

class DelayedSignal;

class CalcsTab : public QWidget
{
    Q_OBJECT
  public:
    NO_COPYMOVE(CalcsTab);
    explicit CalcsTab(QWidget *parent = nullptr);
    ~CalcsTab() override;

  protected:
    void changeEvent(QEvent *e) override;

  private:
    Ui::CalcsTab *ui;
    QPointer<DelayedSignal> delayedStart;
    CarrierJumpCalculator carrierSelected{ECarrierType::PersonalCarrier};

    void saveSettings();
    void loadSettings();
    void updateCargoToMax();
  private slots:
    void calcCarrierFuel();
    void setTritiumStepping();
    void on_distCalc_clicked();
    void on_leSys1_textChanged(const QString &arg1);
    void on_leSys2_textChanged(const QString &arg1);
    void on_cbKeepCargo_stateChanged(int arg1);
    void on_btnCarMods_clicked();
};
