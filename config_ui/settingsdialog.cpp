// License: MIT, (c) Oleksiy Zakharov, 2016, alexzkhr@gmail.com

#include "settingsdialog.h"

#include "config_ui/stylesheetsloader.h"
#include "globalsettings.h"

#include "ui_settingsdialog.h"

#include <utility>

namespace {
constexpr auto kStyleSheetSetting = "1000_VisualStyleSheet";
constexpr auto kBiggerFont = "1010_BiggerFont";
} // namespace

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    readSettings(this);

    const StaticSettingsMap &sett = StaticSettingsMap::getGlobalSetts();
    auto w = new QWidget();
    ui->scrollSettings->setWidget(w);
    auto layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    w->setLayout(layout);

    auto wl = sett.createWidgets();
    for (const auto &wi : std::as_const(wl))
    {
        wi->setParent(this);
        layout->addWidget(wi);
        if (kStyleSheetSetting == wi->property("SettKey"))
        {
            for (const auto &subs : wi->children())
            {
                if (QComboBox *comboBox = dynamic_cast<QComboBox *>(subs))
                {
                    const auto index = sett.readInt(kStyleSheetSetting);
                    StyleSheetsLoader::LoadStyleSheet(comboBox->itemData(index).toString());
                    StyleSheetsLoader::AttachHandlerToComboBox(comboBox);
                    if (sett.readBool(kBiggerFont))
                    {
                        StyleSheetsLoader::UseBiggerFont();
                    }
                    break;
                }
            }
        }
    }
}

SettingsDialog::~SettingsDialog()
{
    writeSettings(this);
    delete ui;
}

void SettingsDialog::changeEvent(QEvent *e)
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

void SettingsDialog::hideEvent(QHideEvent *event)
{
    emit dialogHidden();
    QDialog::hideEvent(event);
}
