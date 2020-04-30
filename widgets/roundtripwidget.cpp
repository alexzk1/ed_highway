#include "roundtripwidget.h"
#include "ui_roundtripwidget.h"
#include "spanshsyssuggest.h"
#include "edsmwrapper.h"
#include <QSettings>
#include <QVariant>

const static QString settingsGroup{"RoundTripWidget"};

RoundTripWidget::RoundTripWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RoundTripWidget)
{
    ui->setupUi(this);
    ui->btnAdd->setAction(ui->actionAdd);
    new SpanshSysSuggest(ui->edSysManul);

    const auto switchAddAction = [this](const QString & txt)
    {
        const bool enabled = !txt.isEmpty();
        ui->actionAdd->setEnabled(enabled);
    };
    switchAddAction("");
    connect(ui->edSysManul, &QLineEdit::textChanged, this, switchAddAction);
    connect(ui->edSysManul, &QLineEdit::returnPressed, ui->actionAdd, &QAction::trigger);

    model = new EDSMSystemsModel(this);
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);


    loadValues();
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            SLOT(slotSystemSelected(const QItemSelection &, const QItemSelection &)));


}

RoundTripWidget::~RoundTripWidget()
{
    saveValues();
    delete ui;
}

void RoundTripWidget::changeEvent(QEvent *e)
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

void RoundTripWidget::on_actionAdd_triggered()
{
    model->addSystem(ui->edSysManul->text());
    //triggering network request to edsm if any, so later this data will be cached already
    EDSMWrapper::requestSysInfo(ui->edSysManul->text(), [](auto, auto) {});
    ui->edSysManul->clear();
}

void RoundTripWidget::slotSystemSelected(const QItemSelection &, const QItemSelection &n)
{
    const auto ind = n.indexes();
    if (!ind.empty())
    {
        const auto& i = ind.at(0);
        on_tableView_clicked(i);
    }
}

void RoundTripWidget::on_tableView_clicked(const QModelIndex &index)
{
    //some bug, when just clicking it does not change selection ...
    //    QJsonObject object = model->getJsonObject(index);
    //    lastSelectedSystem = object["system"].toString();
    //    QClipboard *clipboard = QGuiApplication::clipboard();
    //    if (clipboard)
    //    {
    //        clipboard->setText(lastSelectedSystem);
    //        saveValues();
    //    }
}

void RoundTripWidget::saveValues() const
{
    QSettings settings;
    settings.beginGroup(settingsGroup);
    settings.setValue("system_list", model->getSystems());
    settings.endGroup();
    settings.sync();
}

void RoundTripWidget::loadValues()
{
    QSettings settings;
    settings.beginGroup(settingsGroup);
    model->setSystems(settings.value("system_list", {}).toStringList());
    settings.endGroup();
}

void RoundTripWidget::on_btnClear_clicked()
{
    model->clearSystems();
}
