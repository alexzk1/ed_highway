#ifndef FURIGANA_H
#define FURIGANA_H

#include <leptonica/allheaders.h>

class Furigana
{
public:

    static bool eraseFuriganaVertical(PIX *pixs, float scaleFactor, int *numTextLines);
    static bool eraseFuriganaHorizontal(PIX *pixs, float scaleFactor, int *numTextLines);

private:
    // Span of lines that contain foreground text. Used during furigana removal.
    struct FuriganaSpan
    {
        int start;
        int end;

        int getLength() const
        {
            return std::abs(end - start) + 1;
        }
        FuriganaSpan() = delete;

        FuriganaSpan(int s, int e)
        {
            start = s;
            end = e;
        }
    };

    Furigana();

    // Minimum number of foreground pixels that a line must contain for it to be part of a span.
    // Needed because sometimes furigana does not have a perfect gap between the text and itself.
    static const float FURIGANA_MIN_FG_PIX_PER_LINE;

    // Minimum width of a span (in pixels) for it to be included in the span list.
    static const float FURIGANA_MIN_WIDTH;

    static bool eraseAreaLeftToRight(PIX *pixs, int x, int width);
    static bool eraseAreaTopToBottom(PIX *pixs, int y, int height);

    static bool averageLargestSpans(PIX *pixs, float scaleFactor);


};

#endif // FURIGANA_H
