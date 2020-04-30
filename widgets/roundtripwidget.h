#ifndef ROUNDTRIPWIDGET_H
#define ROUNDTRIPWIDGET_H

#include <QWidget>
#include <QPointer>
#include "edsmsystemsmodel.h"
#include <QItemSelection>
#include <QMenu>
#include <QList>

namespace Ui
{
    class RoundTripWidget;
}

class RoundTripWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RoundTripWidget(QWidget *parent = nullptr);
    ~RoundTripWidget();

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_actionAdd_triggered();
    void slotSystemSelected(const QItemSelection &, const QItemSelection &n);
    void on_tableView_clicked(const QModelIndex &index);
    void on_btnClear_clicked();
    void on_btnCalc_clicked();
    void updateSelection();
    void on_actionRemoveSelected_triggered();
    void doUndo();
private:
    Ui::RoundTripWidget *ui;
    QPointer<EDSMSystemsModel> model;
    QString lastSelected;
    QPointer<QMenu> rclickMenu;
    QList<QStringList> undoList;

    void saveValues() const;
    void loadValues();


    void switchUI(bool enabled);

    void pushUndo();

};

#endif // ROUNDTRIPWIDGET_H
