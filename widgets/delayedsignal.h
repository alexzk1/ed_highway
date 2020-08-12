#ifndef DELAYEDSIGNAL_H
#define DELAYEDSIGNAL_H

#include <QObject>
#include <QTimer>

//this object receives signal and issues it delayed
//skipping repeats of original source

class DelayedSignal : public QObject
{
    Q_OBJECT
public:
    explicit DelayedSignal(QObject *parent = nullptr);
public slots:
    void sourceSignal(int delay_ms);
signals:
    void delayedSignal();
private:
    QTimer timer;
};

#endif // DELAYEDSIGNAL_H
