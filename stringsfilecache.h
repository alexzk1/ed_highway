#pragma once
#include "lzokay.hpp"
#include "utils/cm_ctors.h"

#include <QCache>
#include <QMap>
#include <QPair>
#include <QString>
#include <QVector>

#include <stdint.h>

#include <mutex>

// this is string;string pairs which is saved to files on disk
// values are compressed, each value is in separated file with expire time
// something like memcache do, but on disk persistent

#define ONE_DAY_SECONDS (60u * 60u * 24u)

class StringsFileCache
{
  protected:
    using binary_blob = QPair<int, QVector<uint8_t>>; // int is original uncompressed size,
                                                      // compressor need it to unpack

    StringsFileCache();
    virtual ~StringsFileCache();

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
    bool addData(const QString &key, const QString &value,
                 uint32_t valid_for_seconds = ONE_DAY_SECONDS);

    // if data anyhow is corrupted or expired, or absent, it will drop this key and return empty
    QString getData(const QString &key);

    void cleanAll();

  private:
    using written_value_type = QPair<quint64, binary_blob>;

    std::mutex lock;
    lzokay::Dict<> dict;
    QMap<QString, QString> key2filename{}; // stores ky/filename

    QCache<QString, QString> ram_cache; // unlike above stores key/value-from-file

    binary_blob compress(const QString &value);
    const QString &getFileNameOrEmpty(const QString &key);
    void dumpListFile() const;
    void dropKey(const QString &key);
    void addToRam(const QString &key, const QString &value);
};
