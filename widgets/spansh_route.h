#pragma once
#include "utils/restclient.h"
#include "utils/strfmt.h"

#include <string>

class SpanshRoute
{
  private:
    RestClient::parameters p;

  public:
    SpanshRoute(uint32_t eff, float range, const std::string &from, const std::string &to)
    {
        p["efficiency"] = stringfmt("%u", eff);
        p["range"] = stringfmt("%02f", range);
        p["from"] = from;
        p["to"] = to;
    }

    const std::string &api() const
    {
        const static std::string r{"route"};
        return r;
    }

    const auto &params() const
    {
        return p;
    }

    constexpr bool hasJob() const
    {
        return true;
    }
};
