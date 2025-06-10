#ifndef CALCSTAB_H
#define CALCSTAB_H

#include "saveable_widget.h"
#include "utils/cm_ctors.h"

#include <QPointer>
#include <QWidget>

namespace Ui {
class CalcsTab;
}

class DelayedSignal;

class CalcsTab : public QWidget
{
    Q_OBJECT
  public:
    NO_COPYMOVE(CalcsTab);
    CalcsTab(QWidget *parent = nullptr);
    ~CalcsTab();

  protected:
    void changeEvent(QEvent *e);

  private:
    Ui::CalcsTab *ui;
    QPointer<DelayedSignal> delayedStart;

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

#endif // CALCSTAB_H
