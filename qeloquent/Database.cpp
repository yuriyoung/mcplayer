#include "Database.h"
#include "ConnectionProvider.h"
#include "Connection.h"

#include <QJsonObject>
#include <QDebug>

//Q_GLOBAL_STATIC(Database, g_instance)
static Database *g_instance = nullptr;

class DatabasePrivate
{
public:

    /**
     * @brief Database::makeConnection
     * @param name the connection name
     * @return
     */
    Connection *makeConnection(const QString &name)
    {
        Connection *connection = provider->createConnection(name);
        connection->setReconnection(reconnector);
        return connection;
    }

    /**
     * @brief make a new connector to refresh
     * @param name the connection name
     * @return
     */
    Connection *refreshConnection(const QString &name)
    {
        Connector *connector = provider->createConnector(name);
        connections[name]->setConnector(connector);
        return connections[name];
    }

    ConnectionProvider *provider = nullptr;
    Connection::Closure reconnector = nullptr;
    QMap<QString, Connection *> connections;
};

Database::Database(QObject *parent)
    : QObject(parent), d_ptr(new DatabasePrivate)
{
    Q_D(Database);
    g_instance = this;
    d->provider = new ConnectionProvider(this);
    d->reconnector = [this](Connection *db) -> bool {
        return this->reconnect(db->connectionName()) != nullptr;
    };
}

Database::~Database()
{
    qDebug() << "Database::~Database()";
    Q_D(Database);
    foreach (auto conn, d->connections)
        d->provider->releaseConnection(conn);

    delete d->provider;
}

Database *Database::instance()
{
    if(!g_instance)
        g_instance = new Database();

    return g_instance;
}

QueryBuilder Database::table(const QString &connection)
{
    return Database::instance()->connection(connection)->queryBuilder();
}

SchemaBuilder Database::schema(const QString &connection)
{
    return  Database::instance()->connection(connection)->schemaBuilder();
}

Connection *Database::connection(const QString &connection)
{
    Q_D(Database);

    QString connName = connection.isEmpty() ? d->provider->defaultConnection() : connection;
    if(!d->connections.contains(connName))
    {
        d->connections.insert(connName, d->makeConnection(connName));
    }

    return d->connections[connName];
}

Connection *Database::reconnect(const QString &connection)
{
    Q_D(Database);
    QString connName = connection.isEmpty() ? d->provider->defaultConnection() : connection;
    this->disconnect(connName);

    if(!d->connections.contains(connName))
    {
        return this->connection(connName);
    }

    return d->refreshConnection(connName);
}

void Database::disconnect(const QString &name)
{
    Q_D(Database);
    QString connName = name.isEmpty() ? d->provider->defaultConnection() : name;

    if(d->connections.contains(connName))
    {
        d->connections[connName]->disconnect();
    }
}

void Database::addConnection(const QJsonObject &config, const QString &name)
{
    Q_D(Database);
    // just add a configure for new connection
    d->provider->addConnection(name, config);
}
