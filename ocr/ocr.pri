#ocr bindins to tesseract are based on Capture2Text project of Christopher Brochtrup

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/BoundingTextRect.h \
    $$PWD/Furigana.h \
    $$PWD/OcrEngine.h \
    $$PWD/PreProcess.h \
    $$PWD/PreProcessCommon.h \
    $$PWD/eliteocr.h \
    $$PWD/ocr_ptr_types.h

SOURCES += \
    $$PWD/BoundingTextRect.cpp \
    $$PWD/Furigana.cpp \
    $$PWD/OcrEngine.cpp \
    $$PWD/PreProcess.cpp \
    $$PWD/eliteocr.cpp

LIBS *= -ltesseract
LIBS *= -llept

DEFINES += OCR_ADDED
