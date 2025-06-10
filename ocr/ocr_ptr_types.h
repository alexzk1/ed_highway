#pragma once
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <memory>
#include <string>

using PIXPtr = std::shared_ptr<PIX>;
using BOXPtr = std::shared_ptr<BOX>;

namespace OCRHelpers {
inline void pixDeletor(PIX *p)
{
    if (p)
        pixDestroy(&p);
}

inline void boxDeletor(BOX *p)
{
    if (p)
        boxDestroy(&p);
}

inline PIXPtr createPP(PIX *p)
{
    return PIXPtr(p, pixDeletor);
}

inline void dumpPix(const std::string &file, const PIXPtr &ptr)
{
    pixWriteImpliedFormat(file.c_str(), ptr.get(), 0, 0);
}
} // namespace OCRHelpers
