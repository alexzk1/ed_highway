#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include "spanshapi.h"
#include "spansh_route.h"
#include "spansh_sysname.h"
#include "singleapp/singleapplication.h"

#include "edsmapiv1.h"
#include "edsmv1_nearest.h"
#include "edsmv1_sysinfo.h"
#include <QDebug>
#include "eliteocr.h"

void dotest()
{
    //    SpanshApi test{3};
    //    SpanshRoute r{70, 70, "Sol", "Colonia"};
    //    SpanshSysName sys{"eu"};

    //    EdsmApiV1 test{3};
    //    const static Point A{-2078.71875, -452.125, -1107.375 };
    //    EDSMV1NearerstSystem r(A, 10, false);
    //    EDSMV1SysInfo sys("Maia");

    //    const auto static testr = [](auto err, auto js)
    //    {
    //        if (!err.empty())
    //            std::cerr << err << std::endl;
    //        else
    //            std::cout << js.dump(4) << std::endl;
    //    };

    //    test.executeRequest(r, testr);
    //    test.executeRequest(sys, testr);
    //    test.threads.stop(true);


    //    EliteOCR ocr;
    //    QImage img;
    //    if (img.load("/home/alex/screenshots/EliteDangerousCLIENT_1581844531372_47099.png"))
    //std::cout << "OCR:\n" << ocr.recognize(img).join("\n").toStdString();
    //      std::cout << "OCR: " << EliteOCR::tryDetectStarFromMapPopup(ocr.recognize(img)).toStdString() << std::endl;
    //    else
    //        std::cout << "failed to load sample image\n";
    //std::cout << "Screenshot:\n" << ocr.recognizeScreen().toStdString() << std::endl;

}

int main(int argc, char *argv[])
{

    SingleApplication a(argc, argv, false, SingleApplication::Mode::SecondaryNotification | SingleApplication::Mode::User);
    a.setApplicationName("ED:HighWay");
    a.setApplicationVersion("0.1");
    a.setApplicationDisplayName("ED:HighWay");
    a.setOrganizationDomain("pasteover.net");
    a.setOrganizationName("pasteover.net");

    //    dotest();
    //    return 0;


    MainWindow w;

    QObject::connect(&a, &SingleApplication::instanceStarted, [&w, &a]()
    {
        //actual popup of window will be dependent on desktop settings, for example in kde it is something like "bring to front demanding attention"
        if (a.activeWindow())
            a.activeWindow()->raise();
        else
            w.raise();
    });

    w.show();

    return a.exec();
}
