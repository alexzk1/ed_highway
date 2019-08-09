#ifndef SPANSHROUTEWIDGET_H
#define SPANSHROUTEWIDGET_H

#include <QWidget>
#include "spanshapi.h"

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

private:
    Ui::SpanshRouteWidget *ui;
    SpanshApi router;
};

#endif // SPANSHROUTEWIDGET_H
