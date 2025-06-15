#include "stringsfilecache.h"

#include "utils/guard_on.h"
#include "utils/lzokay.hpp"
#include "writable_path.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>

#include <unistd.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <istream>
#include <optional>
#include <ostream>
#include <vector>

namespace {

template <class T>
void writeToStream(std::ostream &out, T &src)
{
    cereal::BinaryOutputArchive oa(out); // NOLINT
    oa(src);
    out.flush();
}

template <class T>
bool loadFromStream(std::istream &inp, T &dst)
{
    try
    {
        cereal::BinaryInputArchive ia(inp); // NOLINT
        ia(dst);
        return true;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return false;
}

QString getCacheDir()
{
    const static QString path = getWritableLocationApp() + "/cache";
    // std::cout << "File location: " << path.toStdString() << std::endl;
    return path;
}

QString getListFileName()
{
    const static QString path = getCacheDir() + "/list.bin";
    return path;
}
} // namespace

StringsFileCache::StringsFileCache() :
    ram_cache(500)
{
    bool need_delete_cache = true;
    {
        std::ifstream file(getListFileName().toStdString(), std::ios_base::in);
        need_delete_cache = !loadFromStream(file, key2filename);
    }
    QDir d(getCacheDir());
    if (need_delete_cache)
    {
        d.removeRecursively();
    }

    d.mkpath(getCacheDir());
}

void StringsFileCache::dumpListFile() const
{
    std::ofstream file(getListFileName().toStdString(), std::ios_base::out | std::ios_base::trunc);
    writeToStream(file, key2filename);
}

void StringsFileCache::dropKey(const QString &key)
{
    ram_cache.remove(key);

    const auto fn = key2filename.getFileNameOrEmpty(key);
    if (!fn.isEmpty())
    {
        QFile::remove(getCacheDir() + "/" + fn);
        key2filename.remove(key.toStdString());
    }
}

void StringsFileCache::addToRam(const QString &key, const QString &value)
{
    ram_cache.insert(key, new QString(value));
}

StringsFileCache::~StringsFileCache()
{
    LOCK_GUARD_ON(lock);
    dumpListFile();
}

static QString buildNumericFnPart()
{
    // we may have many copies running with no gui, for example user presses hot keys fast
    // so they must have different file names to save, lets do it time + pid
    const auto now = QDateTime::currentMSecsSinceEpoch();
    const auto pid = getpid();
    return QStringLiteral("%1_%2").arg(now).arg(pid); // NOLINT
}

bool StringsFileCache::addData(const QString &key, const QString &value,
                               std::chrono::hours timeToKeep)
{
    if (key.isEmpty())
    {
        return false;
    }

    bool result = false;
    LOCK_GUARD_ON(lock);

    if (value.isEmpty())
    {
        dropKey(key);
        result = true;
    }
    else
    {
        const QString filename = QStringLiteral("value_%1").arg(buildNumericFnPart());

        // Most unlikelly this will happen ... but user might change system clock or so and we dont
        // want to overwrite file.
        QString finalName = filename;
        for (int counter = 0; QFile::exists(getCacheDir() + "/" + finalName) && counter < 5000;
             ++counter)
        {
            finalName = QStringLiteral("%1_%2").arg(filename).arg(counter);
        }

        addToRam(key, value);
        const QString finalFullName = getCacheDir() + "/" + finalName;
        std::ofstream file(finalFullName.toStdString(), std::ios_base::out | std::ios_base::trunc);
        dropKey(key);
        key2filename.key2filename[key.toStdString()] = finalName.toStdString();

        cache_compressed_blob blob(value, timeToKeep);
        writeToStream(file, blob);
        result = true;
    }

    if (result)
    {
        static std::uint64_t cntr = 1;
        if ((cntr++) % 10 == 0)
        {
            dumpListFile();
        }
    }

    return result;
}

QString StringsFileCache::getData(const QString &key)
{
    const static QString empty;
    if (!key.isEmpty())
    {
        LOCK_GUARD_ON(lock);

        const auto rp = ram_cache[key];
        if (rp)
        {
            return *rp;
        }

        const auto fn = key2filename.getFileNameOrEmpty(key);
        if (!fn.isEmpty())
        {
            const QString fileName(getCacheDir() + "/" + fn);
            std::ifstream inp(fileName.toStdString(), std::ios_base::in);
            const cache_compressed_blob blob(inp);

            if (blob.isValidYet())
            {
                if (auto str = blob.unpackData())
                {
                    addToRam(key, *str);
                    return *str;
                }
            }
        }
        dropKey(key);
    }
    return empty;
}

void StringsFileCache::cleanAll()
{
    LOCK_GUARD_ON(lock);
    key2filename.clear();
    ram_cache.clear();

    QDir d(getCacheDir());
    d.removeRecursively();
    d.mkpath(getCacheDir());
}

cache_compressed_blob::cache_compressed_blob(const QString &source,
                                             const std::chrono::hours timeToKeep) :
    valid_till{clock_t::now() + timeToKeep}
{
    const auto src = source.toUtf8();
    const std::size_t estimated_size = lzokay::compress_worst_size(src.length());
    unpacked_size = src.length();
    data.resize(estimated_size);

    std::size_t compressed_size = 0u;

    lzokay::Dict<> dict;
    const auto error = lzokay::compress(bit_cast<const uint8_t *>(src.data()), unpacked_size,
                                        data.data(), estimated_size, compressed_size, dict);
    if (error < lzokay::EResult::Success)
    {
        data.clear();
        unpacked_size = 0;
        return;
    }
    assert(compressed_size <= estimated_size);
    // Trunc extra size not used.
    data.resize(compressed_size);
}

cache_compressed_blob::cache_compressed_blob(std::istream &inp)
{
    loadFromStream(inp, *this);
}

std::optional<QString> cache_compressed_blob::unpackData() const
{
    std::vector<std::uint8_t> decompressed;
    decompressed.resize(unpacked_size);

    std::size_t decompressed_size = 0u;
    const auto error = lzokay::decompress(data.data(), data.size(), decompressed.data(),
                                          unpacked_size, decompressed_size);
    if (error >= lzokay::EResult::Success)
    {
        assert(decompressed_size == unpacked_size);
        return QString::fromUtf8(bit_cast<const char *>(decompressed.data()),
                                 static_cast<int>(decompressed_size));
    }
    return std::nullopt;
}
