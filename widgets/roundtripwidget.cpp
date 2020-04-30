#include "roundtripwidget.h"
#include "ui_roundtripwidget.h"
#include "spanshsyssuggest.h"
#include "edsmwrapper.h"
#include <QSettings>
#include <QVariant>
#include <QClipboard>
#include "config_ui/globalsettings.h"
#include <QRegExp>
#include <QProgressDialog>
#include "utils/exec_exit.h"
#include <QThread>
#include "execonmainthread.h"

const static QString settingsGroup{"RoundTripWidget"};

RoundTripWidget::RoundTripWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RoundTripWidget)
{
    ui->setupUi(this);
    ui->btnAdd->setAction(ui->actionAdd);
    new SpanshSysSuggest(ui->edSysManul);
    new SpanshSysSuggest(ui->edCenter);

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
    connect(ui->btnUndo, &QPushButton::clicked, this, &RoundTripWidget::doUndo);

    const static QStringList economies =
    {
        "Agriculture",
        "Colony",
        "Extraction",
        "High Tech",
        "Industrial",
        "Military",
        "Refinery",
        "Service",
        "Terraforming",
        "Tourism",
        "Prison",
        "Repair",
        "Rescue",
        "Damaged",
    };
    ui->list1_or_2->addItems(economies);
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
    pushUndo();
    model->addSystem(ui->edSysManul->text());
    //triggering network request to edsm if any, so later this data will be cached already
    EDSMWrapper::requestSysInfo(ui->edSysManul->text(), [](auto, auto) {});
    ui->edSysManul->clear();
    ui->edSysManul->setFocus();
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

void RoundTripWidget::pushUndo()
{
    undoList.push_back(model->getSystems());
    if (undoList.size() > StaticSettingsMap::getGlobalSetts().readInt("03_Int_UNDO"))
        undoList.pop_front();
}

void RoundTripWidget::doUndo()
{
    if (!undoList.empty())
    {
        model->setSystems(undoList.back());
        undoList.pop_back();
    }
}

void RoundTripWidget::on_btnClear_clicked()
{
    pushUndo();
    model->clearSystems();
    lastSelected = "";
}

void RoundTripWidget::on_btnCalc_clicked()
{
    switchUI(false);
    pushUndo();
    model->startRouteBuild(lastSelected);
}

void RoundTripWidget::on_actionRemoveSelected_triggered()
{
    if (!lastSelected.isEmpty())
    {
        pushUndo();
        model->removeSystem(lastSelected);
    }
}

void RoundTripWidget::on_btnBulkAdd_clicked()
{
    auto src = ui->bulkAdd->toPlainText();
    src.replace("\"", " ");
    src.replace(",", " ");
    src.replace(";", " ");

    if (!src.isEmpty())
    {
        const static QRegExp ns("\\n");
        auto ls = src.split(ns, QString::SplitBehavior::SkipEmptyParts);
        for (auto& s : ls)
            s = s.trimmed();
        if (ls.size())
        {
            pushUndo();
            model->addSystems(ls);
            ui->bulkAdd->clear();
        }
    }
}

void RoundTripWidget::on_btnBulkQuery_clicked()
{
    auto center = ui->edCenter->text();
    if (!center.isEmpty())
    {
        const int ly = ui->spinLY->value();

        switchUI(false);
        if (!progress)
        {
            progress = new QProgressDialog (tr("Getting data from EDSM..."), tr("Cancel"), 0, 100, (QWidget*)this->parent());
            progress->setWindowModality(Qt::WindowModal);
            connect(progress, &QProgressDialog::canceled, this, [this]()
            {
                queryThread.reset();
            });
            progress->setAutoClose(true);
            progress->setAutoReset(true);
        }
        progress->setValue(0);
        progress->show();

        queryThread = utility::startNewRunner([ = ](auto wasCanceled)
        {
            const auto info = EDSMWrapper::requestManySysInfoInRadius(center, ly, [ = ](size_t a, size_t b)->bool
            {
                ExecOnMainThread::get().exec([this, a, b, wasCanceled]()
                {
                    if (progress)
                    {
                        if (progress->maximum() != (int)b)
                            progress->setMaximum(b);
                        progress->setValue(a);
                        if (progress->wasCanceled())
                            *wasCanceled = true;
                    }
                });
                return *wasCanceled;
            });

            ExecOnMainThread::get().exec([ = ]()
            {
                QStringList lst;
                QString eco12 = ui->list1_or_2->currentText();
                const bool eco_filter12 = ui->cb1_or2->isChecked() && !eco12.isEmpty();

                for (const auto& json : info)
                {
                    const auto value_or_none = [&json](const std::string & root, const std::string & name) ->QString
                    {
                        const static QString none ;
                        try
                        {
                            if (root.empty())
                                return QString::fromStdString(EDSMWrapper::valueFromJson<std::string>(json, name));

                            auto r = EDSMWrapper::valueFromJson<nlohmann::json>(json, root);
                            if (name != "population")
                            {
                                auto s = EDSMWrapper::valueFromJson<std::string>(r, name);
                                return QString::fromStdString(s);
                            }
                            else
                            {
                                auto s = EDSMWrapper::valueFromJson<uint64_t>(r, name);
                                return QStringLiteral("%1").arg(s);
                            }
                        }
                        catch (...)
                        {
                        }
                        return none;
                    };

                    bool can_push = true;
                    if (eco_filter12)
                    {
                        const auto e1 = value_or_none("information", "economy");
                        const auto e2 = value_or_none("information", "secondEconomy");
                        can_push = eco12.compare(e1, Qt::CaseInsensitive) == 0 || eco12.compare(e2, Qt::CaseInsensitive) == 0;
                    }
                    if (can_push)
                    {
                        auto n = value_or_none("", "name");
                        if (!n.isEmpty())
                            lst.push_back(n);
                    }
                }
                model->addSystems(lst);

                switchUI(true);
                if (progress)
                    progress->deleteLater();
            });
        });
    }
}
