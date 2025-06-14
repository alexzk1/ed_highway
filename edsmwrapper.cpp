#include "edsmwrapper.h" // IWYU pragma: keep

#include "edsmv1_nearest.h" // IWYU pragma: keep
#include "edsmv1_sysinfo.h" // IWYU pragma: keep
#include "qurl.h"
#include "stringsfilecache.h"       // IWYU pragma: keep
#include "utils/conditional_wait.h" // IWYU pragma: keep
#include "utils/json.hpp"

#include <QUrl>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace std::chrono_literals;

constexpr auto kTimeToKeepSysInfo = 24h;
constexpr auto kTimeToKeepBodiesInfo = 72h;

template <class RequestCallable>
std::vector<nlohmann::json> requestMany(const QStringList &names, const RequestCallable &request,
                                        const EDSMWrapper::progress_update &progress)
{
    const std::shared_ptr<std::vector<nlohmann::json>> res(new std::vector<nlohmann::json>());
    const std::shared_ptr<std::vector<confirmed_pass>> passes(new std::vector<confirmed_pass>());
    const std::size_t sz = names.size();
    if (sz)
    {
        res->resize(sz);
        passes->resize(sz);

        progress(0, sz);
        const std::shared_ptr<std::atomic<bool>> canceled(new std::atomic<bool>(false));

        for (std::size_t i = 0; i < sz; ++i)
        {
            const auto task = [i, sz, res, passes, &progress, canceled](const auto &err,
                                                                        const auto &js) {
                if (!(*canceled))
                {
                    try
                    {
                        if (err.empty())
                        {
                            (*res)[i] = std::move(js);
                        }
                        if (progress(i, sz))
                        {
                            *canceled = true;
                            EDSMWrapper::api().clearAllPendings();
                            for (auto &v : *passes)
                            {
                                v.confirm();
                            }
                        }
                    }
                    catch (...) // NOLINT
                    {
                    }
                    (*passes)[i].confirm();
                }
            };
            request(names.at(static_cast<int>(i)), task);
        }

        for (auto &p : *passes)
        {
            p.waitConfirm();
        }

        progress(sz, sz);
    }
    return std::move(*res);
}

bool tryParseFromCache(const QString &key, const EDSMWrapper::callback_t &callback)
{
    const QString js = StringsFileCache::get().getData(key);
    if (!js.isEmpty())
    {
        nlohmann::json jo;
        try
        {
            jo = nlohmann::json::parse(js.toStdString());
        }
        catch (...)
        {
            return false;
        }
        callback("", jo);
        return true;
    }
    return false;
}

template <class Req>
void execRequest(const QString &key, const Req &src, int timeout,
                 const EDSMWrapper::callback_t &callback, std::chrono::hours timeToKeep = 24h)
{
    const auto testr = [key, callback, timeToKeep](const auto &err, const auto &js) {
        if (err.empty())
        {
            StringsFileCache::get().addData(key, QString::fromStdString(js.dump()), timeToKeep);
        }
        callback(err, js);
    };
    try
    {
        EDSMWrapper::api().executeRequest(src, testr, timeout);
    }
    catch (...)
    {
        const nlohmann::json jo;
        callback("EXCEPTION IN API!", jo);
    }
}
} // namespace

QString EDSMWrapper::getNameFromJson(const nlohmann::json &js)
{
    return QString::fromStdString(valueFromJson<std::string>(js, "name"));
}

void EDSMWrapper::selectSystemsInRadius(const QString &center_name, int radius,
                                        const callback_t &callback)
{
    const auto key = QStringLiteral("RADIUS_%1_%2").arg(center_name).arg(radius); // NOLINT
    if (!tryParseFromCache(key, callback))
    {
        const EDSMV1NearerstSystem r(center_name.toStdString(), static_cast<float>(radius), false);
        execRequest(key, r, 20 + radius, callback);
    }
}

QStringList EDSMWrapper::selectSystemsInRadiusNamesOnly(const QString &center_name, int radius)
{
    QStringList names;

    confirmed_pass pass;

    selectSystemsInRadius(center_name, radius, [&names, &pass](const auto &err, const auto &js) {
        if (err.empty())
        {
            try
            {
                std::transform(std::begin(js), std::end(js), std::back_inserter(names),
                               [](const auto &item) {
                                   return getNameFromJson(item);
                               });
            }
            catch (...) // NOLINT
            {
            }
        }
        pass.confirm();
    });
    pass.waitConfirm();
    return names;
}

std::vector<nlohmann::json>
EDSMWrapper::requestManySysInfo(const QStringList &names,
                                const EDSMWrapper::progress_update &progress)
{
    return requestMany(
      names,
      [](const auto &a, const auto &b) {
          requestSysInfo(a, b);
      },
      progress);
}

std::vector<nlohmann::json> EDSMWrapper::requestManySysInfoInRadius(const QString &center_name,
                                                                    int radius,
                                                                    const progress_update &progress)
{
    const auto names = selectSystemsInRadiusNamesOnly(center_name, radius);
    return requestManySysInfo(names, progress);
}

void EDSMWrapper::requestSysInfo(const QString &sys_name, const EDSMWrapper::callback_t &callback)
{
    const auto key = QStringLiteral("SYSINFO_%1").arg(sys_name); // NOLINT
    if (!tryParseFromCache(key, callback))
    {
        const EDSMV1SysInfo r(sys_name.toStdString());
        execRequest(key, r, 20, callback, kTimeToKeepSysInfo);
    }
}

nlohmann::json EDSMWrapper::requestSysInfo(const QString &sys_name)
{
    nlohmann::json res;
    confirmed_pass pass;
    requestSysInfo(sys_name, [&res, &pass](const auto &err, auto js) {
        if (err.empty())
        {
            res = std::move(js);
        }
        pass.confirm();
    });
    pass.waitConfirm();
    return res;
}

/*
 *

{
  "coords": {
    "x": 149.96875,
    "y": -9.625,
    "z": 9.0625
  },
  "coordsLocked": true,
  "information": {
    "allegiance": "Federation",
    "economy": "Colony",
    "faction": "New Chonsu Alliance",
    "factionState": "Boom",
    "government": "Confederacy",
    "population": 35666,
    "reserve": "Common",
    "secondEconomy": "Extraction",
    "security": "Low"
  },
  "name": "Brigh",
  "primaryStar": {
    "isScoopable": true,
    "name": "Brigh",
    "type": "F (White) Star"
  },
  "requirePermit": false
}

 *
 * */
QString EDSMWrapper::tooltipWithSysInfo(const QString &sys_name)
{
    nlohmann::json json = requestSysInfo(sys_name);

    const auto value_or_none = [&json](const std::string &root,
                                       const std::string &name) -> QString {
        const static QString none = "-";
        try
        {
            auto r = valueFromJson<nlohmann::json>(json, root);
            if (name != "population")
            {
                auto s = valueFromJson<std::string>(r, name);
                return QString::fromStdString(s);
            }
            else
            {
                auto s = valueFromJson<uint64_t>(r, name);
                return QStringLiteral("%1").arg(s); // NOLINT
            }
        }
        catch (...) // NOLINT
        {
        }
        return none;
    };

    // NOLINTNEXTLINE
    return QStringLiteral("<p>Name: %10<br>Star Class: %9</p><hr><<p>Economy: %7<br>Second "
                          "Economy: %8<hr><br>Allegiance: %1<br>Government: %2<br>Faction: "
                          "%3<br>State: %4<br>Population: %5<br>Security: %6<br></p>")
      .arg(value_or_none("information", "allegiance"))
      .arg(value_or_none("information", "government"))
      .arg(value_or_none("information", "faction"))
      .arg(value_or_none("information", "factionState"))
      .arg(value_or_none("information", "population"))
      .arg(value_or_none("information", "security"))
      .arg(value_or_none("information", "economy"))
      .arg(value_or_none("information", "secondEconomy"))
      .arg(value_or_none("primaryStar", "type"))
      .arg(sys_name);
}

void EDSMWrapper::requestBodiesInfo(const QString &sys_name,
                                    const EDSMWrapper::callback_t &callback)
{
    const auto key = QStringLiteral("BODYINFO_%1").arg(sys_name); // NOLINT
    if (!tryParseFromCache(key, callback))
    {
        const EDSMV1SysBodies r(sys_name.toStdString());
        execRequest(key, r, 20, callback, kTimeToKeepBodiesInfo);
    }
}

nlohmann::json EDSMWrapper::requestBodiesInfo(const QString &sys_name)
{
    nlohmann::json res;
    confirmed_pass pass;
    requestBodiesInfo(sys_name, [&res, &pass](const auto &err, const auto &js) {
        if (err.empty())
        {
            res = js;
        }
        pass.confirm();
    });
    pass.waitConfirm();
    return res;
}

std::vector<nlohmann::json>
EDSMWrapper::requestManyBodiesInfo(const QStringList &names,
                                   const EDSMWrapper::progress_update &progress)
{
    return requestMany(
      names,
      [](const auto &a, const auto &b) {
          requestBodiesInfo(a, b);
      },
      progress);
}

std::vector<nlohmann::json>
EDSMWrapper::requestManyBodiesInfoInRadius(const QString &center_name, int radius,
                                           const EDSMWrapper::progress_update &progress)
{
    const auto names = selectSystemsInRadiusNamesOnly(center_name, radius);
    return requestManyBodiesInfo(names, progress);
}

QString EDSMWrapper::getSystemUrl(const QString &systemName)
{
    const auto js = requestBodiesInfo(systemName);
    // std::cout << dump_helper::toStdStr(js) << std::endl;
    const auto id = valueFromJson<uint32_t>(js, "id");
    return QStringLiteral("https://www.edsm.net/en/system/id/%1/name/%2") // NOLINT
      .arg(id)
      .arg(QString::fromUtf8(QUrl::toPercentEncoding(systemName)));
}
