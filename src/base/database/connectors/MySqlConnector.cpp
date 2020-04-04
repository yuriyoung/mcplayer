#include "MySqlConnector.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QJsonObject>
#include <QJsonValue>

MySqlConnector::MySqlConnector(const QString &name, QObject *parent)
    : Connector(name, parent)
{

}

MySqlConnector::MySqlConnector(const QString &name, const QJsonObject &config, QObject *parent)
    : Connector(name, config, parent)
{

}

QSqlDatabase MySqlConnector::connect(const QJsonObject &config)
{
    QString database = this->config().value("database").toString();
    QSqlDatabase db = this->createConnection(database, config);
    this->setEncoding(&db);
    this->setTimezone(&db);

    return db;
}

void MySqlConnector::setTimezone(QSqlDatabase *db)
{
    if(!this->config().value("timezone").isNull())
    {
        QString tz = this->config().value("timezone").toString();
        db->exec(QString("set time_zone=\"%1\"").arg(tz));
    }
}

void MySqlConnector::setEncoding(QSqlDatabase *db)
{
    if(!this->config().value("charset").isNull())
    {
        QString charset = this->config().value("charset").toString();
        QString collate = this->config().value("collation").toString();
        collate = collate.isEmpty() ? "" : QString("collate \"%1\"").arg(collate);

        db->exec(QString("set names \"%1\"%2").arg(charset).arg(collate));
    }
}
