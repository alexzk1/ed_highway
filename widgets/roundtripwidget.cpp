#include "roundtripwidget.h"
#include "ui_roundtripwidget.h"
#include "spanshsyssuggest.h"

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

    loadValues();

    model = new EDSMSystemsModel(this);
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

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
    //    QSettings settings;
    //    settings.beginGroup(settingsGroup);
    //    settings.setValue("from", ui->fromE->text());
    //    settings.setValue("to", ui->toE->text());
    //    settings.setValue("ly", ui->spinBoxRange->value());
    //    settings.setValue("prec", ui->spinBoxPrecise->value());
    //    settings.setValue("lastSelected", lastSelectedSystem);
    //    settings.setValue("revertFrom", revertFrom);
    //    settings.setValue("revertTo", revertTo);
    //    settings.endGroup();
    //    settings.sync();
}

void RoundTripWidget::loadValues()
{
    //    QSettings settings;
    //    settings.beginGroup(settingsGroup);
    //    ui->fromE->setText(settings.value("from", "").toString());
    //    settings.endGroup();
}
