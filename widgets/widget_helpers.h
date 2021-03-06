#pragma once
#include <QWidget>
#include <QCheckBox>
#include <QPointer>
#include <QString>

inline void cleanAllChildren(QWidget *parentWidget)
{
    //https://stackoverflow.com/questions/3940409/how-to-clear-all-the-widgets-in-parent-widgets
    parentWidget->setUpdatesEnabled(false);
    qDeleteAll(parentWidget->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
    parentWidget->setUpdatesEnabled(true);
}

template <class Layout, class Container, class CallbackPrepareCheckbox>
inline void addContainerAsCheckboxes(Layout* addto, const Container& src, const CallbackPrepareCheckbox& func)
{
    for (const auto& v : src)
    {
        QPointer<QCheckBox> checkbox = new QCheckBox(static_cast<const QString&>(v));
        addto->addWidget(checkbox);
        func(checkbox, v);
    }
}
