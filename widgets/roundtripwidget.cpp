#include "roundtripwidget.h"
#include "ui_roundtripwidget.h"
#include "spanshsyssuggest.h"
#include "edsmwrapper.h"
#include <QSettings>
#include <QVariant>
#include <QClipboard>

const static QString settingsGroup{"RoundTripWidget"};

RoundTripWidget::RoundTripWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RoundTripWidget)
{
    ui->setupUi(this);
    ui->btnAdd->setAction(ui->actionAdd);
    new SpanshSysSuggest(ui->edSysManul);

    rclickMenu = new QMenu(this);
    rclickMenu->addAction(ui->actionRemoveSelected);

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


    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, [this](QPoint p)
    {
        rclickMenu->popup(ui->tableView->viewport()->mapToGlobal(p));
    });

    connect(model, &EDSMSystemsModel::routeReady, this, [this]()
    {
        updateSelection();
        switchUI(true);
    });

    connect(model, &EDSMSystemsModel::systemsChanged, this, &RoundTripWidget::updateSelection);
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
    lastSelected = model->getSystemNameAt(index);
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard)
    {
        clipboard->setText(lastSelected);
        saveValues();
    }
}

void RoundTripWidget::saveValues() const
{
    QSettings settings;
    settings.beginGroup(settingsGroup);
    settings.setValue("system_list", model->getSystems());
    settings.setValue("lastSelected", lastSelected);
    settings.endGroup();
    settings.sync();
}

void RoundTripWidget::loadValues()
{
    QSettings settings;
    settings.beginGroup(settingsGroup);
    model->setSystems(settings.value("system_list", {}).toStringList());
    lastSelected = settings.value("lastSelected").toString();
    settings.endGroup();

    updateSelection();
}

void RoundTripWidget::updateSelection()
{
    if (!lastSelected.isEmpty())
    {
        const QVariant find(lastSelected);

        int row = model->rowCount();
        bool found = false;
        for (int i = 0; i < row; ++i)
        {
            const auto index = model->index(i, 0);
            if (model->data(index, Qt::DisplayRole) == find)
            {
                ui->tableView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
                ui->tableView->scrollTo(index);
                found = true;
                break;
            }
        }
        if (!found)
            lastSelected = "";
    }
}

void RoundTripWidget::switchUI(bool enabled)
{
    setEnabled(enabled);
}

void RoundTripWidget::on_btnClear_clicked()
{
    model->clearSystems();
    lastSelected = "";
}

void RoundTripWidget::on_btnCalc_clicked()
{
    switchUI(false);
    model->startRouteBuild(lastSelected);
}

void RoundTripWidget::on_actionRemoveSelected_triggered()
{
    if (!lastSelected.isEmpty())
        model->removeSystem(lastSelected);
}
