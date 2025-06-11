#pragma once

#include "utils/cm_ctors.h"

#include <cereal/cereal.hpp>

#include <QCache>
#include <QMap>
#include <QPair>
#include <QString>
#include <QVector>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <istream>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

// this is string;string pairs which is saved to files on disk
// values are compressed, each value is in separated file with expire time
// something like memcache do, but on disk persistent

struct cache_dict
{
    std::map<std::string, std::string> key2filename;

    // support for Cereal
    template <class taArchive>
    void serialize(taArchive &ar, const std::uint32_t /*version*/)
    {
        ar(key2filename);
    }

    void remove(const std::string &key)
    {
        key2filename.erase(key);
    }

    void clear()
    {
        key2filename.clear();
    }

    QString getFileNameOrEmpty(const QString &key) const
    {
        const auto it = key2filename.find(key.toStdString());
        if (it != key2filename.end())
        {
            return QString::fromStdString(it->second);
        }
        return {};
    }
};
CEREAL_CLASS_VERSION(cache_dict, 1);

struct cache_compressed_blob
{
    // support for Cereal
    template <class taArchive>
    void serialize(taArchive &ar, const std::uint32_t /*version*/)
    {
        ar(unpacked_size, data, valid_till);
    }

    cache_compressed_blob(const QString &source, const int daysToKeep); // NOLINT
    explicit cache_compressed_blob(std::istream &inp);
    cache_compressed_blob() = delete;

    [[nodiscard]]
    bool isValidYet() const
    {
        return clock_t::now() < valid_till && !data.empty();
    }

    [[nodiscard]]
    std::optional<QString> unpackData() const;

  private:
    using clock_t = std::chrono::system_clock;

    clock_t::time_point valid_till;
    std::size_t unpacked_size{0u};
    std::vector<std::uint8_t> data;
};
CEREAL_CLASS_VERSION(cache_compressed_blob, 1);

class StringsFileCache
{
  protected:
    StringsFileCache();
    ~StringsFileCache();

  public:
    NO_COPYMOVE(StringsFileCache);
    STACK_ONLY;

    static inline StringsFileCache &instance()
    {
        static StringsFileCache cache;
        return cache;
    }

    static inline StringsFileCache &get()
    {
        return instance();
    }

    // set empty value to delete data
    bool addData(const QString &key, const QString &value, int daysToKeep = 1);

    // if data anyhow is corrupted or expired, or absent, it will drop this key and return empty
    QString getData(const QString &key);

    void cleanAll();

  private:
    void dumpListFile() const;
    void dropKey(const QString &key);
    void addToRam(const QString &key, const QString &value);

    std::mutex lock;
    cache_dict key2filename;            // stores ky/filename
    QCache<QString, QString> ram_cache; // unlike above stores key/value-from-file
};
