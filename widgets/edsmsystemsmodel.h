#ifndef EDSMSYSTEMSMODEL_H
#define EDSMSYSTEMSMODEL_H

#include "utils/runners.h"

#include <QAbstractTableModel>

#include <mutex>
#include <vector>

class EDSMSystemsModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    explicit EDSMSystemsModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QStringList getSystems() const;

    QString getSystemNameAt(const QModelIndex &index) const;

  public slots:

    // add 1 sys
    void addSystem(const QString &sys);

    void removeSystem(const QString &sys);

    // add many sys
    void addSystems(const QStringList &many_sys);

    // replace anything it have by "bulk" list
    void setSystems(QStringList bulk);

    void clearSystems();

    void startRouteBuild(QString initialSystem);

    void copyCurrentList() const;
  private slots:
  signals:
    void routeReady();
    void systemsChanged();

  private:
    float routeLen{0.f};
    QStringList systemNames;
    std::vector<float> distances;
    mutable std::recursive_mutex lock;
    utility::runner_t routeBuilder{nullptr};

    void fillDistances();
};

#endif // EDSMSYSTEMSMODEL_H
