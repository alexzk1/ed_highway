#ifndef EDSMAPIV1_H
#define EDSMAPIV1_H
#include "utils/cm_ctors.h"
#include <functional>
#include <string>
#include "utils/restclient.h"
#include "utils/ctpl_stl.h"
#include "utils/json.hpp"
#include "point.h"

class EdsmApiV1
{
public:
    EdsmApiV1() = delete;
    NO_COPYMOVE(EdsmApiV1);
    EdsmApiV1(int threads_count) : threads(threads_count) {}
    ~EdsmApiV1();

    //1st parameter is error if any
    using callback_t = std::function<void(std::string, nlohmann::json)>;

    void executeRequest(const std::string& api, const RestClient::parameters& params, bool is_get, callback_t callback);

    template<class AnyType>
    void executeRequest(const AnyType& src, callback_t callback)
    {
        executeRequest(src.api(), src.params(), src.isGet(), std::move(callback));
    }
    bool isWorking() const
    {
        return working > 0;
    }
    static Point pointFromJson(const nlohmann::json& object);
private:
    friend void dotest();
    ctpl::thread_pool threads;
    std::atomic<int32_t> working{0};
};

#endif // EDSMAPIV1_H
