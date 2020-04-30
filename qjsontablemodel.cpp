#include "qjsontablemodel.h"
#include <QJsonObject>
#include "edsmwrapper.h"

QJsonTableModel::QJsonTableModel( const QJsonTableModel::Header& header, QObject * parent )
    : QAbstractTableModel( parent )
    , m_header( header )
{

}

bool QJsonTableModel::setJson(const QJsonDocument &json)
{
    return setJson( json.array() );
}

bool QJsonTableModel::setJson( const QJsonArray& array )
{
    beginResetModel();
    m_json = array;
    endResetModel();
    return true;
}

QVariant QJsonTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role != Qt::DisplayRole )
        return QVariant();

    switch ( orientation )
    {
        case Qt::Horizontal:
            return m_header[section]["title"];
        case Qt::Vertical:
            //return section + 1;
            return QVariant();
        default:
            return QVariant();
    }

}

int QJsonTableModel::rowCount(const QModelIndex & ) const
{
    return m_json.size();
}

int QJsonTableModel::columnCount(const QModelIndex & ) const
{
    return m_header.size();
}


QJsonObject QJsonTableModel::getJsonObject( const QModelIndex &index ) const
{
    const QJsonValue& value = m_json[index.row() ];
    return value.toObject();
}

QVariant QJsonTableModel::data( const QModelIndex &index, int role ) const
{
    QJsonObject obj = getJsonObject( index );
    switch ( role )
    {
        case Qt::DisplayRole:
        {
            const QString& key = m_header[index.column()]["index"];
            if ( obj.contains( key ))
            {
                QJsonValue v = obj[ key ];

                if ( v.isString() )
                    return v.toString();
                else
                    if ( v.isDouble() )
                        return QString::number( v.toDouble() );
                    else
                        return QVariant();
            }
            else
                return QVariant();
        }
        case Qt::ToolTipRole:
        {
            const QString& key = m_header[index.column()]["index"];
            if ( obj.contains( key ))
            {
                QJsonValue v = obj[ key ];
                if ( v.isString() )
                    return QStringLiteral("<p>Selected row is copied.</p><hr>%1").arg(EDSMWrapper::tooltipWithSysInfo(v.toString()));
            }
        }
        default:
            break;
    }
    return QVariant();
}
