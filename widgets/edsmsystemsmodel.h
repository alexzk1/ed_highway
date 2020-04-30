#ifndef EDSMSYSTEMSMODEL_H
#define EDSMSYSTEMSMODEL_H

#include <mutex>
#include <QAbstractTableModel>


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
public slots:

    //add 1 sys
    void addSystem(const QString& sys);

    //add many sys
    void addSystems(const QStringList& many_sys);

    //replace anything it have by "bulk" list
    void setSystems(QStringList bulk);
private:
    QStringList systemNames;
    mutable std::recursive_mutex lock;
};

#endif // EDSMSYSTEMSMODEL_H
