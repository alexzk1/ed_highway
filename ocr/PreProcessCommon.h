#ifndef PRE_PROCESS_COMMON_H
#define PRE_PROCESS_COMMON_H

const int NO_VALUE    = -1;
const int LEPT_TRUE   = 1;
const int LEPT_FALSE  = 0;
const int LEPT_OK     = 0;
const int LEPT_ERROR  = 1;

struct OcrPoint
{
    int x{0};
    int y{0};
    OcrPoint() = default;
    OcrPoint(int _x, int _y)
    {
        x = _x;
        y = _y;
    }
};


#endif // PRE_PROCESS_COMMON_H
