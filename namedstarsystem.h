#pragma once
#include "edsmwrapper.h"
#include "point.h"
#include "utils/cm_ctors.h"
#include "utils/json.hpp"

#include <QString>
#include <QVariant>

#include <string>

struct NamedStarSystem
{
  public:
    Point p;
    QString name;
    bool blank{true};

  public:
    NamedStarSystem() = default;
    DEFAULT_COPYMOVE(NamedStarSystem);
    static NamedStarSystem fromJsonInfo(const nlohmann::json &src)
    {
        NamedStarSystem r;
        try
        {
            r.p = Point::fromJson(src);
            r.name = QString::fromStdString(EDSMWrapper::valueFromJson<std::string>(src, "name"));
            r.blank = false;
        }
        catch (...)
        {
        }

        return r;
    }

    operator const QString &() const noexcept
    {
        return name;
    }

    operator const QVariant() const noexcept
    {
        return name;
    }

    operator Point &() noexcept
    {
        return p;
    }

    operator const Point &() const noexcept
    {
        return p;
    }

    bool operator==(const QString &v) const noexcept
    {
        return v == name;
    }
};
TEST_MOVE_NOEX(NamedStarSystem);
