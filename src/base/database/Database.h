#ifndef DATABASE_H
#define DATABASE_H

#include "query/QueryBuilder.h"
#include "schema/SchemaBuilder.h"

#include <QObject>
#include <QMap>

/**
 * TODO: create a class to parse the json configs
 *
 * // database.json config for connection
 * {
 *      "migrations": "migrations",
 *      "default": "sqlite",
 *      "connections": {
 *          "sqlite_0": {
 *              "driver": "sqlite",
 *              "url": "URL",
 *              "databse": "database_path",
 *              "prefix": "",
 *              "username": "",
 *              "password": ""
 *          },
 *          "sqlite_1": {
 *              "driver": "sqlite",
 *              "url": "URL",
 *              "databse": "database_another_path",
 *              "prefix": "",
 *              "username": "",
 *              "password": ""
 *          },
 *          "mysql": {
 *              "driver": "mysql",
 *              "url": "URL",
 *              "host": "127.0.0.1",
 *              "port": "3306",
 *              "databse": "forge",
 *              "username": "forge",
 *              "password": "",
 *              "charset": "utf8mb4",
 *              "collation": "utf8mb4_unicode_ci",
 *              "prefix": "",
 *              "strict": true
 *          }
 *          "sqlsrv": {...} // ms sql server
 *
 *          // other database driver
 *          // ...
 *      }
 * }
 *
 */

class Connection;
class Connector;
class ConnectionProvider;

class DatabasePrivate;
class Database : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Database)
    QScopedPointer<DatabasePrivate> d_ptr;
public:
    ~Database();

    static Database *instance();
    static QueryBuilder table(const QString &table, const QString &as = "", const QString &connection = "");
    static SchemaBuilder schema(const QString &connection = "");

    // get a datanase connection by specified config name, not a connection name
    // make a connection if dose not exists
    Connection *connection(const QString &connection = {});
    Connection *reconnect(const QString &connection = {});
    void disconnect(const QString &name = {});

    // add a configure for new connection
    void addConnection(const QJsonObject &config, const QString &name = "default");

private:
    explicit Database(QObject *parent = nullptr);
};

#endif // DATABASE_H
