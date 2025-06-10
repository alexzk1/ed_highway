#ifndef ELITEOCR_H
#define ELITEOCR_H

#include "OcrEngine.h"
#include "PreProcess.h"

#include <QImage>
#include <QObject>

class EliteOCR
{
  public:
    EliteOCR();
    QStringList recognize(const QImage &img);

    // warning! this must be called from GUI thread
    QStringList recognizeScreen();
    static QString tryDetectStarFromMapPopup(const QStringList &src);

  private:
    OcrEngine ocrEngine;
    PreProcess preProcess;

    QStringList split_filter(const QString &src) const;
};

#endif // ELITEOCR_H
