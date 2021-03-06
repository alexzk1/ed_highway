#include <QUrl>
#include <iostream>

#include "edsmwrapper.h"
#include "stringsfilecache.h"
#include "edsmv1_nearest.h"
#include "edsmv1_sysinfo.h"
#include "dump_help.h"


template <class RequestCallable>
static std::vector<nlohmann::json> requestMany(const QStringList& names, const RequestCallable& request, const EDSMWrapper::progress_update& progress)
{
    std::shared_ptr<std::vector<nlohmann::json>> res(new std::vector<nlohmann::json>());
    std::shared_ptr<std::vector<confirmed_pass>> passes(new std::vector<confirmed_pass>());
    const size_t sz = names.size();
    if (sz)
    {
        res->resize(sz);
        passes->resize(sz);

        progress(0, sz);
        std::shared_ptr<std::atomic<bool>> canceled(new std::atomic<bool>(false));

        for (size_t i = 0; i < sz; ++i)
        {
            const auto task = [i, sz, res, passes, &progress, canceled](auto err, auto js)
            {
                if (!(*canceled))
                {
                    try
                    {
                        if (err.empty())
                            (*res)[i] = std::move(js);
                        if (progress(i, sz))
                        {
                            *canceled = true;
                            EDSMWrapper::api().clearAllPendings();
                            for (auto& v : *passes)
                                v.confirm();
                        }
                    }
                    catch (...)
                    {
                    }
                    (*passes)[i].confirm();
                }
            };
            request(names.at(i), task);
        }

        for (auto& p : *passes)
            p.waitConfirm();

        progress(sz, sz);
    }
    return std::move(*res);
}

static bool tryParseFromCache(const QString& key, const EDSMWrapper::callback_t& callback)
{
    QString js = StringsFileCache::get().getData(key);
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
void execRequest(const QString& key, const Req& src, int timeout, EDSMWrapper::callback_t callback, int days2keep = 1)
{
    const auto testr = [key, callback, days2keep](auto err, auto js)
    {
        if (err.empty())
            StringsFileCache::get().addData(key, QString::fromStdString(js.dump()), ONE_DAY_SECONDS * days2keep);

        callback(err, js);
    };
    try
    {
        EDSMWrapper::api().executeRequest(src, testr, timeout);
    }
    catch (...)
    {
        nlohmann::json jo;
        callback("EXCEPTION IN API!", jo);
    }
}

QString EDSMWrapper::getNameFromJson(const nlohmann::json &js)
{
    return QString::fromStdString(valueFromJson<std::string>(js, "name"));
}

void EDSMWrapper::selectSystemsInRadius(const QString &center_name, int radius, callback_t callback)
{
    const auto key = QStringLiteral("RADIUS_%1_%2").arg(center_name).arg(radius);
    if (!tryParseFromCache(key, callback))
    {
        EDSMV1NearerstSystem r(center_name.toStdString(), radius, false);
        execRequest(key, r, 20 + radius, std::move(callback));
    }
}

QStringList EDSMWrapper::selectSystemsInRadiusNamesOnly(const QString &center_name, int radius)
{
    QStringList names;

    confirmed_pass pass;

    selectSystemsInRadius(center_name, radius, [&names, &pass](auto err, auto js)
    {
        if (err.empty())
        {
            try
            {
                std::transform(std::begin(js), std::end(js), std::back_inserter(names), [](const auto & item)
                {
                    return getNameFromJson(item);
                });
            }
            catch (...)
            {
            }
        }
        pass.confirm();
    });
    pass.waitConfirm();
    return names;
}

std::vector<nlohmann::json> EDSMWrapper::requestManySysInfo(const QStringList &names, const EDSMWrapper::progress_update &progress)
{
    return std::move(requestMany(names, [](const auto & a, auto b)
    {
        requestSysInfo(a, b);
    }, progress));
}

std::vector<nlohmann::json> EDSMWrapper::requestManySysInfoInRadius(const QString &center_name, int radius, const progress_update &progress)
{
    const auto names = selectSystemsInRadiusNamesOnly(center_name, radius);
    return requestManySysInfo(names, progress);
}

void EDSMWrapper::requestSysInfo(const QString &sys_name, EDSMWrapper::callback_t callback)
{
    const auto key = QStringLiteral("SYSINFO_%1").arg(sys_name);
    if (!tryParseFromCache(key, callback))
    {
        EDSMV1SysInfo r(sys_name.toStdString());
        execRequest(key, r, 20, std::move(callback), 3);
    }
}

nlohmann::json EDSMWrapper::requestSysInfo(const QString &sys_name)
{
    nlohmann::json res;
    confirmed_pass pass;
    requestSysInfo(sys_name, [&res, &pass](auto err, auto js)
    {
        if (err.empty())
            res = std::move(js);
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

    const auto value_or_none = [&json](const std::string & root, const std::string & name) ->QString
    {
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
                return QStringLiteral("%1").arg(s);
            }
        }
        catch (...)
        {
        }
        return none;
    };

    return QStringLiteral("<p>Name: %10<br>Star Class: %9</p><hr><<p>Economy: %7<br>Second Economy: %8<hr><br>Allegiance: %1<br>Government: %2<br>Faction: %3<br>State: %4<br>Population: %5<br>Security: %6<br></p>")
           .arg(value_or_none("information", "allegiance"))
           .arg(value_or_none("information", "government"))
           .arg(value_or_none("information", "faction"))
           .arg(value_or_none("information", "factionState"))
           .arg(value_or_none("information", "population"))
           .arg(value_or_none("information", "security"))
           .arg(value_or_none("information", "economy"))
           .arg(value_or_none("information", "secondEconomy"))
           .arg(value_or_none("primaryStar", "type"))
           .arg(sys_name)
           ;
}

void EDSMWrapper::requestBodiesInfo(const QString &sys_name, EDSMWrapper::callback_t callback)
{
    const auto key = QStringLiteral("BODYINFO_%1").arg(sys_name);
    if (!tryParseFromCache(key, callback))
    {
        EDSMV1SysBodies r(sys_name.toStdString());
        execRequest(key, r, 20, std::move(callback), 3);
    }
}

nlohmann::json EDSMWrapper::requestBodiesInfo(const QString &sys_name)
{
    nlohmann::json res;
    confirmed_pass pass;
    requestBodiesInfo(sys_name, [&res, &pass](auto err, auto js)
    {
        if (err.empty())
            res = std::move(js);
        pass.confirm();
    });
    pass.waitConfirm();
    return res;
}

std::vector<nlohmann::json> EDSMWrapper::requestManyBodiesInfo(const QStringList &names, const EDSMWrapper::progress_update &progress)
{
    return std::move(requestMany(names, [](const auto & a, auto b)
    {
        requestBodiesInfo(a, b);
    }, progress));
}

std::vector<nlohmann::json> EDSMWrapper::requestManyBodiesInfoInRadius(const QString &center_name, int radius, const EDSMWrapper::progress_update &progress)
{
    const auto names = selectSystemsInRadiusNamesOnly(center_name, radius);
    return requestManyBodiesInfo(names, progress);
}

QString EDSMWrapper::getSystemUrl(const QString &systemName)
{
    const auto js = requestBodiesInfo(systemName);
    // std::cout << dump_helper::toStdStr(js) << std::endl;
    const auto id = valueFromJson<uint32_t>(js, "id");
    return QStringLiteral("https://www.edsm.net/en/system/id/%1/name/%2").arg(id).arg(QString(QUrl::toPercentEncoding(systemName)));
}
