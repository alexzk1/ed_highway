#pragma once

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QObject>
#include <QVector>

#include <qcontainerfwd.h>
#include <qnamespace.h>

#include <cstdint>

class QJsonTableModel : public QAbstractTableModel
{
  public:
    enum class VerticalNums : std::uint8_t {
        NONE,
        BASEZERO,
        BASEONE
    };
    using Heading = QMap<QString, QString>;
    using Header = QVector<Heading>;
    explicit QJsonTableModel(Header header, QObject *parent = nullptr,
                             VerticalNums nums = VerticalNums::NONE);

    bool setJson(const QJsonDocument &json);
    bool setJson(const QJsonArray &array);

    [[nodiscard]]
    virtual QJsonObject getJsonObject(const QModelIndex &index) const;

    [[nodiscard]]
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    [[nodiscard]]
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]]
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]]
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  private:
    const VerticalNums vhdr;
    Header m_header;
    QJsonArray m_json;
};
