#pragma once

#include "utils/cm_ctors.h"
#include "utils/ctpl_stl.h"
#include "utils/json.hpp"
#include "utils/restclient.h"

#include <atomic>
#include <cstddef>
#include <functional>
#include <string>

class EdsmApiV1
{
  public:
    EdsmApiV1() = delete;
    NO_COPYMOVE(EdsmApiV1);
    explicit EdsmApiV1(int threads_count) :
        threads(threads_count)
    {
    }
    ~EdsmApiV1();

    // 1st parameter is error if any
    using callback_t = std::function<void(std::string, nlohmann::json)>;

    void executeRequest(const std::string &api, const RestClient::parameters &params, bool is_get,
                        const callback_t &callback, int timeout_seconds = 20);

    template <class AnyType>
    void executeRequest(const AnyType &src, const callback_t &callback, int timeout_seconds = 20)
    {
        executeRequest(src.api(), src.params(), src.isGet(), callback, timeout_seconds);
    }
    bool isWorking() const
    {
        return working > 0;
    }

    std::size_t tasksCount() const
    {
        return threads.tasksCount();
    }

    void clearAllPendings();

  private:
    friend void dotest();
    ctpl::thread_pool threads;
    std::atomic<int32_t> working{0};
};
