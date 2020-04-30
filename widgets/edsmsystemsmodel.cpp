#include "edsmsystemsmodel.h"
#include "stringsfilecache.h"
#include "utils/containers_helpers.h"
#include "utils/guard_on.h"

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
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    LOCK_GUARD_ON(lock);
    return systemNames.size();
}

int EDSMSystemsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 1;
}

QVariant EDSMSystemsModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    //const int col = index.column();

    if (index.isValid())
    {
        if (Qt::DisplayRole == role)
        {
            LOCK_GUARD_ON(lock);
            if (row > -1 && row < systemNames.size())
                return systemNames.at(row);
        }
    }

    return QVariant();
}

void EDSMSystemsModel::addSystem(const QString &sys)
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    systemNames.push_back(sys);
    endResetModel();
}

void EDSMSystemsModel::addSystems(const QStringList &many_sys)
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    systemNames.append(many_sys);
    endResetModel();
}

void EDSMSystemsModel::setSystems(QStringList bulk)
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    systemNames = std::move(bulk);
    endResetModel();
}
