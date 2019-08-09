#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include "spanshapi.h"
#include "spansh_route.h"
#include "spansh_sysname.h"
#include "singleapp/singleapplication.h"

void dotest()
{
    SpanshApi test;
    SpanshRoute r{70, 70, "Sol", "Colonia"};
    SpanshSysName sys{"eu"};

    const auto static testr = [](auto err, auto js)
    {
        if (!err.empty())
            std::cerr << err << std::endl;
        else
            std::cout << js.dump(4) << std::endl;
    };

    test.executeRequest(r, testr);
    test.executeRequest(sys, testr);
    test.threads.stop(true);
}

int main(int argc, char *argv[])
{
    //dotest();

    SingleApplication a(argc, argv, false, SingleApplication::Mode::SecondaryNotification | SingleApplication::Mode::User);
    a.setApplicationName("ED:HighWay");
    a.setApplicationVersion("0.1");
    a.setApplicationDisplayName("ED:HighWay");
    a.setOrganizationDomain("pasteover.net");
    a.setOrganizationName("pasteover.net");
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
