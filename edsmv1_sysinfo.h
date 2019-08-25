#ifndef EDSMV1_SYSINFO_H
#define EDSMV1_SYSINFO_H
#include "utils/restclient.h"

class EDSMV1SysInfo
{
    RestClient::parameters p;
public:
    EDSMV1SysInfo() = delete;

    EDSMV1SysInfo(const std::string& name)
    {
        p["systemName"] = name;
        p["showCoordinates"] = "1";
        p["showPermit"] = "1";
        p["showInformation"] = "1";
        p["showPrimaryStar"] = "1";
    }

    constexpr bool isGet() const
    {
        return true;
    }

    const auto& params() const
    {
        return p;
    }

    const std::string& api() const
    {
        const static std::string v{"system"};
        return v;
    }
};

#endif // EDSMV1_SYSINFO_H
