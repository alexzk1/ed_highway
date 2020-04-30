#include "edsmsystemsmodel.h"
#include "stringsfilecache.h"
#include "utils/containers_helpers.h"
#include "utils/guard_on.h"
#include "salesman/LittleAlgorithm.h"

EDSMSystemsModel::EDSMSystemsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(this, &EDSMSystemsModel::DO_NO_CONNECT_THIS_1, this, &EDSMSystemsModel::buildCrossResolve, Qt::QueuedConnection);
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
            return getSystemNameAt(index);

        if (Qt::ToolTipRole == role)
        {
            LOCK_GUARD_ON(lock);
            return QStringLiteral("<p>Selected row is copied + is a 1st system during ordering.</p><hr%1").arg(EDSMWrapper::tooltipWithSysInfo(systemNames.at(row)));
        }
    }

    return QVariant();
}

QStringList EDSMSystemsModel::getSystems() const
{
    LOCK_GUARD_ON(lock);
    return systemNames;
}

QString EDSMSystemsModel::getSystemNameAt(const QModelIndex &index) const
{
    QString r;
    LOCK_GUARD_ON(lock);
    if (index.row() > -1 && index.row() < systemNames.size())
        r = systemNames.at(index.row());
    return r;
}

void EDSMSystemsModel::addSystem(const QString &sys)
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    systemNames.push_back(sys);
    endResetModel();
    emit systemsChanged();
}

void EDSMSystemsModel::removeSystem(const QString &sys)
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    types_ns::remove_if(systemNames, [&sys](const auto & v)
    {
        return v == sys;
    });
    endResetModel();
    emit systemsChanged();
}

void EDSMSystemsModel::addSystems(const QStringList &many_sys)
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    systemNames.append(many_sys);
    endResetModel();
    emit systemsChanged();
}

void EDSMSystemsModel::setSystems(QStringList bulk)
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    systemNames.clear();
    systemNames = std::move(bulk);
    endResetModel();
    emit systemsChanged();
}

void EDSMSystemsModel::clearSystems()
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    systemNames.clear();
    endResetModel();
    emit systemsChanged();
}

void EDSMSystemsModel::startRouteBuild(QString initialSystem)
{
    {
        LOCK_GUARD_ON(lock);
        if (systemNames.empty())
        {
            emit routeReady();
            return;
        }
        if (initialSystem.isEmpty())
            initialSystem = systemNames.at(0);
    }
    routeBuilder = utility::startNewRunner([this, initialSystem](auto)
    {
        QStringList snapshoot;
        {
            LOCK_GUARD_ON(lock);
            snapshoot = systemNames;
        }
        float len;
        auto route = LittleAlgorithm::route(snapshoot, initialSystem, &len);
        {
            LOCK_GUARD_ON(lock);
            routeLen = len;
        }
        emit DO_NO_CONNECT_THIS_1(std::move(route));
    });
}

void EDSMSystemsModel::buildCrossResolve(QStringList route)
{
    //exectues in main thread
    routeBuilder.reset();
    setSystems(route);
    emit routeReady();
}
