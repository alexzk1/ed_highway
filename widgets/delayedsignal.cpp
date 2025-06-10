#include "delayedsignal.h"

DelayedSignal::DelayedSignal(QObject *parent) :
    QObject(parent)
{
}

void DelayedSignal::sourceSignal(int delay_ms)
{
    timer.singleShot(delay_ms, this, [this]() {
        emit delayedSignal();
    });
}
