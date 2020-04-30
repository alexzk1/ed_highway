#include "edsmsystemsmodel.h"
#include "stringsfilecache.h"
#include "utils/containers_helpers.h"

EDSMSystemsModel::EDSMSystemsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant EDSMSystemsModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    const static QString headers[] =
    {
        tr("System Name"),
    };

    const size_t index = static_cast<size_t>(section);

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (index < types_ns::countof(headers))
            return headers[section];
    }
    return QVariant();
}

int EDSMSystemsModel::rowCount(const QModelIndex &parent) const
{
    //if (parent.isValid())


    // FIXME: Implement me!
    return 1;
}

int EDSMSystemsModel::columnCount(const QModelIndex &parent) const
{
    //if (parent.isValid())


    // FIXME: Implement me!
    return 1;
}

QVariant EDSMSystemsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (Qt::DisplayRole == role)
            return "1";

    }

    // FIXME: Implement me!
    return QVariant();
}
