#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config_ui/globalsettings.h"
#include <QClipboard>
#include <QMessageBox>
#include "edsmwrapper.h"
#include "stringsfilecache.h"
#include "carriermodulesdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settDialog(new SettingsDialog(this))
#ifdef OCR_ADDED
    , ocrKey(new QHotkey(this))
#endif
{
    ui->setupUi(this);
    connect(ui->actionShow_Settings, &QAction::triggered, this, [this]()
    {
        if (settDialog)
        {
            settingsBeforeShow();
            settDialog->show();
        }
    });
    connect(settDialog, &SettingsDialog::dialogHidden, this, &MainWindow::settingsHidden);
    readSettings(this);
    settingsHidden();
#ifdef OCR_ADDED
    connect(ocrKey, &QHotkey::activated, this, &MainWindow::doScreenOCR, Qt::QueuedConnection);
#endif
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
    settings.setValue(QStringLiteral("maximized"),  isMaximized());
    settings.setValue(QStringLiteral("tab_index"), ui->tabWidget->currentIndex());
}

void MainWindow::recurseRead(QSettings &settings, QObject *object)
{
    Q_UNUSED(object);
    restoreState(settings.value(QStringLiteral("mainwinstate")).toByteArray());
    if (settings.value(QStringLiteral("maximized"), false).toBool())
        showMaximized();
    else
        showNormal();

    const auto tabi = std::min(ui->tabWidget->count() - 1, std::max(0, settings.value(QStringLiteral("tab_index"), 0).toInt()));
    ui->tabWidget->setCurrentIndex(tabi);
}

void MainWindow::settingsHidden()
{
    //need to install global shortcuts for ocr
#ifdef OCR_ADDED
    if (ocrKey)
        ocrKey->setShortcut(QKeySequence(StaticSettingsMap::getGlobalSetts().readString(QStringLiteral("51_MapOcrHotkey"))), true);
#endif
}

void MainWindow::settingsBeforeShow()
{
#ifdef OCR_ADDED
    if (ocrKey)
        ocrKey->setRegistered(false);
#endif
}


void MainWindow::doScreenOCR()
{
#ifdef OCR_ADDED
    const auto list = ocrElite.recognizeScreen();
    const auto s = EliteOCR::tryDetectStarFromMapPopup(list);
    if (s.isEmpty())
        QMessageBox::critical(this, tr("OCR Error"), tr("Cannot read system name."));
    else
        qApp->clipboard()->setText(s);
#endif
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
