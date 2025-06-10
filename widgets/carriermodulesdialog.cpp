#include "carriermodulesdialog.h"

#include "ui_carriermodulesdialog.h"

CarrierModulesDialog::CarrierModulesDialog(bool ok_only, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CarrierModulesDialog)
{
    ui->setupUi(this);

    if (ok_only)
        ui->buttonBox->setStandardButtons({QDialogButtonBox::Ok});
}

CarrierModulesDialog::~CarrierModulesDialog()
{
    delete ui;
}

const CarrierModuleInfo &CarrierModulesDialog::getTotal() const
{
    return ui->widget->getTotal();
}

void CarrierModulesDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
