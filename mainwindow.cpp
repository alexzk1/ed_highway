#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config_ui/globalsettings.h"
#include <QClipboard>

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
    settings.setValue("mainwinstate", this->saveState());
    settings.setValue("maximized", this->isMaximized());
}

void MainWindow::recurseRead(QSettings &settings, QObject *object)
{
    Q_UNUSED(object);
    this->restoreState(settings.value("mainwinstate").toByteArray());
    if (settings.value("maximized", false).toBool())
        showMaximized();
    else
        showNormal();
}

void MainWindow::settingsHidden()
{
    //need to install global shortcuts for ocr
#ifdef OCR_ADDED
    if (ocrKey)
        ocrKey->setShortcut(QKeySequence(StaticSettingsMap::getGlobalSetts().readString("51_MapOcrHotkey")), true);
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
    const auto list = ocrElite.recognizeScreen();
    const auto s = EliteOCR::tryDetectStarFromMapPopup(list);
    qApp->clipboard()->setText(s);
}
