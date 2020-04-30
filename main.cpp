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
#include "stringsfilecache.h"
#include "edsmwrapper.h"
#include "dump_help.h"
#include "salesman/LittleAlgorithm.h"

void test_ocr()
{
#ifdef SRC_PATH
    const static QString tests[] =
    {
        // PHROI PRI NX-A D1-785
        "EliteDangerousCLIENT_1581844531372_47099.png",

        //HYPOE FLYI QB-N C23-3651 1 (planet)
        "EliteDangerousCLIENT_1581953981012_111958.png",

        //HYPOE FLYI QB-N C23-3651 2 A (planet)
        "EliteDangerousCLIENT_1581953984790_111968.png",

        //HYPOE FLYI QB-N C23-3651 4 E (planet)
        "EliteDangerousCLIENT_1581953988331_111976.png",

        //those 2 below has many stars on BG

        //HYPOE FLYI LA-P C22-1092
        "EliteDangerousCLIENT_1581954014555_111994.png",

        //HYPOE FLYI FN-H D11-1555
        "EliteDangerousCLIENT_1581954025449_112006.png",
    };
    EliteOCR ocr;
    const static auto p = QStringLiteral("%1/test_ocr_screens/").arg(SRC_PATH);
    for (const auto& s : tests)
    {
        QImage img;
        if (img.load(p + s))
            std::cout << "OCR: " << EliteOCR::tryDetectStarFromMapPopup(ocr.recognize(img)).toStdString() << std::endl;

    }
    exit(0);
#endif
}

void dotest()
{
#ifdef SRC_PATH
    //    SpanshApi test {3};
    //    SpanshRoute r{70, 70, "Sol", "Colonia"};
    //    SpanshSysName sys{"eu"};

    EdsmApiV1 test{3};

    EDSMV1NearerstSystem r("Borann", 20, false);
    EDSMV1SysInfo sys("Maia");

    const auto static testr = [](auto err, auto js)
    {
        if (!err.empty())
            std::cerr << err << std::endl;
        else
        {
            std::cout << js.dump(4) << std::endl;
            StringsFileCache::get().addData("test", QString::fromStdString(js.dump()));
        }
    };

    test.executeRequest(r, testr);
    test.executeRequest(sys, testr);
    test.threads.stop(true);
    exit(0);
#endif
}

void test_wrapper()
{
#ifdef SRC_PATH
    auto list = EDSMWrapper::requestManySysInfoInRadius("Borann", 30, [](auto a, auto b)
    {
        if (a % 10 == 0)
            std::cout << a << " out of " << b << std::endl;
    });
    dump_helper::dumpContainer(list);

    std::cout << EDSMWrapper::tooltipWithSysInfo("Brigh").toStdString() << std::endl;

    exit(0);
#endif
}


int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv, false, SingleApplication::Mode::SecondaryNotification | SingleApplication::Mode::User);
    a.setApplicationName("ED:HighWay");
    a.setApplicationVersion("0.14");
    a.setApplicationDisplayName("ED:HighWay");
    a.setOrganizationDomain("pasteover.net");
    a.setOrganizationName("pasteover.net");

    StringsFileCache::get(); //init cache

    //test_wrapper();
    //LittleAlgorithm::selfTest3();
    //exit(0);

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
