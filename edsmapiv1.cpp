#include "edsmapiv1.h"

#include "utils/exec_exit.h"
#include "utils/strutils.h"

#include <thread>

#define ERROR(MSG) throw std::runtime_error(MSG)

EdsmApiV1::~EdsmApiV1()
{
    threads.stop(true);
}

void EdsmApiV1::executeRequest(const std::string &api, const RestClient::parameters &params,
                               bool is_get, EdsmApiV1::callback_t callback, int timeout_seconds)
{
    using namespace nlohmann;

    const auto url = (utility::strcontains(api, "https://"))
                       ? api
                       : stringfmt("https://www.edsm.net/api-v1/%s", api);

    // TODO: uncoment to print GET link
    // std::cout << "API: " << url << std::endl;

    const auto eparams{RestClient::encodePOSTParameters(params)};
    if (!is_get)
    {
        callback("POST not implemented yet", {});
        return;
    }

    ++working;

    const auto executor = [url, eparams, callback, is_get, this, timeout_seconds](auto id) {
        (void)id;
        // std::cout << "Running task on thread " << id <<  "Tasks in q: " << tasksCount() <<
        // std::endl;
        exec_onexit ensure([this]() {
            --this->working;
        });
        (void)ensure;
        try
        {
            if (is_get)
            {
                const auto res_url{stringfmt("%s?%s", url, eparams)};
                for (int i = 0; i < 50; ++i)
                {
                    const auto resp2{RestClient::get(res_url, timeout_seconds)};
                    if (resp2.body.empty())
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    else
                    {
                        const auto j2{json::parse(resp2.body)};
                        callback("", j2.at(0)); // for some reason json parser adds extra array
                        return;
                    }
                }
                ERROR("Result didn't come in time.");
            }
        }
        catch (const std::exception &e)
        {
            callback(e.what(), {});
        }
    };

    threads.push(executor, []() {
        return true;
    });
}

void EdsmApiV1::clearAllPendings()
{
    threads.clear_queue();
}
