#include "spanshroutewidget.h"
#include "ui_spanshroutewidget.h"
#include "spanshsyssuggest.h"

SpanshRouteWidget::SpanshRouteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpanshRouteWidget)
{
    ui->setupUi(this);
    new SpanshSysSuggest(ui->fromE);
}

SpanshRouteWidget::~SpanshRouteWidget()
{
    delete ui;
}

void SpanshRouteWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
