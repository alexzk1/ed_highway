#pragma once
#include "edsmapiv1.h"
#include "utils/conditional_wait.h"

#include <QString>
#include <QStringList>

#include <atomic>
#include <memory>

// this class solves some exact tasks, using caching, so do not stress edsm too much

class EDSMWrapper
{
  public:
    using callback_t = EdsmApiV1::callback_t;
    using progress_update = std::function<bool(size_t, size_t)>; // 1st current value, 2nd total
    EDSMWrapper() = delete;

    static QString getNameFromJson(const nlohmann::json &js);

    // does not block caller thread, callback executed in other thread scope (not the caller) if
    // network request made or immediatly in caller thread if cache used
    static void selectSystemsInRadius(const QString &center_name, int radius, callback_t callback);

    // blocks caller thread until have return value, may return empty list
    static QStringList selectSystemsInRadiusNamesOnly(const QString &center_name, int radius);

    // blocks caller thread until all done
    static std::vector<nlohmann::json> requestManySysInfo(
      const QStringList &names, const progress_update &progress = [](auto, auto) {
          return false;
      });
    static std::vector<nlohmann::json> requestManySysInfoInRadius(
      const QString &center_name, int radius, const progress_update &progress = [](auto, auto) {
          return false;
      });

    // does not block caller thread, callback executed in other thread scope (not the caller) if
    // network request made or immediatly in caller thread if cache used
    static void requestSysInfo(const QString &sys_name, callback_t callback);

    // blocks caller thread
    static nlohmann::json requestSysInfo(const QString &sys_name);

    // blocks caller thread
    static QString tooltipWithSysInfo(const QString &sys_name);

    // does not block caller thread, callback executed in other thread scope (not the caller) if
    // network request made or immediatly in caller thread if cache used
    static void requestBodiesInfo(const QString &sys_name, callback_t callback);

    // blocks caller thread
    static nlohmann::json requestBodiesInfo(const QString &sys_name);

    // blocks caller thread until all done
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
            throw std::runtime_error(stringfmt("JSON object does not have field '%s'", fieldName));
        return it->get<Res>();
    }

    static EdsmApiV1 &api()
    {
        static EdsmApiV1 edsm((int)std::max(1u, std::thread::hardware_concurrency() / 2u));
        return edsm;
    }

    static QString getSystemUrl(const QString &systemName);
};
