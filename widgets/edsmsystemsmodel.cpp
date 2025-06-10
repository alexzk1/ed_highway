#include "edsmsystemsmodel.h"

#include "execonmainthread.h"
#include "salesman/LittleAlgorithm.h"
#include "stringsfilecache.h"
#include "utils/containers_helpers.h"
#include "utils/guard_on.h"

#include <QClipboard>
#include <QGuiApplication>

EDSMSystemsModel::EDSMSystemsModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

QVariant EDSMSystemsModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    const static QString headers[] = {
      tr("System Name"),
      tr("Distances"),
    };

    const size_t index = static_cast<size_t>(section);

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (index < types_ns::countof(headers))
            return headers[section];
    }

    if (role == Qt::DisplayRole && orientation == Qt::Vertical)
        return section;

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

    return 2;
}

QVariant EDSMSystemsModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const int col = index.column();

    if (index.isValid())
    {
        if (Qt::DisplayRole == role)
        {
            if (col == 0)
                return getSystemNameAt(index);
            if (col == 1)
                return distances.at(row);
        }
        if (Qt::ToolTipRole == role)
        {
            LOCK_GUARD_ON(lock);
            return QStringLiteral(
                     "<p>Selected row is copied + is a 1st system during ordering.</p><hr%1")
              .arg(EDSMWrapper::tooltipWithSysInfo(systemNames.at(row)));
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
    fillDistances();
    endResetModel();
    emit systemsChanged();
}

void EDSMSystemsModel::removeSystem(const QString &sys)
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    types_ns::remove_if(systemNames, [&sys](const auto &v) {
        return v == sys;
    });
    fillDistances();
    endResetModel();
    emit systemsChanged();
}

void EDSMSystemsModel::addSystems(const QStringList &many_sys)
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    systemNames.append(many_sys);
    fillDistances();
    endResetModel();
    emit systemsChanged();
}

void EDSMSystemsModel::setSystems(QStringList bulk)
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    systemNames.clear();
    systemNames = std::move(bulk);
    fillDistances();
    endResetModel();
    emit systemsChanged();
}

void EDSMSystemsModel::clearSystems()
{
    LOCK_GUARD_ON(lock);
    beginResetModel();
    systemNames.clear();
    fillDistances();
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
    routeBuilder = utility::startNewRunner([this, initialSystem](auto) {
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

        ExecOnMainThread::get().exec([route, this]() {
            routeBuilder.reset();
            setSystems(std::move(route));
            emit routeReady();
        });
    });
}

void EDSMSystemsModel::copyCurrentList() const
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard)
        clipboard->setText(systemNames.join("\n"));
}

void EDSMSystemsModel::fillDistances()
{
    const auto static point = [](const QString &name) {
        Point p;
        try
        {
            const auto json = EDSMWrapper::requestSysInfo(name);
            p = Point::fromJson(json);
        }
        catch (...)
        {
        }
        return p;
    };
    distances.clear();
    const auto sz = systemNames.size();
    distances.reserve(sz);
    distances.push_back(0);
    if (sz > 1)
    {
        float sum = 0.f;
        Point prev = point(systemNames.first());
        for (int i = 1; i < sz; ++i)
        {
            auto p = point(systemNames.at(i));
            sum += prev.distance(p);
            prev = std::move(p);
            distances.push_back(sum);
        }
    }
}
