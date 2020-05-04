#include "Connector.h"

#include <QSqlDatabase>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

const static QString OptionsKey     = "options";
const static QString DriverKey      = "driver";
const static QString DatabaseKey    = "database";
const static QString UsernameKey    = "username";
const static QString PasswordKey    = "password";
const static QString HostKey        = "host";
const static QString PortKey        = "port";

class ConnectorPrivate
{
public:
    QJsonObject config;
    QString options;
    QString connectionName;
    QString driver;
    QString database;
    QString host;
    QString username;
    QString password;
    int port;
};

Connector::Connector(const QString &name, QObject *parent)
    : QObject(parent), d_ptr(new ConnectorPrivate)
{
    Q_D(Connector);
    d->connectionName = name;
}

Connector::Connector(const QString &name, const QJsonObject &config, QObject *parent)
    : QObject(parent), d_ptr(new ConnectorPrivate)
{
    Q_D(Connector);
    d->config = config;
    d->options = config.value(OptionsKey).toString();
    d->connectionName = name;
}

Connector::~Connector()
{
    qDebug() << "Connector::~Connector()";
    this->disconnect();
}

QSqlDatabase Connector::connect()
{
    Q_D(Connector);
    return this->connect(d->config);
}

QSqlDatabase Connector::connect(const QJsonObject &config)
{
    Q_D(Connector);

    if(QSqlDatabase::database(d->connectionName).isValid())
        return QSqlDatabase::database(d->connectionName);

    if(config.isEmpty()
            || config.value(DriverKey).isNull()
            || config.value(DatabaseKey).isNull())
        return QSqlDatabase::database();

    QString driver = config.value(DriverKey).toString();
    QString database = config.value(DatabaseKey).isNull() ? "" : config.value(DatabaseKey).toString();
    if(driver.compare("sqlite", Qt::CaseInsensitive) == 0)
    {
        // SQLite database file name
        // TODO: verify the file name is valid or no.
        if(database.isEmpty())
        {
            // TODO: throw a invalid Exception ?
            d->driver = driver;
            d->database = database;
            return QSqlDatabase::database();
        }
    }

    return createConnection(database, config);
}

void Connector::disconnect()
{
    Q_D(Connector);
    QSqlDatabase::database(d->connectionName).close();
}

QString Connector::connectionName() const
{
    Q_D(const Connector);
    return d->connectionName;
}

QString Connector::driverName() const
{
    Q_D(const Connector);
    return d->driver;
}

QJsonObject Connector::config() const
{
    Q_D(const Connector);
    return d->config;
}

void Connector::setConfig(const QJsonObject &config)
{
    Q_D(Connector);
    d->config = config;
}

QString Connector::options() const
{
    Q_D(const Connector);
    return d->options;
}

void Connector::setOptions(const QString &options)
{
    Q_D(Connector);
    if(d->options == options)
        return;

    d->options = options;
}

QSqlDatabase Connector::createConnection(const QString &database, const QJsonObject &config, const QString &options)
{
    Q_D(Connector);
    d->driver = config.value(DriverKey).toString();
    d->driver = d->driver.toUpper();
    d->driver = d->driver.startsWith("Q") ? d->driver : d->driver.prepend("Q");

    d->database = database;
    d->username = config.value(UsernameKey).isNull() ? "" : config.value(UsernameKey).toString();
    d->password = config.value(PasswordKey).isNull() ? "" : config.value(PasswordKey).toString();
    d->host = config.value(HostKey).isNull() ? "" : config.value(HostKey).toString();
    d->port = config.value(PortKey).isNull() ? 0 : config.value(PortKey).toInt();

    // options
    QString opts = config.value(OptionsKey).toString();
    if(!options.isEmpty())
        opts.append(";").append(options);

    QSqlDatabase db = QSqlDatabase::addDatabase(d->driver, d->connectionName);
    db.setDatabaseName(d->database);
    db.setUserName(d->username);
    db.setPassword(d->password);

    if(!d->host.isEmpty())
        db.setHostName(d->host);
    if(d->port > 0)
        db.setPort(d->port);
    if(!opts.isEmpty())
        db.setConnectOptions(options);

    db.open();

    d->config = config;
    d->options = db.connectOptions();

    qDebug() << "connected:" << d->connectionName << d->database;

    return db;
}
