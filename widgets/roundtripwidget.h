#ifndef ROUNDTRIPWIDGET_H
#define ROUNDTRIPWIDGET_H

#include <QWidget>
#include <QPointer>
#include "edsmsystemsmodel.h"
#include <QItemSelection>
#include <QMenu>
#include <QList>
#include "utils/runners.h"
#include <QProgressDialog>

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
    void on_btnBulkAdd_clicked();

    void on_btnBulkQuery_clicked();

    void on_btnCopy_clicked();

    void on_actionOpen_in_Browser_triggered();

private:
    Ui::RoundTripWidget *ui;
    QPointer<EDSMSystemsModel> model;
    QString lastSelected;
    QPointer<QMenu> rclickMenu;
    QList<QStringList> undoList;
    utility::runner_t queryThread{nullptr};
    QPointer<QProgressDialog> progress{nullptr};

    void saveValues() const;
    void loadValues();


    void switchUI(bool enabled);

    void pushUndo();

};

#endif // ROUNDTRIPWIDGET_H
