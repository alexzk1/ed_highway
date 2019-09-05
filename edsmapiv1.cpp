#include "edsmapiv1.h"
#include "utils/exec_exit.h"
#include "utils/strutils.h"
#include "point.h"
#include <iostream>

#define ERROR(MSG) throw std::runtime_error(MSG)

EdsmApiV1::~EdsmApiV1()
{
    threads.stop(false);
}

static auto iteratorOrException(const nlohmann::json &object, const std::string& name)
{
    const auto it = object.find(name);
    if (it == object.end())
        ERROR(stringfmt("JSON object does not have field '%s'", name));
    return it;
}

void EdsmApiV1::executeRequest(const std::string &api, const RestClient::parameters &params, bool is_get, EdsmApiV1::callback_t callback)
{
    using namespace nlohmann;

    const auto url = (utility::strcontains(api, "https://")) ? api : stringfmt("https://www.edsm.net/api-v1/%s", api);
    const auto eparams{RestClient::encodePOSTParameters(params)};
    if (!is_get)
    {
        callback("POST not implemented yet", {});
        return;
    }

    ++working;

    const auto executor = [url, eparams, callback, is_get, this](auto)
    {
        exec_onexit ensure([this]()
        {
            --this->working;
        });
        (void)ensure;
        try
        {
            if (is_get)
            {
                const auto res_url{stringfmt("%s?%s", url, eparams)};
                //std::cout << res_url << std::endl;
                for (int i = 0; i < 50; ++i)
                {
                    const auto resp2{RestClient::get(res_url, 20)};
                    if (resp2.body.empty())
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    else
                    {
                        // std::cout << resp2.body << std::endl;
                        const auto j2{json::parse(resp2.body)};
                        callback("", j2.at(0)); //for some reason json parser adds extra array
                        return;
                    }
                }
                ERROR("Result didn't come in time.");
            }
        }
        catch (const std::exception& e)
        {
            callback(e.what(), {});
        }
    };

    threads.push(executor, []()
    {
        return true;
    });
}



Point EdsmApiV1::pointFromJson(const nlohmann::json &object)
{
    const auto cr = iteratorOrException(object, "coords");
    const auto get = [&cr](const std::string & f)
    {
        const auto it = iteratorOrException(*cr, f);
        return it->get<float>();
    };
    return Point{get("x"), get("y"), get("z")};
}
