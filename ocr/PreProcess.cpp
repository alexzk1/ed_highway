#include "PreProcess.h"

#include "BoundingTextRect.h"
#include "Furigana.h"
#include "PreProcessCommon.h"
#include "utils/shared_wrapper.h"

#include <QBuffer>
#include <QDebug>

#include <QtGlobal>

using namespace OCRHelpers;

PreProcess::PreProcess() :
    verticalText(false),
    removeFurigana(false)
{
}

QRect PreProcess::getBoundingRect() const
{
    return QRect(boundingRect.x / scaleFactor, boundingRect.y / scaleFactor,
                 boundingRect.w / scaleFactor, boundingRect.h / scaleFactor);
}

bool PreProcess::getRemoveFurigana() const
{
    return removeFurigana;
}

void PreProcess::setRemoveFurigana(bool value)
{
    removeFurigana = value;
}

bool PreProcess::getVerticalText() const
{
    return verticalText;
}

void PreProcess::setVerticalOrientation(bool value)
{
    verticalText = value;
}

// Set DPI so that Tesseract 4.0 doesn't issue this warning:
// "Warning. Invalid resolution 0 dpi. Using 70 instead."
void PreProcess::setDPI(const PIXPtr &pixs)
{
    pixs->xres = 300;
    pixs->yres = 300;
}

int PreProcess::getJapNumTextLines() const
{
    return japNumTextLines;
}

float PreProcess::getScaleFactor() const
{
    return scaleFactor;
}

void PreProcess::setScaleFactor(float value)
{
    scaleFactor = qMin(qMax(value, 0.71f), 5.0f);
}

void PreProcess::debugMsg(const QString &str, bool error)
{
#ifdef QT_DEBUG
    if (debug || error)
        qDebug() << str;
#else
    Q_UNUSED(str);
    Q_UNUSED(error);
#endif
}

void PreProcess::debugImg(const QString &filename, const PIXPtr &pixs)
{
#ifdef QT_DEBUG
    if (debug)
    {
        debugImgCount++;
        QString file = QString("G:\\Temp\\Temp\\c2t_debug\\%1_%2")
                         .arg(debugImgCount, 2, 10, QChar('0'))
                         .arg(filename);
        QByteArray ba = file.toLocal8Bit();
        pixWriteImpliedFormat(ba.constData(), pixs.get(), 0, 0);
    }
#else
    Q_UNUSED(filename);
    Q_UNUSED(pixs);
#endif
}

// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::convertImageToPix(const QImage &image)
{
    // On Windows, pixReadMem() only works with the TIF format,
    // so save the image to memory in TIF format first.
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "TIF");

    return createPP(pixReadMem((const unsigned char *)ba.data(), ba.size()));
}

// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::makeGray(const PIXPtr &pixs)
{
    return createPP(pixConvertRGBToGray(pixs.get(), 0.0f, 0.0f, 0.0f));
}

// pixs must be 8 bpp.
// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::scale(const PIXPtr &pixs)
{
    return createPP(pixScaleGrayLI(pixs.get(), scaleFactor, scaleFactor));
}

// pixs must be 8 bpp.
// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::unsharpMask(const PIXPtr &pixs)
{
    const int usmHalfwidth = 5;
    const float usmFract = 2.5f;

    return createPP(pixUnsharpMaskingGray(pixs.get(), usmHalfwidth, usmFract));
}

// pixs must be 8 bpp.
// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::binarize(const PIXPtr &pixs)
{
    const int otsuSX = 2000;
    const int otsuSY = 2000;
    const int otsuSmoothX = 0;
    const int otsuSmoothY = 0;
    const float otsuScorefract = 0.0f;

    PIX *binarize_pixs = nullptr;

#if 1
    int status = pixOtsuAdaptiveThreshold(pixs.get(), otsuSX, otsuSY, otsuSmoothX, otsuSmoothY,
                                          otsuScorefract, nullptr, &binarize_pixs);

    if (status != LEPT_OK)
    {
        debugMsg("binarize: failed!");
        return nullptr;
    }
#else
    binarize_pixs =
      pixOtsuThreshOnBackgroundNorm(pixs.get(), nullptr, otsuSX, otsuSY, 100, 50, 255, otsuSmoothX,
                                    otsuSmoothY, otsuScorefract, nullptr);

    if (binarize_pixs == nullptr)
    {
        debugMsg("binarize: failed!");
        return nullptr;
    }
#endif

    return createPP(binarize_pixs);
}

// pixs must be 8 bpp.
// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::scaleUnsharpBinarize(const PIXPtr &pixs)
{
    return binarize(unsharpMask(scale(pixs)));
}

// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::deskew(const PIXPtr &pixs)
{
#if 0
    l_float32 angle;
    l_float32 conf;
    PIX *pixDeskew = pixFindSkewAndDeskew(pixs.get(), 0, &angle, &conf);
    qDebug() << "Angle: " << angle << " Conf: " << conf;
#else
    PIX *pixDeskew = pixFindSkewAndDeskew(pixs.get(), 0, nullptr, nullptr);
#endif
    if (pixDeskew == nullptr)
    {
        debugMsg("deskew: failed!");
        return nullptr;
    }
    return createPP(pixDeskew);
}

// pixs must be 1 bpp.
// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::addBorder(const PIXPtr &pixs)
{
    const int borderWidth = 10;
    return createPP(pixAddBlackOrWhiteBorder(pixs.get(), borderWidth, borderWidth, borderWidth,
                                             borderWidth, L_GET_WHITE_VAL));
}

// pixs must be 1 bpp.
// Remove very small blobs.
// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::removeNoise(const PIXPtr &pixs)
{
    // int minBlobSize = (int)(1.86 * scaleFactor);
    int minBlobSize = 3;

    // Remove noise if both dimensions are less than minBlobSize (yes, L_SELECT_IF_EITHER is correct
    // here).
    return createPP(pixSelectBySize(pixs.get(), minBlobSize, minBlobSize, 8, L_SELECT_IF_EITHER,
                                    L_SELECT_IF_GT, nullptr));
}

// pixs must be 1 bpp.
PIXPtr PreProcess::eraseFurigana(const PIXPtr &pixs)
{
    PIXPtr denoisePixs = nullptr;
    if (removeFurigana)
    {
        bool status = true;
        if (verticalText)
            status = Furigana::eraseFuriganaVertical(pixs.get(), scaleFactor, &japNumTextLines);
        else
            status = Furigana::eraseFuriganaHorizontal(pixs.get(), scaleFactor, &japNumTextLines);

        if (status)
            denoisePixs = removeNoise(pixs);
        else
            debugMsg("eraseFurigana: failed!");
    }
    else
        denoisePixs = createPP(pixClone(pixs.get()));

    return denoisePixs;
}

// Standard pre-process for OCR.
// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::processImage(const PIXPtr &pixs, bool performDeskew, bool trim)
{
    debugImgCount = 0;

    // Convert to grayscale
    auto pixGray = makeGray(pixs);

    if (pixGray == nullptr)
        return nullptr;

    // Binarize for negate determination
    auto binarizeForNegPixs = binarize(pixGray);

    if (binarizeForNegPixs == nullptr)
        return nullptr;

    float pixelAvg = 0.0f;

    // Get the average intensity of the border pixels,
    // with average of 0.0 being completely white and 1.0 being completely black.
    // Top, bottom, left, right.
    pixelAvg = pixAverageOnLine(binarizeForNegPixs.get(), 0, 0, binarizeForNegPixs->w - 1, 0, 1);
    pixelAvg += pixAverageOnLine(binarizeForNegPixs.get(), 0, binarizeForNegPixs->h - 1,
                                 binarizeForNegPixs->w - 1, binarizeForNegPixs->h - 1, 1);
    pixelAvg += pixAverageOnLine(binarizeForNegPixs.get(), 0, 0, 0, binarizeForNegPixs->h - 1, 1);
    pixelAvg += pixAverageOnLine(binarizeForNegPixs.get(), binarizeForNegPixs->w - 1, 0,
                                 binarizeForNegPixs->w - 1, binarizeForNegPixs->h - 1, 1);
    pixelAvg /= 4.0f;

    binarizeForNegPixs.reset();

    // If background is dark
    if (pixelAvg > darkBgThreshold)
    {
        // Negate image (yes, input and output can be the same PIX)
        pixInvert(pixGray.get(), pixGray.get());

        if (pixGray == nullptr)
            return nullptr;
    }

    // Scale, Unsharp Mask, Binarize
    auto pixBinarize = scaleUnsharpBinarize(pixGray);
    pixGray.reset();

    if (pixBinarize == nullptr)
        return nullptr;

    // Deskew
    if (performDeskew)
    {
        auto pixDeskew = deskew(pixBinarize);

        // Deskew isn't critical, ignore on failure
        if (pixDeskew != nullptr)
            pixBinarize = pixDeskew;
    }

    // Erase furigana
    auto furiganaPixs = eraseFurigana(pixBinarize);
    pixBinarize.reset();

    if (furiganaPixs == nullptr)
        return nullptr;

    if (trim)
    {
        PIXPtr foregroundPixs = nullptr;

        // Remove border

        int status = pixClipToForeground(
          furiganaPixs.get(), shared_ptr_tmp_wrapper<PIXPtr>(foregroundPixs, pixDeletor), nullptr);
        if (status != LEPT_OK)
        {
            debugMsg("pixClipToForeground failed!");
            return nullptr;
        }
        furiganaPixs.reset();

        // Add border
        auto borderPixs = addBorder(foregroundPixs);
        foregroundPixs.reset();

        if (borderPixs == nullptr)
            return nullptr;

        setDPI(borderPixs);
        return borderPixs;
    }

    setDPI(furiganaPixs);
    return furiganaPixs;
}

// Extract the text block closest to the provided point.
// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::extractTextBlock(const PIXPtr &pixs, int pt_x, int pt_y, int lookahead,
                                    int lookbehind, int searchRadius)
{
    debugImgCount = 0;
    int status = LEPT_ERROR;

    // Convert to grayscale
    auto pixGray = makeGray(pixs);

    if (pixGray == nullptr)
        return nullptr;

    // Binarize for negate determination
    auto binarizeForNegPixs = binarize(pixGray);

    debugImg("binarizeForNegPixs.png", binarizeForNegPixs);

    if (binarizeForNegPixs == nullptr)
        return nullptr;

    // Get the average intensity in the area around the start coordinates
    BOX negRect;
    const int idealNegRectLength = 40;
    negRect.refcount = 1;
    negRect.x = qMax(0, pt_x - idealNegRectLength / 2);
    negRect.y = qMax(0, pt_y - idealNegRectLength / 2);
    negRect.w = qMin((int)binarizeForNegPixs->w - negRect.x, idealNegRectLength);
    negRect.h = qMin((int)binarizeForNegPixs->h - negRect.y, idealNegRectLength);

    float pixelAvg = 0.0f;
    status = pixAverageInRect(binarizeForNegPixs.get(), nullptr, &negRect, 0,
                              std::numeric_limits<l_int32>::max(), 0, &pixelAvg);
    binarizeForNegPixs.reset();

    // qDebug() << "Pixel Avg: " << pixelAvg;

    if (status != LEPT_OK)
    {
        // Assume white background
        pixelAvg = 0.0;
    }

    // If background is dark
    if (pixelAvg > darkBgThreshold)
    {
        if (pixGray == nullptr)
            return nullptr;

        // Negate image (yes, input and output can be the same PIX)
        pixInvert(pixGray.get(), pixGray.get());
    }

    // Scale, Unsharp Mark, Binarize
    auto binarizePixs = scaleUnsharpBinarize(pixGray);
    pixGray.reset();

    if (binarizePixs == nullptr)
        return nullptr;

    // Remove black pixels connected to the border.
    // This eliminates annoying things like text bubbles in manga.
    auto connCompsPixs = createPP(pixRemoveBorderConnComps(binarizePixs.get(), 8));

    if (connCompsPixs == nullptr)
    {
        debugMsg("pixRemoveBorderConnComps failed!");
        return nullptr;
    }

    debugImg("connCompsPixs.png", connCompsPixs);

    // Remove noise
    auto denoisePixs = removeNoise(connCompsPixs);
    connCompsPixs.reset();

    if (denoisePixs == nullptr)
        return nullptr;

    // Get rectangle surrounding the text to extract
    boundingRect = BoundingTextRect::getBoundingRect(
      denoisePixs, pt_x * scaleFactor, pt_y * scaleFactor, verticalText, lookahead * scaleFactor,
      lookbehind * scaleFactor, searchRadius * scaleFactor);
    denoisePixs.reset();

    if (boundingRect.w < 3 && boundingRect.h < 3)
    {
        debugMsg("BoundingRect too small!");
        return nullptr;
    }

    auto croppedPixs = createPP(pixClipRectangle(binarizePixs.get(), &boundingRect, nullptr));
    binarizePixs.reset();

    if (croppedPixs == nullptr)
        return nullptr;

    debugImg("croppedPixs.png", croppedPixs);

    // Erase furigana
    auto furiganaPixs = eraseFurigana(croppedPixs);
    croppedPixs.reset();

    if (furiganaPixs == nullptr)
        return nullptr;

    PIXPtr foregroundPixs = nullptr;
    BOXPtr foregroundBox = nullptr;

    // Remove border
    status = pixClipToForeground(furiganaPixs.get(),
                                 shared_ptr_tmp_wrapper<PIXPtr>(foregroundPixs, pixDeletor),
                                 shared_ptr_tmp_wrapper<BOXPtr>(foregroundBox, boxDeletor));
    furiganaPixs.reset();

    if (status != LEPT_OK)
    {
        debugMsg("pixClipToForeground failed!");
        return nullptr;
    }

    // Adjust bounding rect to account for removed border
    boundingRect.x = boundingRect.x + foregroundBox->x;
    boundingRect.y = boundingRect.y + foregroundBox->y;
    boundingRect.w = foregroundBox->w;
    boundingRect.h = foregroundBox->h;

    // Add border
    auto borderPixs = addBorder(foregroundPixs);
    foregroundPixs.reset();

    if (borderPixs == nullptr)
        return nullptr;

    setDPI(borderPixs);
    return borderPixs;
}

static PIXPtr pixInvert(const PIXPtr &p1, const PIXPtr &p2)
{
    return createPP(pixInvert(p1.get(), p2.get()));
}

// Extract all text within an enclosed area such as a comic book speech/thought bubble.
// Be sure to call pixDestroy() on the returned PIX pointer to avoid memory leak.
PIXPtr PreProcess::extractBubbleText(const PIXPtr &pixs, int pt_x, int pt_y)
{
    debugImgCount = 0;
    l_int32 status = LEPT_ERROR;

    pt_x = (int)(pt_x * scaleFactor);
    pt_y = (int)(pt_y * scaleFactor);

    // Convert to grayscale
    auto grayPixs = makeGray(pixs);

    if (grayPixs == nullptr)
        return nullptr;

    // Scale, Unsharp Mark, Binarize
    auto binarizePixs = scaleUnsharpBinarize(grayPixs);
    grayPixs.reset();

    if (binarizePixs == nullptr)
        return nullptr;

    // Get color of the start pixel
    l_uint32 startPtIsBlack = LEPT_FALSE;
    status = pixGetPixel(binarizePixs.get(), pt_x, pt_y, &startPtIsBlack);

    if (status != LEPT_OK)
    {
        debugMsg("pixGetPixel failed!");
        return nullptr;
    }

    // Invert if bubble is black background with white text
    if (startPtIsBlack)
    {
        if (binarizePixs == nullptr)
        {
            debugMsg("pixInvert failed!");
            return nullptr;
        }
        pixInvert(binarizePixs, binarizePixs);
    }

    // Create the seed start point
    auto seedStartPixs = createPP(pixCreateTemplate(binarizePixs.get()));

    if (seedStartPixs == nullptr)
    {
        debugMsg("pixCreateTemplate failed!");
        return nullptr;
    }

    status = pixSetPixel(seedStartPixs.get(), pt_x, pt_y, 1);

    if (status != LEPT_OK)
    {
        debugMsg("pixSetPixel failed!");
        return nullptr;
    }

    // Dilate to thicken lines and connect small gaps in the bubble
    int thickenAmount = (int)(2 * scaleFactor);
    auto thickenLinesPixs =
      createPP(pixDilateBrick(nullptr, binarizePixs.get(), thickenAmount, thickenAmount));

    if (thickenLinesPixs == nullptr)
    {
        debugMsg("pixMorphSequence failed!");
        return nullptr;
    }

    debugImg("thickenLinesPixs.png", thickenLinesPixs);

    // Invert for seed fill
    auto binarizeNegPixs = pixInvert(nullptr, thickenLinesPixs);
    thickenLinesPixs.reset();

    if (binarizeNegPixs == nullptr)
    {
        debugMsg("pixInvert failed!");
        return nullptr;
    }

    // Seed fill
    auto seedFillPixs =
      createPP(pixSeedfillBinary(nullptr, seedStartPixs.get(), binarizeNegPixs.get(), 8));
    seedStartPixs.reset();
    binarizeNegPixs.reset();

    if (seedFillPixs == nullptr)
    {
        debugMsg("pixSeedfillBinary failed!");
        return nullptr;
    }

    debugImg("seedFillPixs.png", seedFillPixs);

    // Negate seed fill
    pixInvert(seedFillPixs, seedFillPixs);

    if (seedFillPixs == nullptr)
    {
        debugMsg("pixInvert 2 failed!");
        return nullptr;
    }

    debugImg("seedFillPixs_Neg.png", seedFillPixs);

    // Remove foreground pixels touching the border
    auto noBorderPixs = createPP(pixRemoveBorderConnComps(seedFillPixs.get(), 8));
    seedFillPixs.reset();

    if (noBorderPixs == nullptr)
    {
        debugMsg("pixRemoveBorderConnComps failed!");
        return nullptr;
    }

    debugImg("noBorderPixs.png", noBorderPixs);

    // AND with original binary image to remove everything except for the text
    auto andPixs = createPP(pixAnd(nullptr, noBorderPixs.get(), binarizePixs.get()));
    binarizePixs.reset();
    noBorderPixs.reset();

    if (andPixs == nullptr)
    {
        debugMsg("pixAnd failed!");
        return nullptr;
    }

    debugImg("andPixs.png", andPixs);

    auto denoisePixs = removeNoise(andPixs);
    andPixs.reset();

    if (denoisePixs == nullptr)
        return nullptr;

    // Erase furigana
    auto furiganaPixs = eraseFurigana(denoisePixs);
    denoisePixs.reset();

    if (furiganaPixs == nullptr)
        return nullptr;

    // Clip to text
    PIXPtr clippedPixs = nullptr;
    BOXPtr foregroundBox = nullptr;
    status = pixClipToForeground(furiganaPixs.get(),
                                 shared_ptr_tmp_wrapper<PIXPtr>(clippedPixs, pixDeletor),
                                 shared_ptr_tmp_wrapper<BOXPtr>(foregroundBox, boxDeletor));
    furiganaPixs.reset();

    if (status != LEPT_OK)
    {
        debugMsg("pixClipToForeground failed!");
        return nullptr;
    }

    debugImg("clippedPixs.png", clippedPixs);

    // Add border
    auto borderPixs = addBorder(clippedPixs);
    clippedPixs.reset();

    if (borderPixs == nullptr)
        return nullptr;

    boundingRect.x = foregroundBox->x;
    boundingRect.y = foregroundBox->y;
    boundingRect.w = foregroundBox->w;
    boundingRect.h = foregroundBox->h;

    setDPI(borderPixs);
    return borderPixs;
}
