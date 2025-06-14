#ifndef QJSONTABLEMODEL_H
#define QJSONTABLEMODEL_H

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QObject>
#include <QVector>

class QJsonTableModel : public QAbstractTableModel
{
  public:
    enum class VerticalNums {
        NONE,
        BASEZERO,
        BASEONE
    };
    typedef QMap<QString, QString> Heading;
    typedef QVector<Heading> Header;
    QJsonTableModel(const Header &header, QObject *parent = 0,
                    VerticalNums nums = VerticalNums::NONE);

    bool setJson(const QJsonDocument &json);
    bool setJson(const QJsonArray &array);

    virtual QJsonObject getJsonObject(const QModelIndex &index) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

  private:
    const VerticalNums vhdr;
    Header m_header;
    QJsonArray m_json;
};

#endif // QJSONTABLEMODEL_H
