#include "mainwindow.h"

#include "carriermodulesdialog.h"
#include "config_ui/settingsdialog.h"
#include "stringsfilecache.h"
#include "ui_mainwindow.h" // IWYU pragma: keep

#include <qstringliteral.h>
#include <qtpreprocessorsupport.h>

#include <QAction>
#include <QClipboard>
#include <QEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QWidget>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settDialog(new SettingsDialog(this))
{
    ui->setupUi(this);
    connect(ui->actionShow_Settings, &QAction::triggered, this, [this]() {
        if (settDialog)
        {
            settDialog->show();
        }
    });
    readSettings(this);
}

MainWindow::~MainWindow()
{
    writeSettings(this);
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void MainWindow::recurseWrite(QSettings &settings, QObject *object)
{
    Q_UNUSED(object);
    settings.setValue(QStringLiteral("mainwinstate"), saveState());
    settings.setValue(QStringLiteral("maximized"), isMaximized());
    settings.setValue(QStringLiteral("tab_index"), ui->tabWidget->currentIndex());
}

void MainWindow::recurseRead(QSettings &settings, QObject *object)
{
    Q_UNUSED(object);
    restoreState(settings.value(QStringLiteral("mainwinstate")).toByteArray());
    if (settings.value(QStringLiteral("maximized"), false).toBool())
    {
        showMaximized();
    }
    else
    {
        showNormal();
    }

    const auto tabi = std::min(ui->tabWidget->count() - 1,
                               std::max(0, settings.value(QStringLiteral("tab_index"), 0).toInt()));
    ui->tabWidget->setCurrentIndex(tabi);
}

void MainWindow::on_actionClear_Cache_triggered()
{
    StringsFileCache::instance().cleanAll();
}

void MainWindow::on_actionCarrier_Calculator_triggered()
{
    CarrierModulesDialog cd(true, this);
    cd.exec();
}
