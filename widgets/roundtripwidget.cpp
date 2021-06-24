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
#include <QDesktopServices>
#include <QMessageBox>
#include "execonmainthread.h"
#include "utils/strutils.h"
#include "point.h"

const static QString settingsGroup{"RoundTripWidget"};

void static limitRangeToEdsm(QSpinBox* ptr)
{
    constexpr static int edsm_range_limit = 100; //limited by website
    if (ptr)
        ptr->setMaximum(std::min(edsm_range_limit, ptr->maximum()));
}

RoundTripWidget::RoundTripWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RoundTripWidget)
{
    ui->setupUi(this);
    ui->btnAdd->setAction(ui->actionAdd);

    //limiting whatever we configured in editor to edsm limit
    limitRangeToEdsm(ui->spinInnerLY);
    limitRangeToEdsm(ui->spinLY);

    new SpanshSysSuggest(ui->edSysManul);
    new SpanshSysSuggest(ui->edCenter);

    rclickMenu = new QMenu(this);
    rclickMenu->addAction(ui->actionOpen_in_Browser);

    rclickMenu->addSeparator();
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
    ui->tableView->verticalHeader()->setVisible(true);

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


    connect(ui->cbgN, &QCheckBox::toggled, this, [this](bool t)
    {
        ui->giantsSpin->setEnabled(t);
    });

    ui->giantsSpin->setEnabled(ui->cbgN->isChecked());

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
    QStringList values;
    QString last;

    QSettings settings;
    settings.beginGroup(settingsGroup);
    values = settings.value("system_list", {}).toStringList();
    last   = settings.value("lastSelected").toString();
    settings.endGroup();

    //setSystems may trigger long EDSM query for distances, lets try to do it by timer so GUI is shown
    if (values.size() > 0)
    {
        switchUI(false);
        QTimer::singleShot(500, [last, values, this]()
        {
            blockSignals(true);
            model->setSystems(values);
            lastSelected = last;
            updateSelection();
            blockSignals(false);
            switchUI(true);
        });
    }
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
    saveValues();
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
    const auto center = ui->edCenter->text();
    if (!center.isEmpty())
    {
        const auto center_point = Point::fromJson(EDSMWrapper::requestSysInfo(center));
        const int ly = ui->spinLY->value();
        const int inner_ly = ui->spinInnerLY->value();
        if (inner_ly > ly)
        {
            showError(tr("Inner LY must be less-or-equal to LY parameter."));
            return;
        }

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

        const bool r_icy        = ui->cbrIcy->isChecked();
        const bool r_rocky      = ui->cbrRocky->isChecked();
        const bool r_metall     = ui->cbrMetal->isChecked();
        const bool r_richmetall = ui->cbrRichMetall->isChecked();

        const bool r_giants     = ui->cbgN->isChecked();

        //this variable allows to add more body-related filters later easier
        const bool need_bodies_info = r_icy || r_rocky || r_metall || r_richmetall || r_giants;

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

            //doing a map, where key is a system name
            std::map<QString, nlohmann::json> binfo;
            if (need_bodies_info)
            {
                auto tmp = EDSMWrapper::requestManyBodiesInfoInRadius(center, ly, [ = ](size_t a, size_t b)->bool
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
                for (auto& v : tmp)
                {
                    try
                    {
                        auto n = EDSMWrapper::valueFromJson<std::string>(v, "name");
                        binfo[QString::fromStdString(n)] = std::move(v);
                    }
                    catch (...)
                    {
                    }
                }
            }


            ExecOnMainThread::get().exec([ = ]()
            {
                QStringList lst;
                QString eco12 = ui->list1_or_2->currentText();
                const bool eco_filter12 = ui->cb1_or2->isChecked() && !eco12.isEmpty();
                const bool star_class_filter = ui->starClass->areLimitsInEffect();


                for (const auto& json : info)
                {
                    const auto current_point = Point::fromJson(json);
                    if (center_point.distance(current_point) < inner_ly)
                        continue;

                    //lamda to extract value from json
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

                    //filtering sys-info
                    const auto sys_name = value_or_none("", "name");


                    if (!sys_name.isEmpty())
                    {
                        bool can_push_1 = true;
                        bool can_push_2 = !need_bodies_info;

                        if (eco_filter12)
                        {
                            const auto e1 = value_or_none("information", "economy");
                            const auto e2 = value_or_none("information", "secondEconomy");
                            can_push_1 = eco12.compare(e1, Qt::CaseInsensitive) == 0 || eco12.compare(e2, Qt::CaseInsensitive) == 0;
                        }

                        if (star_class_filter)
                        {
                            const auto type_name = value_or_none("primaryStar", "type");
                            can_push_1 = can_push_1 && ui->starClass->isSelected(type_name);
                        }

                        if (can_push_1)
                        {
                            //filtering by bodies
                            if (need_bodies_info)
                            {
                                can_push_2 = false;
                                const auto it = binfo.find(sys_name);
                                if (it != binfo.end())
                                    try
                                    {
                                        const auto& bjs = it->second; //bodies full json
                                        const auto bodies = EDSMWrapper::valueFromJson<nlohmann::json>(bjs, "bodies");

                                        const auto Ng = ui->giantsSpin->value();
                                        int current_giants_count = 0;

                                        for (const auto& b : bodies)
                                        {
                                            if (can_push_2)
                                                break;

                                            //testing rings, ignore warning about always true, need_bodies_info may contain more conditions later
                                            if (r_icy || r_rocky || r_metall || r_richmetall)
                                            {
                                                try
                                                {
                                                    const auto rings = EDSMWrapper::valueFromJson<nlohmann::json>(b, "rings");
                                                    for (const auto& r : rings)
                                                    {
                                                        const auto rtype = EDSMWrapper::valueFromJson<std::string>(r, "type");
                                                        can_push_2 = can_push_2
                                                                     || (r_icy   && rtype == "Icy")
                                                                     || (r_rocky && rtype == "Rocky")
                                                                     || (r_richmetall && rtype == "Metal Rich")
                                                                     || (r_metall && rtype == "Metallic");
                                                        if (can_push_2)
                                                            break;
                                                    }
                                                }
                                                catch (...)
                                                {
                                                }
                                            }

                                            if (r_giants)
                                            {
                                                //"subType": "Gas giant with ammonia-based life",
                                                try
                                                {
                                                    const auto rtype = EDSMWrapper::valueFromJson<std::string>(b, "subType");
                                                    current_giants_count += (utility::strcontains(rtype, "giant")) ? 1 : 0;
                                                    can_push_2 = can_push_2 || (current_giants_count >= Ng);
                                                }
                                                catch (...)
                                                {

                                                }
                                            }
                                        }
                                    }
                                    catch (...)
                                    {
                                    }
                            }

                            if (can_push_2)
                                lst.push_back(sys_name);
                        }
                    }
                }

                //making sure "center" is 1st in list if it is present there
                for (auto & v : lst)
                {
                    if (v == center)
                    {
                        if (v != lst.first())
                            std::swap(v, lst.first());
                        break;
                    }
                }

                model->addSystems(lst);

                switchUI(true);
                if (progress)
                    progress->deleteLater();
            });
        });
    }
    else
        showError(tr("Center is not given."));
}

void RoundTripWidget::on_btnCopy_clicked()
{
    model->copyCurrentList();
}

void RoundTripWidget::on_actionOpen_in_Browser_triggered()
{
    if (!lastSelected.isEmpty())
        try
        {
            QDesktopServices::openUrl(EDSMWrapper::getSystemUrl(lastSelected));
        }
        catch (...)
        {
        }
}

void RoundTripWidget::showError(const QString &msg) const
{
    QMessageBox::critical((QWidget*)this->parent(), qAppName(), msg);
}
