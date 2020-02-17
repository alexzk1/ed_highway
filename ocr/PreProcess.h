#ifndef PRE_PROCESS_H
#define PRE_PROCESS_H

#include <QImage>
#include "ocr_ptr_types.h"

class PreProcess
{
public:
    PreProcess();

    bool getVerticalText() const;
    void setVerticalOrientation(bool value);

    bool getRemoveFurigana() const;
    void setRemoveFurigana(bool value);

    int getJapNumTextLines() const;

    QRect getBoundingRect() const;

    float getScaleFactor() const;
    void setScaleFactor(float value);

    PIXPtr convertImageToPix(const QImage &image);

    PIXPtr processImage(const PIXPtr &pixs, bool performDeskew = false, bool trim = false);
    PIXPtr extractTextBlock(const PIXPtr &pixs, int pt_x, int pt_y, int lookahead, int lookbehind, int searchRadius);
    PIXPtr extractBubbleText(const PIXPtr &pixs, int pt_x, int pt_y);

private:
    PIXPtr makeGray(const PIXPtr &pixs);
    PIXPtr scale(const PIXPtr &pixs);
    PIXPtr unsharpMask(const PIXPtr &pixs);
    PIXPtr binarize(const PIXPtr &pixs);
    PIXPtr scaleUnsharpBinarize(const PIXPtr &pixs);
    PIXPtr deskew(const PIXPtr &pixs);
    PIXPtr addBorder(const PIXPtr &pixs);
    PIXPtr removeNoise(const PIXPtr &pixs);
    PIXPtr eraseFurigana(const PIXPtr &pixs);
    void setDPI(const PIXPtr &pixs);

    void debugMsg(const QString &str, bool error = true);
    void debugImg(const QString& filename, const PIXPtr &pixs);

#ifdef QT_DEBUG
    const bool debug = false;
#endif

    // From 0.0 to 1.0, with 0 being all white and 1 being all black
    const float darkBgThreshold = 0.3f;

    // Amount to scale input image to meet OCR engine minimum DPI requirements
    float scaleFactor = 3.5f;

    // Is the text vertical (affects furigana removal)
    bool verticalText;

    // The last bounding rect extracted.
    // Set in extractTextBlock() and extractBubbleText().
    // Can be used for display purposes.
    BOX boundingRect;

    bool removeFurigana;

    // Number of lines detected when last furigana removal was performed
    int japNumTextLines = 0;

    int debugImgCount = 0;
};

#endif // PRE_PROCESS_H
