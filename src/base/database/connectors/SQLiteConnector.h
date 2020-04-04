#ifndef SQLITECONNECTOR_H
#define SQLITECONNECTOR_H

#include "Connector.h"

class SQLiteConnector : public Connector
{
    Q_OBJECT
public:
    explicit SQLiteConnector(const QString &name, QObject *parent = nullptr);
    explicit SQLiteConnector(const QString &name, const QJsonObject &config, QObject *parent = nullptr);
};

#endif // SQLITECONNECTOR_H
