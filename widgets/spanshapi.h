#ifndef SPANCHAPI_H
#define SPANCHAPI_H

#include "utils/cm_ctors.h"
#include "utils/ctpl_stl.h"
#include "utils/json.hpp"
#include "utils/restclient.h"

#include <functional>
#include <string>

class SpanshApi
{
  public:
    // 1st parameter is error if any
    using callback_t = std::function<void(std::string, nlohmann::json)>;

    SpanshApi(int threads_count) :
        threads(threads_count)
    {
    }
    ~SpanshApi();

    SpanshApi() = delete;
    NO_COPYMOVE(SpanshApi);
    void executeRequest(const std::string &api, const RestClient::parameters &params, bool has_job,
                        callback_t callback);

    template <class AnyType>
    void executeRequest(const AnyType &src, callback_t callback)
    {
        executeRequest(src.api(), src.params(), src.hasJob(), std::move(callback));
    }
    bool isWorking() const
    {
        return working > 0;
    }

  private:
    friend void dotest();
    ctpl::thread_pool threads;
    std::atomic<int32_t> working{0};
};

#endif // SPANCHAPI_H
