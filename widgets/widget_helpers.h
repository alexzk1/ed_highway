#pragma once
#include <QWidget>

inline void cleanAllChildren(QWidget *parentWidget)
{
    //https://stackoverflow.com/questions/3940409/how-to-clear-all-the-widgets-in-parent-widgets
    parentWidget->setUpdatesEnabled(false);
    qDeleteAll(parentWidget->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
    parentWidget->setUpdatesEnabled(true);
}
