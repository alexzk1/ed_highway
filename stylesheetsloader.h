#pragma once

#include <QObject>

class StyleSheetsLoader : public QObject
{
    Q_OBJECT
  public:
    explicit StyleSheetsLoader(QObject *parent = nullptr);

  signals:
};
