#ifndef SPANSHSYSSUGGEST_H
#define SPANSHSYSSUGGEST_H

//based on: https://doc.qt.io/qt-5/qtnetwork-googlesuggest-example.html

#include <QObject>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTimer>
#include <QPointer>
#include "spanshapi.h"

class SpanshSysSuggest : public QObject
{
    Q_OBJECT
public:
    explicit SpanshSysSuggest(QLineEdit *parent = nullptr);
    ~SpanshSysSuggest() override;

    bool eventFilter(QObject *obj, QEvent *ev) override;
signals:
private slots:
    void showCompletion(const QVector<QString> &choices);
public slots:
    void doneCompletion();
    void preventSuggest();
    void autoSuggest();
private:
    QPointer<QLineEdit>   editor{nullptr};
    QPointer<QTreeWidget> popup{nullptr};
    QTimer timer;
    SpanshApi sapi{3};
};

#endif // SPANSHSYSSUGGEST_H
