#ifndef CARRIERMODULESDIALOG_H
#define CARRIERMODULESDIALOG_H

#include <QDialog>
#include "carriers_info.h"

namespace Ui
{
    class CarrierModulesDialog;
}

class CarrierModulesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CarrierModulesDialog(bool ok_only, QWidget *parent = nullptr);
    ~CarrierModulesDialog();
    const CarrierModuleInfo& getTotal() const;
protected:
    void changeEvent(QEvent *e);

private:
    Ui::CarrierModulesDialog *ui;
};

#endif // CARRIERMODULESDIALOG_H
