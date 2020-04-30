#pragma once
#include <QString>
#include <QVariant>
#include <string>
#include "utils/json.hpp"
#include "utils/cm_ctors.h"
#include "point.h"
#include "edsmwrapper.h"

struct NamedStarSystem
{
public:
    Point p;
    QString name;

public:
    NamedStarSystem() = default;
    DEFAULT_COPYMOVE(NamedStarSystem);
    static NamedStarSystem fromJsonInfo(const nlohmann::json& src)
    {
        NamedStarSystem r;
        r.p = Point::fromJson(src);
        r.name = QString::fromStdString(EDSMWrapper::valueFromJson<std::string>(src, "name"));
        return r;
    }

    operator const QString& () const noexcept
    {
        return name;
    }

    operator const QVariant () const noexcept
    {
        return name;
    }

    operator Point& () noexcept
    {
        return p;
    }

    operator const Point& () const noexcept
    {
        return p;
    }

    bool operator ==(const QString& v) const noexcept
    {
        return v == name;
    }
};
TEST_MOVE_NOEX(NamedStarSystem);
