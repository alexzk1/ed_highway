#pragma once
#include <QWidget>
#include <QCheckBox>
#include <QPointer>
#include <QString>
#include <QLocale>

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

template<class Integer>
inline auto spaced_1000s(const Integer value)
{
    static_assert (std::is_integral<Integer>::value, "Only integers are accepted.");

    //this makes thouthands to be space-separated, which I like
    const static QLocale pretty_numbers(QLocale::Language::Finnish, QLocale::Country::Finland);

    const static QLocale dumb;
    const static bool needs_pretty_numbers = dumb.toString(115222).length() == 6;

    return (needs_pretty_numbers) ? pretty_numbers.toString(value) : dumb.toString(value);
}
