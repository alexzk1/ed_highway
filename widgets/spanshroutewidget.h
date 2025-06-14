#ifndef SPANSHROUTEWIDGET_H
#define SPANSHROUTEWIDGET_H

#include "qjsontablemodel.h"
#include "spanshapi.h"

#include <QItemSelection>
#include <QMenu>
#include <QPointer>
#include <QStringList>
#include <QWidget>

namespace Ui {
class SpanshRouteWidget;
}

class SpanshRouteWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit SpanshRouteWidget(QWidget *parent = nullptr);
    ~SpanshRouteWidget();

  protected:
    void changeEvent(QEvent *e);
    bool switchRouteBtn();
  private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_btnRoute_clicked();

    void crossThreadHasRoute(QString error, QString json);
    void slotSystemSelected(const QItemSelection &, const QItemSelection &n);
    void on_tableView_clicked(const QModelIndex &index);

    void on_btnSwap_clicked();

    void on_btnUp_clicked();

    void on_actionOpen_In_Browser_triggered();

  private:
    Ui::SpanshRouteWidget *ui;
    SpanshApi router{3};
    QPointer<QJsonTableModel> model;
    QString lastSelectedSystem;
    QStringList revertFrom;
    QStringList revertTo;
    QPointer<QMenu> fromMenu;
    QPointer<QMenu> toMenu;
    QPointer<QMenu> rclickOnTableMenu;

    void saveValues() const;
    void loadValues();

    void updateButtonsMenu();
  signals:
    void crossThreadHasRouteSignal(QString error, QString json);
};

#endif // SPANSHROUTEWIDGET_H
