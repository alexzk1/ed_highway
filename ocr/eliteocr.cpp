#include "eliteocr.h"
#include <QScreen>
#include <QPixmap>
#include <QApplication>

EliteOCR::EliteOCR()
{
    preProcess.setVerticalOrientation(false);
    preProcess.setRemoveFurigana(false);
    preProcess.setScaleFactor(3.5f);
}

QString EliteOCR::recognize(const QImage &img)
{
    auto pix  = preProcess.convertImageToPix(img);
    //OCRHelpers::dumpPix("./pix.png", pix);
    auto pix2 = preProcess.processImage(pix, false, false);
    //OCRHelpers::dumpPix("./pix2.png", pix2);
    return ocrEngine.performOcr(pix2, false);
}

QString EliteOCR::recognizeScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();

    if (!screen)
        return "";

    QPixmap capturePixmap = screen->grabWindow(0);

    return recognize(capturePixmap.toImage());
}
