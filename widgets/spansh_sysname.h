#pragma once
#include <string>
#include "utils/restclient.h"
#include "utils/strfmt.h"

class SpanshSysName
{
private:
    RestClient::parameters p;
public:
    SpanshSysName(const std::string& templ)
    {
        p["q"] = templ;
    }

    const std::string& api() const
    {
        const static std::string r{"systems"};
        return r;
    }

    const auto& params() const
    {
        return p;
    }

    constexpr bool hasJob() const
    {
        return false;
    }
};
