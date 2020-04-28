#pragma once
#include <QString>
#include <QStandardPaths>
#include <QDir>

QString inline getWritableLocation()
{
    static QString s = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (s.isEmpty())
    {
        s = QDir::homePath(); //fallback
        if (s.isEmpty())
            throw std::runtime_error("Can not detect writable location to store data.");
    }
    return s;
}

QString inline getWritableLocationApp()
{
    return getWritableLocation() + "/ED_Highway";
}
