#ifndef CONDITIONAL_WAIT_H
#define CONDITIONAL_WAIT_H
#include "cm_ctors.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

// this one ensures that "confirm" will not be missed IF confirm() happened before waitConfirm()
//(that is nothing about password, pass = "passing")
class confirmed_pass
{
  private:
    std::unique_ptr<std::mutex> waitMtx;
    std::unique_ptr<std::condition_variable> cv;
    bool conf{false}; //"Effective C++ 14" - no need in atomical when is guarded by mutex

  public:
    using wait_lambda = std::function<bool()>;
    confirmed_pass() :
        waitMtx(new std::mutex()),
        cv(new std::condition_variable())
    {
    }
    ~confirmed_pass() = default;
    MOVEONLY_ALLOWED(confirmed_pass);

    void waitConfirm()
    {
        std::unique_lock<std::mutex> lck(*waitMtx);
        if (!conf)
        {
            cv->wait(lck, [this]() -> bool {
                return conf;
            });
        }
    }

    // returns false if timeouted (and unblocks thread!), should be used in while() GUI to process
    // events
    bool tryWaitConfirm(int millis)
    {
        std::unique_lock<std::mutex> lck(*waitMtx);
        if (!conf)
        {
            cv->wait_for(lck, std::chrono::milliseconds(millis), [this]() -> bool {
                return conf;
            });
        }
        return conf;
    }

    // allows to do additional check of extern stop pereodicaly
    void waitConfirm(const std::atomic<bool> &isStopped, int periodms = 500)
    {
        while (!tryWaitConfirm(periodms) && !isStopped)
        {
        }
    }

    void waitConfirm(const wait_lambda &isStopped, int periodms = 500)
    {
        while (!tryWaitConfirm(periodms) && !isStopped())
        {
        }
    }

    void confirm()
    {
        const std::unique_lock<std::mutex> lck(*waitMtx);
        conf = true;
        cv->notify_all();
    }
};

#endif // CONDITIONAL_WAIT_H
