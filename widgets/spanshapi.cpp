#include "spanshapi.h"

#include "utils/exec_exit.h"
#include "utils/json.hpp"
#include "utils/restclient.h"
#include "utils/strfmt.h"
#include "utils/strutils.h"

#include <chrono>
#include <cstddef>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <utility>

/// Based on: https://github.com/chriszero/ED-Router/blob/master/libspanch/SpanchApi.cs

constexpr static std::size_t timeout_1st_request = 10; // seconds
constexpr static std::size_t timeout_2nd_request = 5;  // seconds

#define ERROR(MSG) throw std::runtime_error(MSG)

SpanshApi::~SpanshApi()
{
    threads.stop(false);
}

void SpanshApi::executeRequest(const std::string &api, const RestClient::parameters &params,
                               bool has_job, callback_t callback)
{
    using namespace nlohmann;

    const static std::string results_url{"https://spansh.co.uk/api/results"};
    const auto url{stringfmt("https://spansh.co.uk/api/%s", api)};
    const auto eparams{RestClient::encodePOSTParameters(params)};

    ++working;

    const auto executor = [url, eparams, callback = std::move(callback), has_job, this](auto) {
        const exec_onexit ensure([this]() {
            --this->working;
        });
        (void)ensure;

        try
        {
            const RestClient::headermap headers = {
              {"Content-Type", "application/x-www-form-urlencoded; charset=UTF-8"},
              {"Origin", "https://www.spansh.co.uk"},
              {"Referer", "https://www.spansh.co.uk/plotter/"},
              {"XRequestedWith", "XMLHttpRequest"},
              {"User=Agent",
               "Mozilla/5.0 (X11; Linux x86_64; rv:139.0) Gecko/20100101 Firefox/139.0"},
            };
            const auto resp1{RestClient::post(url, eparams, headers, timeout_1st_request)};
            if (resp1.body.empty())
            {
                ERROR("Job ID request returned empty body.");
            }

            static const auto isBodyQueued = [](const std::string &body) {
                return utility::strcontains(body, ":\"queued\"");
            };

            const auto j1{json::parse(resp1.body)};
            const auto &j1root = j1.at(0);
            if (has_job)
            {
                const auto jobit = j1root.find("job");
                if (jobit == j1root.end())
                {
                    ERROR("Job ID request does not return job id.");
                }

                const std::string jobid{jobit->get<std::string>()};
                if (jobid.empty())
                {
                    ERROR("JOBID is empty");
                }

                if (isBodyQueued(resp1.body))
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }

                const auto res_url{stringfmt("%s/%s", results_url, jobid)};

                for (int i = 0; i < 50; ++i)
                {
                    const auto resp2{RestClient::get(res_url, timeout_2nd_request)};
                    if (isBodyQueued(resp2.body))
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    }
                    else
                    {
                        const auto j2{json::parse(resp2.body)};
                        const auto &j2root = j2.at(0);
                        if (j2root.contains("error"))
                        {
                            std::cerr << "Full response from spansh web-site: \n"
                                      << j2root.dump(2) << std::endl;
                            ERROR(j2root["error"]);
                            return;
                        }
                        const auto rit = j2root.find("result");
                        if (rit == j2root.end())
                        {
                            ERROR("Could not detect result field in web-site's response.");
                            return;
                        }
                        callback("", *rit);
                        return;
                    }
                }
                ERROR("Result from the web-site didn't come in time.");
            }
            else
            {
                callback("", j1root);
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
