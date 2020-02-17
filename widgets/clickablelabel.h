#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H
#include <QLabel>
#include <QMouseEvent>
#include <QColor>


class ColorLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
public:
    explicit ColorLabel( const QString& text="", QWidget* parent=nullptr ): QLabel(text, parent){}
    explicit ColorLabel( QWidget* parent=nullptr ): QLabel(parent){}

    void setColor (QColor color)
    {
        setStyleSheet(QString("color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));
    }
    QColor color()
    {
        return palette().color(QPalette::WindowText);
    }
};

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel( const QString& text="", QWidget* parent=nullptr ): QLabel(text, parent){}
    explicit ClickableLabel( QWidget* parent=nullptr ): QLabel(parent){}
signals:
    void clicked();
protected:
    virtual void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
            emit clicked();
    }
};

#endif // CLICKABLELABEL_H
