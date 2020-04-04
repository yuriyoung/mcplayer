#include "ConnectionProvider.h"
#include "MySqlConnection.h"
#include "SQLiteConnection.h"
#include "connectors/MySqlConnector.h"
#include "connectors/SQLiteConnector.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QFile>
#include <QDebug>

class ConnectionProviderPrivate
{
public:
    Connection *create(const QString &driver, const QString &prefix)
    {
        if(driver.compare("qmysql", Qt::CaseInsensitive) == 0)
        {
            return new MySqlConnection(prefix);
        }
        else if(driver.compare("qsqlite", Qt::CaseInsensitive) == 0)
        {
            return new SQLiteConnection(prefix);
        }

        // TODO: throw a Invalid argument exception
        qCritical() << "Unsupported driver:" << driver;

        // return SQLite connection default ?
        return new SQLiteConnection(prefix);
    }

    void loadConfig()
    {
        QFile file(":/config/database.json");
        if(!file.open(QIODevice::ReadOnly))
        {
            // TODO: throw a exception
            qCritical() << file.errorString() << file.fileName();
            return ;
        }
        QByteArray json = file.readAll();
        file.close();

        jDoc = QJsonDocument::fromBinaryData(json);
        if(jDoc.isNull())
            jDoc = QJsonDocument::fromJson(json);
    }

    QJsonDocument jDoc;
};

ConnectionProvider::ConnectionProvider(QObject *parent)
    : QObject(parent), d_ptr(new ConnectionProviderPrivate)
{
    d_ptr->loadConfig();
}

ConnectionProvider::~ConnectionProvider()
{

}

Connection *ConnectionProvider::createConnection(const QString &name)
{
    Q_D(ConnectionProvider);
    QJsonObject config = this->configuration(name);
    QString prefix = config.value("prefix").toString();
    QString driver = config.value("driver").toString();
    driver = driver.startsWith("q") ? driver : driver.append("q");

    Connection *conn = d->create(driver, prefix);
    conn->setConnector(createConnector(name));

    return conn;
}

Connection *ConnectionProvider::createConnection(const QString &name, const QJsonObject &config)
{
    Q_D(ConnectionProvider);
    QString prefix = config.value("prefix").toString();
    QString driver = config.value("driver").toString();
    driver = driver.startsWith("q") ? driver : driver.append("q");

    Connection *conn = d->create(driver, prefix);
    conn->setConnector(createConnector(name));

    return conn;
}

Connector *ConnectionProvider::createConnector(const QString &name)
{
    QJsonObject config = this->configuration(name);
    QString driver = config.value("driver").toString();
    driver = driver.startsWith("q") ? driver : driver.append("q");

    Connector *connector = nullptr;
    if(driver.compare("qmysql", Qt::CaseInsensitive) == 0)
    {
        connector = new MySqlConnector(name, config);
    }
    else if(driver.compare("qsqlite", Qt::CaseInsensitive) == 0)
    {
        connector = new SQLiteConnector(name, config);
    }
    else
    {
        // TODO: throw a Invalid argument exception
        qCritical() << "Unsupported driver:" << driver;
        connector = new SQLiteConnector("");
    }

    return connector;
}

void ConnectionProvider::releaseConnection(Connection *database)
{
    if(database)
    {
        database->disconnect();
        delete database;
    }
}

void ConnectionProvider::addConnection(const QString &name, const QJsonObject &config)
{
    Q_D(ConnectionProvider);
    QJsonObject jobject = d->jDoc.object();
    QJsonObject connections = jobject.value("connections").toObject();
    connections.insert(name, config);
    jobject["connections"] = connections;
    d->jDoc.setObject(jobject);
}

QStringList ConnectionProvider::availableDrivers() const
{
    return QSqlDatabase::drivers();
}

/**
 * TODO: get configuration by specified the name if have in config file, otherwise throw a error
 * TODO: parse configuration to get driver, database name, host, port, username, password
 * TODO: create a json parse class for parse configuration
 * @brief get connection configuration by specified connection name
 * @param name
 * @return
 */
QJsonObject ConnectionProvider::configuration(const QString &name) const
{
    Q_D(const ConnectionProvider);
    QJsonObject jobject = d->jDoc.object();
    QJsonObject connections = jobject.value("connections").toObject();
    return connections.value(name).toObject();
}

QString ConnectionProvider::defaultConnection() const
{
     Q_D(const ConnectionProvider);
    QJsonObject jobject = d->jDoc.object();
    return jobject.value("default").toString();
}
