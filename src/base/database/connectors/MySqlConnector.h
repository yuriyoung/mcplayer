#ifndef MYSQLCONNECTOR_H
#define MYSQLCONNECTOR_H

#include "Connector.h"

/**
 * NOTE: unused
 *
 * @brief The MySqlConnector class
 */
class MySqlConnector : public Connector
{
    Q_OBJECT
public:
    explicit MySqlConnector(const QString &name, QObject *parent = nullptr);
    MySqlConnector(const QString &name, const QJsonObject &config, QObject *parent = nullptr);

    virtual QSqlDatabase connect(const QJsonObject &config) override;

protected:
    void setTimezone(QSqlDatabase *db);
    void setEncoding(QSqlDatabase *db);
};

#endif // MYSQLCONNECTOR_H
