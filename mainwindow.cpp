#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settDialog(new SettingsDialog(this))
{
    ui->setupUi(this);
    connect(ui->actionShow_Settings, &QAction::triggered, this, [this]()
    {
        if (settDialog)
            settDialog->show();
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
