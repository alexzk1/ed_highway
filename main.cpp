#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include "spanshapi.h"
#include "spansh_route.h"
#include "spansh_sysname.h"

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
    //    QApplication a(argc, argv);
    //    MainWindow w;
    //    w.show();

    //    return a.exec();

    dotest();
}
