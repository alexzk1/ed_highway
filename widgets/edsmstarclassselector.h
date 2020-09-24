#pragma once

#include <QWidget>
#include <QStringList>
#include <QSet>

namespace Ui
{
    class EDSMStarClassSelector;
}

class EDSMStarClassSelector : public QWidget
{
    Q_OBJECT
public:
    explicit EDSMStarClassSelector(QWidget *parent = nullptr);
    ~EDSMStarClassSelector();
    static const QStringList& edsmStarClasses();
    bool isSelected(const QString& star_class) const;
    bool areLimitsInEffect() const;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::EDSMStarClassSelector *ui;
    void buildGui(bool multi_select = true);
    QSet<QString> selected;
};
