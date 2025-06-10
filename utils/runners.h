#pragma once

#include "ctpl_stl.h"

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>

namespace utility {
using runner_t = std::shared_ptr<std::thread>;
using runnerint_t = std::shared_ptr<std::atomic<bool>>;

using runner_f_t = std::function<void(const runnerint_t should_int)>;

// simple way to execute lambda in thread, in case when shared_ptr is cleared it will send
// stop notify and join(), so I can ensure 1 pointer has only 1 running thread always for the same
// task
inline runner_t startNewRunner(runner_f_t func)
{
    auto stop = runnerint_t(new std::atomic<bool>(false));
    return runner_t(new std::thread(func, stop), [stop](auto p) {
        stop->store(true);
        if (p)
        {
            if (p->joinable())
                p->join();
            delete p;
        }
    });
}

inline size_t currentThreadId()
{
    return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

template <class Cont, class Callback>
void inline ForEachParallel(ctpl::thread_pool &pool, Cont &cont, const Callback &todo)
{
    std::atomic<size_t> totaldone{0};

    std::atomic<size_t> counter{0};
    const size_t slice = cont.size() / pool.size() + 1;

    for (auto it = cont.begin(); it != cont.end();)
    {
        const size_t dist = std::min<size_t>(std::distance(it, cont.end()), slice);
        auto itend = it;
        std::advance(itend, dist);
        ++counter;
        pool.push(
          [&counter, todo, it, itend, &totaldone](int tid) {
              (void)tid;
              try
              {
                  for (auto i = it; i != itend; ++i)
                  {
                      todo(*i);
                      ++totaldone;
                  }
              }
              catch (std::exception &e)
              {
                  std::cerr << "Exception into ForEachParallel: " << e.what() << std::endl;
              }
              catch (...)
              {
                  std::cerr << "Exception into ForEachParallel" << std::endl;
              }
              --counter;
          },
          []() {
              return true;
          });

        it = itend;
    }

    while (counter)
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    assert(totaldone == cont.size());
}
} // namespace utility
