#ifndef ROUNDTRIPWIDGET_H
#define ROUNDTRIPWIDGET_H

#include <QWidget>
#include <QPointer>
#include "edsmsystemsmodel.h"
#include <QItemSelection>

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

private:
    Ui::RoundTripWidget *ui;
    QPointer<QAbstractTableModel> model;

    void saveValues() const;
    void loadValues();
};

#endif // ROUNDTRIPWIDGET_H
