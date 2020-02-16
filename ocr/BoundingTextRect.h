#ifndef BOUNDING_TEXT_RECT_H
#define BOUNDING_TEXT_RECT_H

#include <QList>
#include "ocr_ptr_types.h"
#include "PreProcessCommon.h"

class BoundingTextRect
{
public:
    enum class D8
    {
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
        TopLeft
    };

    struct DirDist
    {
        D8 dir;
        int dist;

        DirDist(D8 _dir, int _dist)
            : dir(_dir), dist(_dist) { }
    };

    static BOX getBoundingRect(const PIXPtr& pixs, int startX, int startY, bool vertical,
                               int lookahead, int lookbehind, int maxSearchDist);

    static OcrPoint findNearestBlackPixel(const PIXPtr& pixs, int startX, int startY, int max_dist);
    static bool isBlack(const PIXPtr& pixs, int x, int y);
    static bool inRangeX(const PIXPtr& pixs, int x);
    static bool inRangeY(const PIXPtr& pixs, int y);
    static bool lineContainBlackHoriz(const PIXPtr& pixs, int startX, int startY, int width);
    static bool lineContainBlackVert(const PIXPtr& pixs, int startX, int startY, int height);

private:
    BoundingTextRect();

    static bool tryExpandRect(const PIXPtr &pixs, BOX& rect, D8 dir, int dist);
    static void expandRect(const PIXPtr& pixs, QList<DirDist> &dirDistList, BOX& rect, bool keep_going);
};

#endif // BOUNDING_TEXT_RECT_H
