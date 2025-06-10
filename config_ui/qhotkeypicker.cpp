#include "qhotkeypicker.h"

#include <QKeyEvent>

QHotkeyPicker::QHotkeyPicker(QWidget *owner) :
    QPushButton(owner),
    timer(new QTimer(this)),
    seconds_left{0}
{
    setCheckable(true);

    connect(this, &QPushButton::toggled, this, [this](bool checked) {
        if (checked)
        {
            setFocus(Qt::MouseFocusReason);
            seconds_left = 10;
            showSeconds();
            timer->start(1000);
        }
        else
        {
            timer->stop();
            setText(hot_string);
        }
    });

    connect(timer, &QTimer::timeout, this, [this]() {
        if (--seconds_left < 1)
            setChecked(false);
        else
            showSeconds();
    });
}

void QHotkeyPicker::showSeconds()
{
    setText(QString("%1").arg(seconds_left));
}

void QHotkeyPicker::setHot(const QString &v)
{
    hot_string = v;
    if (!isChecked())
        setText(v);
    emit hotStringChanged(v);
}

QString QHotkeyPicker::getHot() const
{
    return hot_string;
}

static bool isModifier(const int key)
{
    const static int keys[] = {
      Qt::Key_unknown, Qt::Key_Control, Qt::Key_Shift, Qt::Key_Meta, Qt::Key_Alt, Qt::Key_AltGr,
    };

    return std::any_of(std::begin(keys), std::end(keys), [&key](auto v) {
        return key == v;
    });
}

// fixme: this might be a bit broken if kb lang is switched ...
static const QString &unshift(const QString &src)
{
    const static std::map<QString, QString> remap = {
      {"!", "1"},  {"@", "2"}, {"#", "3"},  {"$", "4"}, {"%", "5"}, {"^", "6"}, {"&", "7"},
      {"*", "8"},  {"(", "9"}, {")", "0"},  {"_", "-"}, {"+", "="}, {"{", "["}, {"}", "]"},
      {"\"", "'"}, {":", ";"}, {"|", "\\"}, {">", "."}, {"<", ","}, {"?", "/"}, {"~", "`"},
    };
    auto it = remap.find(src);
    if (it != remap.end())
        return it->second;
    return src;
}

void QHotkeyPicker::keyPressEvent(QKeyEvent *event)
{
    if (isChecked() && event->key() != Qt::Key_unknown)
    {
        const auto mods = QKeySequence(event->modifiers()).toString();
        const auto key = unshift(QKeySequence(event->key()).toString());
        hot_string = mods + key;
    }
}

void QHotkeyPicker::keyReleaseEvent(QKeyEvent *event)
{
    if (isChecked() && !isModifier(event->key()))
        setChecked(false);
}
