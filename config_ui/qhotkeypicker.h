#ifndef QHOTKEYPICKER_H
#define QHOTKEYPICKER_H
#include <QPushButton>
#include <QTimer>

class QHotkeyPicker : public QPushButton
{
    Q_OBJECT
  private:
    QString hot_string;
    QTimer *timer{nullptr};
    int seconds_left;

    void showSeconds();

  public:
    QHotkeyPicker(QWidget *owner = nullptr);
    void setHot(const QString &v);
    QString getHot() const;

  protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
  signals:
    void hotStringChanged(const QString &);
};

#endif // QHOTKEYPICKER_H
