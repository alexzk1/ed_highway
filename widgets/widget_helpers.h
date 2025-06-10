#pragma once
#include <QCheckBox>
#include <QLocale>
#include <QPointer>
#include <QString>
#include <QWidget>

#include <random>
#include <vector>

inline void cleanAllChildren(QWidget *parentWidget)
{
    // https://stackoverflow.com/questions/3940409/how-to-clear-all-the-widgets-in-parent-widgets
    parentWidget->setUpdatesEnabled(false);
    qDeleteAll(parentWidget->findChildren<QWidget *>("", Qt::FindDirectChildrenOnly));
    parentWidget->setUpdatesEnabled(true);
}

template <class Layout, class Container, class CallbackPrepareCheckbox>
inline auto addContainerAsCheckboxes(Layout *addto, const Container &src,
                                     const CallbackPrepareCheckbox &func)
{
    std::vector<QPointer<QCheckBox>> res;
    res.reserve(std::distance(std::begin(src), std::end(src)));

    for (const auto &v : src)
    {
        QPointer<QCheckBox> checkbox = new QCheckBox(static_cast<const QString &>(v));
        addto->addWidget(checkbox);
        func(checkbox, v);
        res.push_back(std::move(checkbox));
    }

    return res;
}

template <class Integer>
inline auto spaced_1000s(const Integer value, const bool force_spaces = false)
{
    static_assert(std::is_integral<Integer>::value, "Only integers are accepted.");

    // this makes thouthands to be space-separated, which I like
    const static QLocale pretty_numbers(QLocale::Language::Finnish, QLocale::Country::Finland);

    const static QLocale dumb;
    const static bool needs_pretty_numbers = dumb.toString(115222).length() == 6;

    return (needs_pretty_numbers | force_spaces) ? pretty_numbers.toString(value)
                                                 : dumb.toString(value);
}

inline auto spaced_1000s(const float value, const bool force_spaces = false)
{
    return spaced_1000s(static_cast<int32_t>(value), force_spaces);
}

namespace myrnd {
template <class T = float>
inline T uniformRandom(T low = static_cast<T>(0.), T hi = static_cast<T>(1.))
{
    static_assert(std::is_floating_point<T>::value, "T must be floating point one.");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<T> dis(low, hi);
    return dis(gen);
}

//----------------------------------------------------------------------------
template <class T = float>
inline T gaussRandom(T mean = 0.0f, T stdev = 1.0f)
{
    static_assert(std::is_floating_point<T>::value, "T must be floating point one.");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<T> dis(mean, stdev);
    return dis(gen);
}

inline bool randomBool()
{
    return uniformRandom<float>(0.f, 1.f) < 0.5;
}
} // namespace myrnd
