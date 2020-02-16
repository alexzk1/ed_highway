#ifndef ELITEOCR_H
#define ELITEOCR_H

#include <QObject>
#include <QImage>
#include "OcrEngine.h"
#include "PreProcess.h"

class EliteOCR
{
private:
    OcrEngine ocrEngine;
    PreProcess preProcess;
public:
    EliteOCR();
    QString recognize(const QImage& img);

    //warning! this must be called from GUI thread
    QString recognizeScreen();
};

#endif // ELITEOCR_H
