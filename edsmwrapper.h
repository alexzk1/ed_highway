#pragma once

#include "edsmapiv1.h"              // IWYU pragma: keep
#include "utils/conditional_wait.h" // IWYU pragma: keep
#include "utils/json.hpp"
#include "utils/strfmt.h"

#include <QString>
#include <QStringList>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

/// @brief Top level API to use to query EDSM from C++. Hides http details in lower levels calls.
/// It tries to use cache to avoid too often http calls.
class EDSMWrapper
{
  public:
    using callback_t = EdsmApiV1::callback_t;
    using progress_update =
      std::function<bool(std::size_t, std::size_t)>; // 1st current value, 2nd total
    EDSMWrapper() = delete;

    /// @returns value of the field "name" in JSON.
    static QString getNameFromJson(const nlohmann::json &js);

    /// @brief Does not block caller thread, callback executed in other thread scope (not the
    /// caller) if network request made or immediatly in caller thread if cache was used.
    static void selectSystemsInRadius(const QString &center_name, int radius,
                                      const callback_t &callback);

    /// @brief Blocks caller thread until have return value, may return empty list.
    static QStringList selectSystemsInRadiusNamesOnly(const QString &center_name, int radius);

    /// @brief Blocks caller thread until all done.
    static std::vector<nlohmann::json> requestManySysInfo(
      const QStringList &names, const progress_update &progress = [](auto, auto) {
          return false;
      });
    static std::vector<nlohmann::json> requestManySysInfoInRadius(
      const QString &center_name, int radius, const progress_update &progress = [](auto, auto) {
          return false;
      });

    /// @brief Does not block caller thread, callback executed in other thread scope (not the
    /// caller) if network request made or immediatly in caller thread if cache was used.
    static void requestSysInfo(const QString &sys_name, const callback_t &callback);

    /// @brief blocks caller thread
    static nlohmann::json requestSysInfo(const QString &sys_name);

    /// @brief blocks caller thread
    static QString tooltipWithSysInfo(const QString &sys_name);

    /// @brief Does not block caller thread, callback executed in other thread scope (not the
    /// caller) if network request made or immediatly in caller thread if cache was used.
    static void requestBodiesInfo(const QString &sys_name, const callback_t &callback);

    /// @brief blocks caller thread
    static nlohmann::json requestBodiesInfo(const QString &sys_name);

    /// @brief blocks caller thread
    static std::vector<nlohmann::json> requestManyBodiesInfo(
      const QStringList &names, const progress_update &progress = [](auto, auto) {
          return false;
      });
    static std::vector<nlohmann::json> requestManyBodiesInfoInRadius(
      const QString &center_name, int radius, const progress_update &progress = [](auto, auto) {
          return false;
      });

    template <class Res>
    static Res valueFromJson(const nlohmann::json &object, const std::string &fieldName)
    {
        const auto it = object.find(fieldName);
        if (it == object.end())
        {
            throw std::runtime_error(stringfmt("JSON object does not have field '%s'", fieldName));
        }
        return it->get<Res>();
    }

    /// @returns static threaded executor for http-API calls.
    static EdsmApiV1 &api()
    {
        static EdsmApiV1 edsm(
          static_cast<int>(std::max(1u, std::thread::hardware_concurrency() / 2u)));
        return edsm;
    }

    /// @returns GET url for "system" endpoint of the web-API.
    static QString getSystemUrl(const QString &systemName);
};
