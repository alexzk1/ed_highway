#include "eliteocr.h"
#include <QScreen>
#include <QPixmap>
#include <QApplication>
#include <QRegularExpression>
#include <QDesktopWidget>
#include "utils/strutils.h"
#include "utils/strfmt.h"

#ifdef Q_OS_LINUX
    #include <QX11Info>
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
    #include <X11/extensions/Xfixes.h>
#endif

EliteOCR::EliteOCR()
{
    preProcess.setVerticalOrientation(false);
    preProcess.setRemoveFurigana(false);
    preProcess.setScaleFactor(2.7f); //this factor works for galaxy map on 1920 x 1080 screen
}

QStringList EliteOCR::recognize(const QImage &img)
{
    auto pix  = preProcess.convertImageToPix(img);
    if (!pix)
        return QStringList();
    auto pix2 = preProcess.processImage(pix, false, false);
    //auto pix2 = preProcess.extractBubbleText(pix, 0, 0);
    if (!pix2)
        return QStringList();
#ifdef SRC_PATH
    static int dump_id {0};
    OCRHelpers::dumpPix(stringfmt ("./pix2_%d", ++dump_id), pix2);
#endif
    return split_filter(ocrEngine.performOcr(pix2, false));
}


QStringList EliteOCR::recognizeScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();

    if (!screen)
        return QStringList();

    QPixmap capturePixmap = screen->grabWindow(QApplication::desktop()->winId());

    return recognize(capturePixmap.toImage());
}

QString EliteOCR::tryDetectStarFromMapPopup(const QStringList &src)
{
    //    for (const auto & s : src)
    //        std::cout << s.toStdString() << std::endl;
    {
        const auto b = src.begin();
        const auto e = src.end();
        auto it = std::find_if(b, e, [](const auto & s)
        {
            return s.startsWith("DISTANCE: ") || s.startsWith("ARRIVAL POINT:");
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
    const static QRegularExpression planet("^\\d+\\s*\\w*$");

    const static auto test_if_star_pattern = [](auto begin, auto end)
    {
        //at least 3 parts
        if (std::distance(begin, end) < 3)
            return false;
        //checking if last 2 match to regexp
        const bool a = last_check.match(*begin, 0, QRegularExpression::PartialPreferCompleteMatch).hasMatch();
        const bool b = before_last_check.match(*(begin + 1), 0, QRegularExpression::PartialPreferCompleteMatch).hasMatch();
        return a && b;
    };

    auto it = std::find_if(src.rbegin(), src.rend(), [](const auto & s)
    {
        QStringList parts = s.split(" ", QString::SkipEmptyParts);
        if (parts.size() < 3)
            return false;
        //maybe it is planet's name?
        const bool b = (planet.match(*parts.rbegin(), 0, QRegularExpression::PartialPreferCompleteMatch).hasMatch() && test_if_star_pattern(parts.rbegin() + 1, parts.rend()));
        const bool a = test_if_star_pattern(parts.rbegin(), parts.rend());


        return a || b;
        //return a;
    });

    if (it != src.rend())
    {
        QString s = *it;
        s.replace("!", "I");
        return s;
    }

    const static QString nothing;
    return nothing;
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
