#pragma once

#include "json.hpp"

#include <QString>

#include <iostream>
#include <string>

namespace dump_helper {

inline std::string toStdStr(const QString &src)
{
    return src.toStdString();
}

inline std::string toStdStr(const nlohmann::json &src)
{
    return src.dump(2);
}

template <class Anything>
inline std::string toStdStr(const Anything &src)
{
    return src; // use implicit conversion if any there
}

template <class Cont>
void dumpContainer(const Cont &src)
{
    for (const auto &v : src)
        std::cout << toStdStr(v) << std::endl;
}
} // namespace dump_helper
