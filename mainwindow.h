#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "QHotkey"
#include "config_ui/settingsdialog.h"

#include <QMainWindow>
#include <QPointer>

#ifdef OCR_ADDED
    #include "eliteocr.h"
#endif

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
    void changeEvent(QEvent *e);
    void recurseWrite(QSettings &settings, QObject *object) override;
    void recurseRead(QSettings &settings, QObject *object) override;

  private:
    Ui::MainWindow *ui;
    QPointer<SettingsDialog> settDialog;
#ifdef OCR_ADDED
    QPointer<QHotkey> ocrKey;
    EliteOCR ocrElite;
#endif
  private slots:
    void settingsHidden();
    void settingsBeforeShow();
    void doScreenOCR();
    void on_actionClear_Cache_triggered();
    void on_actionCarrier_Calculator_triggered();
};

#endif // MAINWINDOW_H
