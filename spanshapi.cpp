#include "spanshapi.h"
#include "strfmt.h"
#include <iostream>
#include <chrono>
#include "strutils.h"

constexpr static size_t timeout_1st_request = 10; //seconds
constexpr static size_t timeout_2nd_request = 5; //seconds

#define ERROR(MSG) throw std::runtime_error(MSG)

void SpanshApi::executeRequest(const std::string &api, const RestClient::parameters &params, bool has_job, callback_t callback)
{
    using namespace nlohmann;

    const static std::string results_url{"https://spansh.co.uk/api/results"};
    const std::string url{stringfmt("https://spansh.co.uk/api/%s", api)};
    const std::string eparams{RestClient::encodePOSTParameters(params)};

    const auto executor = [url, eparams, callback, has_job](auto)
    {
        try
        {
            const auto resp1{RestClient::post(url, eparams, timeout_1st_request)};
            if (resp1.body.empty())
                ERROR("Job ID request returned empty body.");

            const auto j1{json::parse(resp1.body)};
            const auto& j1root = j1.at(0);
            if (has_job)
            {
                const auto jobit = j1root.find("job");
                if (jobit == j1root.end())
                    ERROR("Job ID request does not return job id.");

                const std::string jobid{jobit->get<std::string>()};
                if (jobid.empty())
                    ERROR("JOBID is empty");

                //const auto job_params{RestClient::encodePOSTParameters({{"job", jobid}})};
                const auto res_url{stringfmt("%s/%s", results_url, jobid)};

                for (int i = 0; i < 50; ++i)
                {
                    const auto resp2{RestClient::get(res_url, timeout_2nd_request)};
                    if (utility::strcontains(resp2.body, ":\"queued\""))
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    else
                    {
                        const auto j2{json::parse(resp2.body)};
                        const auto& j2root = j2.at(0);
                        const auto rit = j2root.find("result");
                        if (rit == j2root.end())
                            ERROR("Could not detect result field.");
                        callback("", *rit);
                        return;
                    }
                }
                ERROR("Result didn't come in time.");
            }
            else
                callback("", j1root);
        }
        catch (std::exception& e)
        {
            callback(e.what(), {});
        }
    };

    threads.push(executor, []()
    {
        return true;
    });
}
