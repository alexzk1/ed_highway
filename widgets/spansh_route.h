#pragma once
#include "utils/restclient.h"
#include "utils/strfmt.h"

#include <cstdint>
#include <locale>

class SpanshRoutePostData
{
  private:
    RestClient::parameters p;

  public:
    SpanshRoutePostData(std::uint32_t eff, float range, const std::string &from,
                        const std::string &to)
    {
        const auto old = std::locale::global(std::locale::classic());

        p["efficiency"] = stringfmt("%u", eff);
        p["range"] = stringfmt("%02f", range);
        p["from"] = from;
        p["to"] = to;

        std::locale::global(old);
    }

    [[nodiscard]]
    const std::string &api() const
    {
        const static std::string r{"route"};
        return r;
    }

    [[nodiscard]]
    const auto &params() const
    {
        return p;
    }

    [[nodiscard]]
    constexpr bool hasJob() const
    {
        return true;
    }
};
