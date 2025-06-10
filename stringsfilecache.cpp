#include "stringsfilecache.h"

#include "utils/guard_on.h"
#include "writable_path.h"

#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>

#include <unistd.h>

#include <iostream>

constexpr static quint32 list_file_version = 0x01;
constexpr static auto v1_stream_version = QDataStream::Qt_5_0;
constexpr static auto current_write_stream_version = v1_stream_version;

template <class T>
void writeToFile(QFile &where, const T &src)
{
    QDataStream out(&where);
    // Write a header with a "magic number" and a version
    out << (quint32)0xA0B0C0D0;
    out << list_file_version;
    out.setVersion(current_write_stream_version);
    // Write the data
    out << src;
    where.flush();
}

template <class T>
bool loadFromFile(QFile &file, T &dst)
{
    QDataStream in(&file);
    // Read and check the header
    quint32 magic;
    in >> magic;
    if (magic == 0xA0B0C0D0)
    {
        // Read the version
        qint32 version;
        in >> version;
        if (version == list_file_version)
        {
            in.setVersion(v1_stream_version);
            // Read the data
            in >> dst;
            return true;
        }
    }
    return false;
}

static QString getCacheDir()
{
    const static QString path = getWritableLocationApp() + "/cache";
    // std::cout << "File location: " << path.toStdString() << std::endl;
    return path;
}

static QString getListFileName()
{
    const static QString path = getCacheDir() + "/list.bin";
    return path;
}

StringsFileCache::StringsFileCache() :
    ram_cache(500)
{
    bool need_delete_cache = true;
    {
        QFile file(getListFileName());
        if (file.open(QIODevice::ReadOnly))
            need_delete_cache = !loadFromFile(file, key2filename);
    }
    QDir d(getCacheDir());
    if (need_delete_cache)
        d.removeRecursively();

    d.mkpath(getCacheDir());
}

void StringsFileCache::dumpListFile() const
{
    QFile file(getListFileName());
    if (file.open(QIODevice::WriteOnly))
        writeToFile(file, key2filename);
}

void StringsFileCache::dropKey(const QString &key)
{
    ram_cache.remove(key);

    const auto &fn = getFileNameOrEmpty(key);
    if (!fn.isEmpty())
    {
        QFile::remove(getCacheDir() + "/" + fn);
        key2filename.remove(key);
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
    const auto now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    const auto pid = getpid();
    return QStringLiteral("%1_%2").arg(now).arg(pid);
}

bool StringsFileCache::addData(const QString &key, const QString &value, uint32_t valid_for_seconds)
{
    if (key.isEmpty())
        return false;

    bool result = false;
    LOCK_GUARD_ON(lock);
    if (value.isEmpty())
    {
        dropKey(key);
        result = true;
    }
    else
    {
        const uint64_t valid_till =
          QDateTime::currentDateTime().toMSecsSinceEpoch() + (uint64_t)valid_for_seconds * 1000u;
        const auto compressed = written_value_type{valid_till, compress(value)};

        const QString filename = QStringLiteral("value_%1").arg(buildNumericFnPart());
        QString finalName = filename;

        // most unlikelly this will happen ... but user might change system clock or so and we dont
        // want to overwrite file
        for (int counter = 0; QFile::exists(getCacheDir() + "/" + finalName) && counter < 5000;
             ++counter)
            finalName = QStringLiteral("%1_%2").arg(filename).arg(counter);

        addToRam(key, value);
        QFile file(getCacheDir() + "/" + finalName);
        if (file.open(QIODevice::WriteOnly))
        {
            dropKey(key);
            writeToFile(file, compressed);
            key2filename[key] = finalName;

            result = true;
        }
    }

    if (result)
    {
        static uint64_t cntr = 1;
        if ((cntr++) % 10 == 0)
            dumpListFile();
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
            return *rp;

        const auto &fn = getFileNameOrEmpty(key);
        if (!fn.isEmpty())
        {
            QFile file(getCacheDir() + "/" + fn);
            if (file.open(QIODevice::ReadOnly))
            {
                written_value_type v;
                if (loadFromFile(file, v))
                {
                    if (static_cast<uint64_t>(QDateTime::currentDateTime().toMSecsSinceEpoch())
                        < v.first)
                    {
                        std::unique_ptr<uint8_t[]> decompressed(new uint8_t[v.second.first]);
                        std::size_t decompressed_size;
                        const auto error =
                          lzokay::decompress(v.second.second.data(), v.second.second.length(),
                                             decompressed.get(), v.second.first, decompressed_size);
                        if (error >= lzokay::EResult::Success)
                        {
                            const auto t =
                              QString::fromUtf8((char *)decompressed.get(), decompressed_size);
                            addToRam(key, t);
                            return t;
                        }
                    }
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

StringsFileCache::binary_blob StringsFileCache::compress(const QString &value)
{
    lzokay::EResult error;
    const auto src = value.toUtf8();
    const size_t estimated_size = lzokay::compress_worst_size(src.length());
    StringsFileCache::binary_blob compressed(src.length(), QVector<uint8_t>(estimated_size));
    std::size_t compressed_size;

    error = lzokay::compress((const uint8_t *)src.data(), compressed.first,
                             compressed.second.data(), estimated_size, compressed_size, dict);
    if (error < lzokay::EResult::Success)
    {
        compressed.second.clear();
        compressed.first = 0;
    }
    else
        compressed.second.resize(compressed_size);

    return compressed;
}

const QString &StringsFileCache::getFileNameOrEmpty(const QString &key)
{
    const auto it = key2filename.find(key);
    if (it != key2filename.end())
        return it.value();

    const static QString empty;
    return empty;
}
