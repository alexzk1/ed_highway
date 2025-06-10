#ifndef OCR_ENGINE_H
#define OCR_ENGINE_H

#include "ocr_ptr_types.h"
#include "utils/guard_on.h"

#include <QMap>
#include <QRect>
#include <QString>
#include <QStringList>

#include <mutex>

class OcrEngine
{
  public:
    OcrEngine();
    ~OcrEngine();

  public:
    static QStringList getInstalledLangs();
    static const QStringList &getFoldersToFindLangs();
    static bool isLangInstalled(const QString &lang);
    static QString getFirstInstalledLang();
    static QString altLangToLang(QString ocrLang);

    bool setLang(QString lang);
    QString performOcr(const PIXPtr &pixs, bool singleLine);

    QString getLang()
    {
        LOCK_GUARD_ON(mutex);
        return lang;
    }

    bool getVerticalOrientation() const
    {
        LOCK_GUARD_ON(mutex);
        return verticalOrientation;
    }

    void setVerticalOrientation(bool value)
    {
        LOCK_GUARD_ON(mutex);
        verticalOrientation = value;
    }

    QString getWhitelist() const
    {
        LOCK_GUARD_ON(mutex);
        return whitelist;
    }

    void setWhitelist(const QString &value)
    {
        LOCK_GUARD_ON(mutex);
        whitelist = value;
    }

    QString getBlacklist() const
    {
        LOCK_GUARD_ON(mutex);
        return blacklist;
    }

    void setBlacklist(const QString &value)
    {
        LOCK_GUARD_ON(mutex);
        blacklist = value;
    }

    QString getConfigFile() const
    {
        LOCK_GUARD_ON(mutex);
        return configFile;
    }

    void setConfigFile(const QString &value)
    {
        LOCK_GUARD_ON(mutex);
        configFile = value.trimmed();
    }

  private:
    bool isLangCodeInstalled(const QString &langCode);

    // Key = Lang name, Value = Tesseract Code
    static const QMap<QString, QString> &populateLangMap();
    // Key = Tesseract Code, Value = Lang name
    static const QMap<QString, QString> &populateCodeMap();
    // Key = Alt Lang name, Value = Lang name
    static const QMap<QString, QString> &populateAltLangMap();

    mutable std::mutex mutex;
    QString lang;
    bool verticalOrientation = false;
    QString whitelist;
    QString blacklist;
    QString configFile;
    static QString langsFolder;

    std::shared_ptr<tesseract::TessBaseAPI> tessApi{nullptr};
};

#endif // OCR_ENGINE_H
