#ifndef EXECONMAINTHREAD_H
#define EXECONMAINTHREAD_H

#include <QObject>

#include <qtmetamacros.h>

#include <functional>

using SimpleVoidFunction = std::function<void()>;

class ExecOnMainThread : public QObject
{
    Q_OBJECT
    explicit ExecOnMainThread(QObject *parent = nullptr);

  public:
    void exec(SimpleVoidFunction func) const;
    static const ExecOnMainThread &get();
  signals:
    // NOLINTNEXTLINE(const-signal-or-slot)
    void needExec(SimpleVoidFunction lambda) const;
  private slots:
    void doExex(const SimpleVoidFunction &lambda) const;
};

#endif // EXECONMAINTHREAD_H
