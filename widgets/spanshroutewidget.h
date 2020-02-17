#ifndef SPANSHROUTEWIDGET_H
#define SPANSHROUTEWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QStringList>
#include <QItemSelection>
#include <QMenu>
#include "spanshapi.h"
#include "qjsontablemodel.h"

namespace Ui
{
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

private:

    Ui::SpanshRouteWidget *ui;
    SpanshApi router{3};
    QPointer<QJsonTableModel> model;
    QString lastSelectedSystem;
    QStringList revertFrom;
    QStringList revertTo;
    QPointer<QMenu> fromMenu;
    QPointer<QMenu> toMenu;

    void saveValues() const;
    void loadValues();

    void updateButtonsMenu();
signals:
    void crossThreadHasRouteSignal(QString error, QString json);
};

#endif // SPANSHROUTEWIDGET_H
