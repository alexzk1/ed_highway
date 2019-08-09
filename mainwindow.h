#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "config_ui/settingsdialog.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow, protected utility::SaveableWidget<MainWindow>
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    virtual void recurseWrite(QSettings& settings, QObject* object) override;
    virtual void recurseRead(QSettings& settings, QObject* object) override;
private:
    Ui::MainWindow *ui;
    QPointer<SettingsDialog> settDialog;
};

#endif // MAINWINDOW_H
