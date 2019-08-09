#ifndef SPANCHAPI_H
#define SPANCHAPI_H

#include "cm_ctors.h"
#include <functional>
#include <string>
#include "restclient.h"
#include "ctpl_stl.h"
#include "json.hpp"

class SpanshApi
{
public:
    //1st parameter is error if any
    using callback_t = std::function<void(std::string, nlohmann::json)>;

    SpanshApi() = default;
    NO_COPYMOVE(SpanshApi);
    void executeRequest(const std::string& api, const RestClient::parameters& params, callback_t callback);

    template<class AnyType>
    void executeRequest(const AnyType& src, callback_t callback)
    {
        executeRequest(src.api(), src.params(), std::move(callback));
    }

private:
    friend void dotest();
    ctpl::thread_pool threads{5};
};

#endif // SPANCHAPI_H
