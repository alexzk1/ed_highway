#pragma once
#include "utils/floats_to_string_locale.h"
#include "utils/restclient.h"
#include "utils/strfmt.h"

#include <cstdint>
#include <string>

/// @brief Input parameters for neutron plotter.
struct SpanshRouteInputParams
{
    std::uint32_t efficiency;
    float ship_jump_range;
    std::string system_from;
    std::string system_to;
    std::uint32_t supercharg_multiplier{4};

    SpanshRouteInputParams &useCaspian()
    {
        supercharg_multiplier = 6;
        return *this;
    }
};

/// @brief Passed to executor will make proper web-api request based on supplied C++ values.
class SpanshRoutePostData
{
  private:
    RestClient::parameters p;

  public:
    explicit SpanshRoutePostData(const SpanshRouteInputParams &params)
    {
        // It must be "." (point) as int.float separator used. Web-site fails if comma used.
        const FloatsShouldUsePointAsString properRangeAsString;

        p["efficiency"] = stringfmt("%u", params.efficiency);
        p["range"] = stringfmt("%02f", params.ship_jump_range);
        p["from"] = params.system_from;
        p["to"] = params.system_to;
        p["supercharge_multiplier"] = stringfmt("%u", params.supercharg_multiplier);
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
