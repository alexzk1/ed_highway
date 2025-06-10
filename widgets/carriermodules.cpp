#include "carriermodules.h"

#include "widget_helpers.h"

#include "ui_carriermodules.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QPointer>
#include <QVBoxLayout>

CarrierModules::CarrierModules(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::carriermodules)
{
    ui->setupUi(this);
    buildGui();
    showTotal();
}

CarrierModules::~CarrierModules()
{
    delete ui;
}

const CarrierModuleInfo &CarrierModules::getTotal() const
{
    return total;
}

void CarrierModules::changeEvent(QEvent *e)
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

void CarrierModules::buildGui()
{
    total = {"", 0, 5000000, 5000000, 0};
    cleanAllChildren(ui->scrollArea);
    const auto container = new QGroupBox(ui->scrollArea);
    container->setTitle("");
    const auto containerLayout = new QVBoxLayout();
    container->setLayout(containerLayout);
    ui->scrollArea->setWidget(container);

    // temporary in-ram storing of checkboxes states, do not save on disk
    static std::vector<bool> allChecks;
    size_t index = 0;

    const auto vptrs = addContainerAsCheckboxes(
      containerLayout, getCarrierModulesInfoList(),
      [&](const QPointer<QCheckBox> &checkbox, const CarrierModuleInfo &mi) {
          connect(checkbox, &QCheckBox::toggled, this, [this, checkbox, &mi, index](bool set) {
              if (checkbox)
              {
                  if (set)
                      total += mi;
                  else
                      total -= mi;
                  allChecks[index] = set;
              }
              showTotal();
          });
          ++index;
      });

    // loading checkboxes from ram
    if (allChecks.size() != vptrs.size())
        allChecks.resize(vptrs.size(), false);
    else
    {
        for (size_t i = 0, sz = vptrs.size(); i < sz; ++i)
            vptrs[i]->setChecked(allChecks[i]);
    }
}

void CarrierModules::showTotal()
{
    ui->label->setText(QString("Purchase: %1; Upkeep: %2; Paused Upkeep: %3;\nCargo Use by Mods: "
                               "%4; Cargo Free: %5\nSavings on pausing per week: %6")
                         .arg(spaced_1000s(total.purchase))
                         .arg(spaced_1000s(total.full_upkeep))
                         .arg(spaced_1000s(total.paused_upkeep))
                         .arg(spaced_1000s(total.cargo_use))
                         .arg(spaced_1000s(max_carrier_cargo() - total.cargo_use))
                         .arg(spaced_1000s(total.full_upkeep - total.paused_upkeep)));
}
