#include "edsmstarclassselector.h"
#include "ui_edsmstarclassselector.h"
#include "widget_helpers.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPointer>

EDSMStarClassSelector::EDSMStarClassSelector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EDSMStarClassSelector)
{
    ui->setupUi(this);
    buildGui(true);
}

EDSMStarClassSelector::~EDSMStarClassSelector()
{
    delete ui;
}

void EDSMStarClassSelector::changeEvent(QEvent *e)
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

void EDSMStarClassSelector::buildGui(bool multi_select)
{
    //FIXME: implement radio-group too, so can be selected single option (and all needed to switch modes)
    (void) multi_select;

    selected.clear();
    cleanAllChildren(ui->scrollArea);
    const auto container = new QGroupBox(ui->scrollArea);
    container->setTitle("");
    const auto containerLayout = new QVBoxLayout();
    container->setLayout(containerLayout);
    ui->scrollArea->setWidget(container);
    for (const auto& v : edsmStarClasses())
    {
        QPointer<QCheckBox> checkbox = new QCheckBox(v);
        containerLayout->addWidget(checkbox);
        connect(checkbox, &QCheckBox::toggled, this, [this, checkbox](bool set)
        {
            if (checkbox)
            {
                if (set)
                    selected += checkbox->text();
                else
                    selected -= checkbox->text();
            }
        });
    }
}

bool EDSMStarClassSelector::isSelected(const QString &star_class) const
{
    return selected.contains(star_class);
}

bool EDSMStarClassSelector::areLimitsInEffect() const
{
    //this should return true if star class (1 or couple) is selected, which means filter is in effect
    return !selected.empty();
}

const QStringList &EDSMStarClassSelector::edsmStarClasses()
{
    const static QStringList edsm_types =
    {
        "O (Blue-White) Star ",
        "B (Blue-White) Star",
        "B (Blue-White super giant) Star",
        "A (Blue-White) Star",
        "A (Blue-White super giant) Star",
        "F (White) Star",
        "F (White super giant) Star",
        "G (White-Yellow) Star",
        "G (White-Yellow super giant) Star",
        "K (Yellow-Orange) Star",
        "K (Yellow-Orange giant) Star",
        "M (Red dwarf) Star",
        "M (Red giant) Star",
        "M (Red super giant) Star",
        "L (Brown dwarf) Star",
        "T (Brown dwarf) Star",
        "Y (Brown dwarf) Star",
        "T Tauri Star",
        "Herbig Ae/Be Star",
        "Wolf-Rayet Star",
        "Wolf-Rayet N Star",
        "Wolf-Rayet NC Star",
        "Wolf-Rayet C Star",
        "Wolf-Rayet O Star",
        "CS Star",
        "C Star",
        "CN Star",
        "CJ Star",
        "CH Star",
        "CHd Star",
        "MS-type Star",
        "S-type Star",
        "White Dwarf (D) Star",
        "White Dwarf (DA) Star",
        "White Dwarf (DAB) Star",
        "White Dwarf (DAO) Star",
        "White Dwarf (DAZ) Star",
        "White Dwarf (DAV) Star",
        "White Dwarf (DB) Star",
        "White Dwarf (DBZ) Star",
        "White Dwarf (DBV) Star",
        "White Dwarf (DO) Star",
        "White Dwarf (DOV) Star",
        "White Dwarf (DQ) Star",
        "White Dwarf (DC) Star",
        "White Dwarf (DCV) Star",
        "White Dwarf (DX) Star",
        "Neutron Star",
        "Black Hole",
        "Supermassive Black Hole",
        "X",
        "RoguePlanet",
    };

    return edsm_types;
}
