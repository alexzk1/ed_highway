#include "spanshsyssuggest.h"
#include "spansh_sysname.h"
#include <QHeaderView>
#include <QEvent>
#include <QKeyEvent>
#include <string>
#include "config_ui/globalsettings.h"

SpanshSysSuggest::SpanshSysSuggest(QLineEdit *parent) : QObject(parent), editor(parent)
{
    popup = new QTreeWidget;
    popup->setWindowFlags(Qt::Popup);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->setFocusProxy(parent);
    popup->setMouseTracking(true);

    popup->setColumnCount(1);
    popup->setUniformRowHeights(true);
    popup->setRootIsDecorated(false);
    popup->setEditTriggers(QTreeWidget::NoEditTriggers);
    popup->setSelectionBehavior(QTreeWidget::SelectRows);
    popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->header()->hide();

    popup->installEventFilter(this);

    connect(popup, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(doneCompletion()));
    timer.setSingleShot(true);
    timer.setInterval(StaticSettingsMap::getGlobalSetts().readInt("01_SYS_NAME_DROP_DELAY"));
    connect(&timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(editor, SIGNAL(textEdited(QString)), &timer, SLOT(start()));

    //resolves cross-thread calls (Qt::QueuedConnection)
    connect(this, &SpanshSysSuggest::apiCompletedRequest, this, &SpanshSysSuggest::showCompletion, Qt::QueuedConnection);
}

SpanshSysSuggest::~SpanshSysSuggest()
{
    delete popup;
}

bool SpanshSysSuggest::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj != popup)
        return false;

    if (ev->type() == QEvent::MouseButtonPress)
    {
        popup->hide();
        editor->setFocus();
        return true;
    }

    if (ev->type() == QEvent::KeyPress)
    {
        bool consumed = false;
        int key = static_cast<QKeyEvent*>(ev)->key();
        switch (key)
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                doneCompletion();
                consumed = true;
                break;

            case Qt::Key_Escape:
                editor->setFocus();
                popup->hide();
                consumed = true;
                break;

            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
                break;

            default:
                editor->setFocus();
                editor->event(ev);
                popup->hide();
                break;
        }

        return consumed;
    }

    return false;
}

void SpanshSysSuggest::showCompletion(const QVector<QString> &choices)
{
    if (choices.isEmpty())
        return;

    const QPalette &pal = editor->palette();
    QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);

    popup->setUpdatesEnabled(false);
    popup->clear();

    for (const auto &choice : choices)
    {
        auto item  = new QTreeWidgetItem(popup);
        item->setText(0, choice);
        item->setForeground(0, color);
    }

    popup->setCurrentItem(popup->topLevelItem(0));
    popup->resizeColumnToContents(0);
    popup->setUpdatesEnabled(true);

    popup->move(editor->mapToGlobal(QPoint(0, editor->height())));
    popup->setFocus();
    popup->show();
}

void SpanshSysSuggest::doneCompletion()
{
    timer.stop();
    popup->hide();
    editor->setFocus();
    QTreeWidgetItem *item = popup->currentItem();
    if (item)
    {
        editor->setText(item->text(0));
        QMetaObject::invokeMethod(editor, "returnPressed");
    }
}

void SpanshSysSuggest::preventSuggest()
{
    timer.stop();
}

void SpanshSysSuggest::autoSuggest()
{
    const SpanshSysName sn{editor->text().toStdString()};

    const auto result = [this](auto err, nlohmann::json js)
    {
        QVector<QString> choices;
        if (err.empty())
        {
            using namespace nlohmann;

            for (auto it = js.begin(); it != js.end(); ++it)
                choices.push_back(QString::fromStdString(it->get<std::string>()));
        }
        emit apiCompletedRequest(choices);
    };
    sapi.executeRequest(sn, result);
}
