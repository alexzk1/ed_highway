#include "eliteocr.h"
#include <QScreen>
#include <QPixmap>
#include <QApplication>
#include <QRegularExpression>
#include "utils/strutils.h"

EliteOCR::EliteOCR()
{
    preProcess.setVerticalOrientation(false);
    preProcess.setRemoveFurigana(false);
    preProcess.setScaleFactor(2.5f); //this factor works for galaxy map on 1920 x 1080 screen
}

QStringList EliteOCR::recognize(const QImage &img)
{
    auto pix  = preProcess.convertImageToPix(img);
    //OCRHelpers::dumpPix("./pix.png", pix);
    auto pix2 = preProcess.processImage(pix, false, false);
    //OCRHelpers::dumpPix("./pix2.png", pix2);
    return split_filter(ocrEngine.performOcr(pix2, false));
}

QStringList EliteOCR::recognizeScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();

    if (!screen)
        return QStringList();

    QPixmap capturePixmap = screen->grabWindow(0);

    return recognize(capturePixmap.toImage());
}

QString EliteOCR::tryDetectStarFromMapPopup(const QStringList &src)
{
    {
        const auto b = src.begin();
        const auto e = src.end();
        auto it = std::find_if(b, e, [](const auto & s)
        {
            return s.startsWith("DISTANCE: ") ;
        });
        if (it != e && it != b)
        {
            std::advance(it, -1);
            return *it;
        }
    }
    //trying regex for auto-generated sys names as last resort, examples:
    //PHROI PRI NX-A D1-785
    //Pyrie Thae XO-Z D13-12
    //"Suvaa LM-W F1-0"
    //but going backward, as at top will be currently selected sys most likely

    const static QRegularExpression last_check("^[A-H]\\d+-\\d+$");
    const static QRegularExpression before_last_check("^\\w\\w-\\w$");
    auto it = std::find_if(src.rbegin(), src.rend(), [](const auto & s)
    {
        QStringList parts = s.split(" ", QString::SkipEmptyParts);
        //at least 3 parts
        if (parts.size() < 3)
            return false;
        //checking if last 2 match to regexp
        const bool a = last_check.match(*parts.rbegin(), 0, QRegularExpression::PartialPreferCompleteMatch).hasMatch();
        const bool b = before_last_check.match(*(parts.rbegin() + 1), 0, QRegularExpression::PartialPreferCompleteMatch).hasMatch();
        return a && b;
    });

    if (it != src.rend())
        return *it;

    return "";
}

QStringList EliteOCR::split_filter(const QString &src) const
{
    QStringList lst = src.split(QRegularExpression("[\n\r]"), QString::SkipEmptyParts);
    QStringList res;
    res.reserve((lst.size()));
    for (auto& s : lst)
    {
        s = s.trimmed();
        s = s.normalized(QString::NormalizationForm_D).toUpper();
        if (s.length() > 3)
            res.push_back(s);
    }
    return res;
}
