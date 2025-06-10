#ifndef EDSMV1_NEAREST_H
#define EDSMV1_NEAREST_H

#include "point.h"
#include "utils/restclient.h"
#include "utils/strfmt.h"

#include <string>

class EDSMV1NearerstSystem
{
  private:
    bool iscube{false};
    RestClient::parameters p;

  public:
    EDSMV1NearerstSystem() = delete;

    EDSMV1NearerstSystem(const Point &src, float size_or_radius, bool iscube = false) :
        iscube(iscube)
    {
        p = src.toParams();
        addSize(size_or_radius);
    }

    EDSMV1NearerstSystem(const std::string &src, float size_or_radius, bool iscube = false) :
        iscube(iscube)
    {
        p["systemName"] = src;
        addSize(size_or_radius);
    }

    [[nodiscard]]
    constexpr bool isGet() const
    {
        return true;
    }

    [[nodiscard]]
    const auto &params() const
    {
        return p;
    }

    [[nodiscard]]
    const std::string &api() const
    {
        const static std::string cube{"cube-systems"};
        const static std::string sphere{"sphere-systems"};
        return (iscube) ? cube : sphere;
    }

  private:
    void addSize(float size_or_radius)
    {
        const auto v = stringfmt("%0.4f", size_or_radius);
        if (iscube)
        {
            p["size"] = v;
        }
        else
        {
            p["radius"] = v;
            p["minRadius"] = "0";
        }
    }
};

#endif // EDSMV1_NEAREST_H
