#ifndef EDSMSYSTEMSMODEL_H
#define EDSMSYSTEMSMODEL_H

#include <mutex>
#include <QAbstractTableModel>
#include "utils/runners.h"

class EDSMSystemsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit EDSMSystemsModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QStringList getSystems() const;

    QString getSystemNameAt(const QModelIndex &index) const;

public slots:

    //add 1 sys
    void addSystem(const QString& sys);

    void removeSystem(const QString& sys);

    //add many sys
    void addSystems(const QStringList& many_sys);

    //replace anything it have by "bulk" list
    void setSystems(QStringList bulk);

    void clearSystems();

    void startRouteBuild(QString initialSystem);
private slots:
signals:
    void routeReady();
    void systemsChanged();
private:
    float routeLen{0.f};
    QStringList systemNames;
    mutable std::recursive_mutex lock;
    utility::runner_t routeBuilder{nullptr};
};

#endif // EDSMSYSTEMSMODEL_H
