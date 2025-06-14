#pragma once
#include "utils/floats_to_string_locale.h"
#include "utils/restclient.h"
#include "utils/strfmt.h"

#include <cstdint>

/// @brief Passed to executor will make proper web-api request based on supplied C++ values.
class SpanshRoutePostData
{
  private:
    RestClient::parameters p;

  public:
    SpanshRoutePostData(const std::uint32_t eff, const float range, const std::string &from,
                        const std::string &to)
    {
        // It must be "." (point) as int.float separator used. Web-site fails if comma used.
        const FloatsShouldUsePointAsString properRangeAsString;

        p["efficiency"] = stringfmt("%u", eff);
        p["range"] = stringfmt("%02f", range);
        p["from"] = from;
        p["to"] = to;
    }

    /// @returns API endpoint for spansh web-site.
    [[nodiscard]]
    const std::string &api() const
    {
        const static std::string r{"route"};
        return r;
    }

    /// @returns POST data for API endpoint on spansh web-site.
    [[nodiscard]]
    const auto &params() const
    {
        return p;
    }

    /// @returns true if it should be followed by GET.
    [[nodiscard]]
    constexpr bool hasJob() const
    {
        return true;
    }
};
