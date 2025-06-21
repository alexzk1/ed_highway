#include "qjsontablemodel.h"

#include "edsmwrapper.h"

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QVector>

#include <qcontainerfwd.h>
#include <qnamespace.h>
#include <qstringliteral.h>

#include <utility>

QJsonTableModel::QJsonTableModel(QJsonTableModel::Header header, QObject *parent,
                                 VerticalNums nums) :
    QAbstractTableModel(parent),
    vhdr(nums),
    m_header(std::move(header))
{
}

bool QJsonTableModel::setJson(const QJsonDocument &json)
{
    return setJson(json.array());
}

bool QJsonTableModel::setJson(const QJsonArray &array)
{
    beginResetModel();
    m_json = array;
    endResetModel();
    return true;
}

QVariant QJsonTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return {};
    }

    switch (orientation)
    {
        case Qt::Horizontal:
            return m_header[section]["title"];
        case Qt::Vertical: {
            if (vhdr == VerticalNums::BASEZERO)
            {
                return section;
            }
            if (vhdr == VerticalNums::BASEONE)
            {
                return section + 1;
            }
            return {};
        }
        default:
            return {};
    }
}

int QJsonTableModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(m_json.size());
}

int QJsonTableModel::columnCount(const QModelIndex &) const
{
    return static_cast<int>(m_header.size());
}

QJsonObject QJsonTableModel::getJsonObject(const QModelIndex &index) const
{
    const QJsonValue &value = m_json[index.row()];
    return value.toObject();
}

QVariant QJsonTableModel::data(const QModelIndex &index, int role) const
{
    const QJsonObject obj = getJsonObject(index);
    switch (role)
    {
        case Qt::DisplayRole: {
            const QString &key = m_header[index.column()]["index"];
            if (obj.contains(key))
            {
                const QJsonValue v = obj[key];

                if (v.isString())
                {
                    return v.toString();
                }
                else if (v.isDouble())
                {
                    return QString::number(v.toDouble());
                }
                else
                {
                    return {};
                }
            }
            else
            {
                return {};
            }
        }
        case Qt::ToolTipRole: {
            const QString &key = m_header[index.column()]["index"];
            if (obj.contains(key))
            {
                const QJsonValue v = obj[key];
                if (v.isString())
                {
                    return QStringLiteral("<p>Selected row is copied.</p><hr>%1")
                      .arg(EDSMWrapper::tooltipWithSysInfo(v.toString()));
                }
            }
        }
        default:
            break;
    }
    return {};
}
