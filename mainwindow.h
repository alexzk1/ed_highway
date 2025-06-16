#pragma once

#include "config_ui/settingsdialog.h"

#include <QMainWindow>
#include <QPointer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, protected utility::SaveableWidget<MainWindow>
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  protected:
    void changeEvent(QEvent *e) override;
    void recurseWrite(QSettings &settings, QObject *object) override;
    void recurseRead(QSettings &settings, QObject *object) override;

  private:
    Ui::MainWindow *ui;
    QPointer<SettingsDialog> settDialog;
  private slots:
    void on_actionClear_Cache_triggered();
    void on_actionCarrier_Calculator_triggered();
};
