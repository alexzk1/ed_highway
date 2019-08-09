#include "spanshroutewidget.h"
#include "ui_spanshroutewidget.h"
#include "spanshsyssuggest.h"
#include "config_ui/globalsettings.h"
#include "spansh_route.h"
#include "utils/json.hpp"

#include <QSpinBox>
#include <QLineEdit>
#include <QSettings>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <QClipboard>

const static QString settingsGroup{"SpanshRouteWidget"};

SpanshRouteWidget::SpanshRouteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpanshRouteWidget)
{
    ui->setupUi(this);
    new SpanshSysSuggest(ui->fromE);
    new SpanshSysSuggest(ui->toE);

    connect(this, &SpanshRouteWidget::crossThreadHasRouteSignal, this, &SpanshRouteWidget::crossThreadHasRoute, Qt::QueuedConnection);

    const auto disable_btn_route = [this](const QString &)
    {
        switchRouteBtn();
    };

    connect(ui->fromE, &QLineEdit::textChanged, this, disable_btn_route);
    connect(ui->toE, &QLineEdit::textChanged, this, disable_btn_route);

    loadValues();

    //if we have all valid data stored, request route again and try make selection
    if (switchRouteBtn() && !lastSelectedSystem.isEmpty())
        on_btnRoute_clicked();

    const static QJsonTableModel::Header header =
    {
        QJsonTableModel::Heading({{"title", tr("System")}, {"index", "system"}}),
        QJsonTableModel::Heading({{"title", tr("Jumps To")}, {"index", "jumps"}}),
        QJsonTableModel::Heading({{"title", tr("Distance Left")}, {"index", "distance_left"}}),
        QJsonTableModel::Heading({{"title", tr("Distance Jumped")}, {"index", "distance_jumped"}}),
    };
    model = new QJsonTableModel( header, this );
    ui->tableView->setModel(model);

    connect(
        ui->tableView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        SLOT(slotSystemSelected(const QItemSelection &, const QItemSelection &))
    );
}

SpanshRouteWidget::~SpanshRouteWidget()
{
    saveValues();
    delete ui;
}

void SpanshRouteWidget::changeEvent(QEvent *e)
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

void SpanshRouteWidget::on_pushButton_2_clicked()
{
    ui->spinBoxRange->setValue(StaticSettingsMap::getGlobalSetts().readInt("02_Int_SHIP_LY"));
}

void SpanshRouteWidget::on_pushButton_clicked()
{
    ui->spinBoxPrecise->setValue(StaticSettingsMap::getGlobalSetts().readInt("03_Int_PRECISE"));
}

void SpanshRouteWidget::on_pushButton_3_clicked()
{
    ui->fromE->setText("");
    ui->fromE->setFocus();
}

void SpanshRouteWidget::on_pushButton_4_clicked()
{
    ui->toE->setText("");
    ui->toE->setFocus();
}

void SpanshRouteWidget::saveValues() const
{
    QSettings settings;
    settings.beginGroup(settingsGroup);
    settings.setValue("from", ui->fromE->text());
    settings.setValue("to", ui->toE->text());
    settings.setValue("ly", ui->spinBoxRange->value());
    settings.setValue("prec", ui->spinBoxPrecise->value());
    settings.setValue("lastSelected", lastSelectedSystem);
    settings.endGroup();
    settings.sync();
}

void SpanshRouteWidget::loadValues()
{
    QSettings settings;
    settings.beginGroup(settingsGroup);
    ui->fromE->setText(settings.value("from", "").toString());
    ui->toE->setText(settings.value("to", "").toString());
    ui->spinBoxRange->setValue(settings.value("ly", StaticSettingsMap::getGlobalSetts().readInt("Int_SHIP_LY")).toDouble());
    ui->spinBoxPrecise->setValue(settings.value("prec", StaticSettingsMap::getGlobalSetts().readInt("Int_PRECISE")).toInt());

    lastSelectedSystem = settings.value("lastSelected").toString();

    settings.endGroup();
}

void SpanshRouteWidget::on_btnRoute_clicked()
{
    ui->btnRoute->setEnabled(false);
    saveValues();
    SpanshRoute r(ui->spinBoxPrecise->value(), ui->spinBoxRange->value(), ui->fromE->text().toStdString(), ui->toE->text().toStdString());
    router.executeRequest(r, [this](auto err, auto js)
    {
        //okey, okey ...not optimal at all, but who cares ... that is not 2000000 routes per second
        //api returned full response, need table only
        const auto it = js.find("system_jumps");
        if (it != js.end())
            emit crossThreadHasRouteSignal(QString::fromStdString(err), QString::fromStdString(it->dump()));
        else
            emit crossThreadHasRouteSignal(QString::fromStdString(err), QString("[]"));
    });
}

bool SpanshRouteWidget::switchRouteBtn()
{
    const bool any_empty = ui->fromE->text().isEmpty() || ui->toE->text().isEmpty();
    const bool enable = !any_empty && !router.isWorking();
    ui->btnRoute->setEnabled(enable);
    return enable;
}

void SpanshRouteWidget::crossThreadHasRoute(QString error, QString json)
{
    //because we did connect with Qt::QueuedConnection now we're in main thread and can use GUI

    //let the thread stop ...
    QTimer::singleShot(1000, [this]()
    {
        switchRouteBtn();
    });
    if (!error.isEmpty())
        QMessageBox::critical(this, qAppName(), error);
    else
    {
        std::cout << json.toStdString() << std::endl;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(json.toUtf8());
        model->setJson(jsonDocument);
        QTimer::singleShot(200, [this]()
        {
            auto hdr = ui->tableView->horizontalHeader();
            if (hdr)
            {
                hdr->setSectionResizeMode(QHeaderView::ResizeToContents);
                hdr->adjustSize();
            }
        });

        //we have something stored, try to find that system in list and select it
        if (!lastSelectedSystem.isEmpty())
        {
            const QVariant find(lastSelectedSystem);

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
                lastSelectedSystem = "";
        }
    }
}

void SpanshRouteWidget::slotSystemSelected(const QItemSelection &, const QItemSelection &n)
{
    const auto ind = n.indexes();
    if (!ind.empty())
    {
        const auto& i = ind.at(0);
        on_tableView_clicked(i);
    }
}

void SpanshRouteWidget::on_tableView_clicked(const QModelIndex &index)
{
    //some bug, when just clicking it does not change selection ...
    QJsonObject object = model->getJsonObject(index);
    lastSelectedSystem = object["system"].toString();
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard)
    {
        clipboard->setText(lastSelectedSystem);
        saveValues();
    }
}
