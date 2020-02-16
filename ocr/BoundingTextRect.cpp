#include "BoundingTextRect.h"


BoundingTextRect::BoundingTextRect()
{

}

OcrPoint BoundingTextRect::findNearestBlackPixel(const PIXPtr &pixs, int startX, int startY, int maxDist)
{
    OcrPoint pt = { startX, startY };

    for (int dist = 1; dist < maxDist; ++dist)
    {
        // Check right one pixel
        pt.x++;

        if (isBlack(pixs, pt.x, pt.y))
            return pt;

        // Check down
        for (int i = 0; i < dist * 2 - 1; i++)
        {
            pt.y++;

            if (isBlack(pixs, pt.x, pt.y))
                return pt;
        }

        // Check left
        for (int i = 0; i < dist * 2; i++)
        {
            pt.x--;

            if (isBlack(pixs, pt.x, pt.y))
                return pt;
        }

        // Check up
        for (int i = 0; i < dist * 2; i++)
        {
            pt.y--;

            if (isBlack(pixs, pt.x, pt.y))
                return pt;
        }

        // Check right
        for (int i = 0; i < dist * 2; i++)
        {
            pt.x++;

            if (isBlack(pixs, pt.x, pt.y))
                return pt;
        }
    }

    pt.x = -1;
    pt.y = -1;

    return pt;
}

bool BoundingTextRect::lineContainBlackHoriz(const PIXPtr& pixs, int startX, int startY, int width)
{
    for (int x = startX; x <= startX + width && inRangeX(pixs, x); ++x)
    {
        if (isBlack(pixs, x, startY))
            return true;
    }

    return false;
}

bool BoundingTextRect::lineContainBlackVert(const PIXPtr &pixs, int startX, int startY, int height)
{
    for (int y = startY; y <= startY + height && inRangeY(pixs, y); ++y)
    {
        if (isBlack(pixs, startX, y))
            return true;
    }

    return false;
}

bool BoundingTextRect::isBlack(const PIXPtr &pixs, int x, int y)
{
    l_uint32 pixelValue = 0;

    const bool r = (inRangeX(pixs, x) && inRangeY(pixs, y)) &&
                   (pixGetPixel(pixs.get(), x, y, &pixelValue) == LEPT_OK) && (pixelValue == 1);

    return r;
}

bool BoundingTextRect::inRangeX(const PIXPtr &pixs, int x)
{
    return ((x >= 0) && ((unsigned int)x < pixs->w));
}

bool BoundingTextRect::inRangeY(const PIXPtr& pixs, int y)
{
    return ((y >= 0) && ((unsigned int)y < pixs->h));
}

bool BoundingTextRect::tryExpandRect(const PIXPtr& pixs, BOX &rect, D8 dir, int dist)
{
    if (dir == D8::Top)
    {
        if (lineContainBlackHoriz(pixs, rect.x, rect.y - dist, rect.w))
        {
            rect.y -= dist;
            rect.h += dist;
            return true;
        }
    }
    else
        if (dir == D8::TopRight)
        {
            if (isBlack(pixs, rect.x + rect.w + dist, rect.y - dist))
            {
                rect.y -= dist;
                rect.h += dist;
                rect.w += dist;
                return true;
            }
        }
        else
            if (dir == D8::Right)
            {
                if (lineContainBlackVert(pixs, rect.x + rect.w + dist, rect.y, rect.h))
                {
                    rect.w += dist;
                    return true;
                }
            }
            else
                if (dir == D8::BottomRight)
                {
                    if (isBlack(pixs, rect.x + rect.w + dist, rect.y + rect.h + dist))
                    {
                        rect.h += dist;
                        rect.w += dist;
                        return true;
                    }
                }
                else
                    if (dir == D8::Bottom)
                    {
                        if (lineContainBlackHoriz(pixs, rect.x, rect.y + rect.h + dist, rect.w))
                        {
                            rect.h += dist;
                            return true;
                        }
                    }
                    else
                        if (dir == D8::BottomLeft)
                        {
                            if (isBlack(pixs, rect.x - dist, rect.y + rect.h + dist))
                            {
                                rect.x -= dist;
                                rect.h += dist;
                                rect.w += dist;
                                return true;
                            }
                        }
                        else
                            if (dir == D8::Left)
                            {
                                if (lineContainBlackVert(pixs, rect.x - dist, rect.y, rect.h))
                                {
                                    rect.x -= dist;
                                    rect.w += dist;
                                    return true;
                                }
                            }
                            else
                                if (dir == D8::TopLeft)
                                {
                                    if (isBlack(pixs, rect.x - dist, rect.y + dist))
                                    {
                                        rect.x -= dist;
                                        rect.y -= dist;
                                        rect.h += dist;
                                        rect.w += dist;
                                        return true;
                                    }
                                }

    return false;
}

void BoundingTextRect::expandRect(const PIXPtr &pixs, QList<DirDist> &dirDistList, BOX& rect, bool keepGoing)
{
    int i = 0;

    while (true)
    {
        DirDist dirDist = dirDistList[i];

        // Try to expand rect in correct direction
        bool hasBlack = tryExpandRect(pixs, rect, dirDist.dir, dirDist.dist);

        // If could not expand (ie no black pixel found in current direction)
        if (!hasBlack)
            ++i;
        // If caller wants to exit upon first successful expansion
        else
            if (!keepGoing)
                return;

        // If we went through the entire list, return
        if (i >= dirDistList.size())
            return;
    }
}

BOX BoundingTextRect::getBoundingRect(const PIXPtr &pixs, int startX, int startY, bool vertical,
                                      int lookahead, int lookbehind, int maxSearchDist)
{
    auto nearestPt = findNearestBlackPixel(pixs, startX, startY, maxSearchDist);
    BOX rect = { nearestPt.x, nearestPt.y, 0, 0, 1};
    BOX rectLast = rect;

    QList<DirDist> listD4;

    if (vertical)
    {
        for (int i = 1; i < lookbehind + 1; i++)
            listD4.append(DirDist(D8::Top, i));

        listD4.append(DirDist(D8::Right, 1));
        listD4.append(DirDist(D8::Left, 1));

        for (int i = 1; i < lookahead + 1; i++)
            listD4.append(DirDist(D8::Bottom, i));
    }
    else
    {
        listD4.append(DirDist(D8::Top, 1));

        for (int i = 1; i < lookbehind + 1; i++)
            listD4.append(DirDist(D8::Left, i));

        listD4.append(DirDist(D8::Bottom, 1));

        for (int i = 1; i < lookahead + 1; i++)
            listD4.append(DirDist(D8::Right, i));
    }

    QList<DirDist> listCorners;
    listCorners.append(DirDist(D8::TopRight, 1));
    listCorners.append(DirDist(D8::BottomRight, 1));
    listCorners.append(DirDist(D8::BottomLeft, 1));
    listCorners.append(DirDist(D8::TopLeft, 1));

    // Try a few iterations to form the best bounding rect
    for (int i = 0; i < 10; ++i)
    {
        expandRect(pixs, listD4, rect, true);
        expandRect(pixs, listCorners, rect, false);

        // No change this iteration, no need to continue
        if (rect.x == rectLast.x
                && rect.y == rectLast.y
                && rect.w == rectLast.w
                && rect.h == rectLast.h)
            break;

        rectLast = rect;
    }

    return rect;
}
