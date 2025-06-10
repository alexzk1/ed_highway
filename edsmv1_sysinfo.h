#ifndef EDSMV1_SYSINFO_H
#define EDSMV1_SYSINFO_H

#include "utils/restclient.h"

#include <string>

class EDSMV1SysInfo
{
  public:
    EDSMV1SysInfo() = delete;

    explicit EDSMV1SysInfo(const std::string &name)
    {
        restParams["systemName"] = name;
        restParams["showCoordinates"] = "1";
        restParams["showPermit"] = "1";
        restParams["showInformation"] = "1";
        restParams["showPrimaryStar"] = "1";
    }

    [[nodiscard]]
    constexpr bool isGet() const
    {
        return true;
    }

    [[nodiscard]]
    const auto &params() const
    {
        return restParams;
    }

    [[nodiscard]]
    const std::string &api() const
    {
        const static std::string v{"system"};
        return v;
    }

  private:
    RestClient::parameters restParams;
};

class EDSMV1SysBodies
{
  public:
    EDSMV1SysBodies() = delete;

    explicit EDSMV1SysBodies(const std::string &name)
    {
        restParams["systemName"] = name;
    }

    [[nodiscard]]
    constexpr bool isGet() const
    {
        return true;
    }

    [[nodiscard]]
    const auto &params() const
    {
        return restParams;
    }

    [[nodiscard]]
    const std::string &api() const
    {
        const static std::string v{"https://www.edsm.net/api-system-v1/bodies"};
        return v;
    }

  private:
    RestClient::parameters restParams;
};

#endif // EDSMV1_SYSINFO_H
