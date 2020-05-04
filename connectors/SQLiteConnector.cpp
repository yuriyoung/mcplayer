#include "SQLiteConnector.h"

#include <QSqlDatabase>
#include <QJsonObject>
#include <QJsonValue>

const static QString DatabaseKey      = "database";

SQLiteConnector::SQLiteConnector(const QString &name, QObject *parent)
    : Connector(name, parent)
{

}

SQLiteConnector::SQLiteConnector(const QString &name, const QJsonObject &config, QObject *parent)
    : Connector(name, config, parent)
{

}
