#pragma once
#include "utils/restclient.h"

#include <string>

class SpanshSysName
{
  private:
    RestClient::parameters p;

  public:
    explicit SpanshSysName(const std::string &templ)
    {
        p["q"] = templ;
    }

    [[nodiscard]]
    const std::string &api() const
    {
        const static std::string r{"systems"};
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
        return false;
    }
};
