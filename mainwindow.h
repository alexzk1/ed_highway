#pragma once

#include "config_ui/settingsdialog.h"
#include "utils/cm_ctors.h"

#include <qtmetamacros.h>

#include <QMainWindow>
#include <QObject>
#include <QPointer>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow final : public QMainWindow, protected utility::SaveableWidget<MainWindow>
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    NO_COPYMOVE(MainWindow);
    ~MainWindow() final;

  protected:
    void changeEvent(QEvent *e) final;
    void recurseWrite(QSettings &settings, QObject *object) final;
    void recurseRead(QSettings &settings, QObject *object) final;

  private:
    Ui::MainWindow *ui;
    QPointer<SettingsDialog> settDialog;
  private slots:
    void on_actionClear_Cache_triggered();
    void on_actionCarrier_Calculator_triggered();
};
