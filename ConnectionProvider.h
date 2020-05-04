#ifndef DATABASEPROVIDER_H
#define DATABASEPROVIDER_H

#include <QObject>

class Connector;
class Connection;
class ConnectionProviderPrivate;
class ConnectionProvider : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ConnectionProvider)
public:
    explicit ConnectionProvider(QObject *parent = nullptr);
    ~ConnectionProvider();

    Connection *createConnection(const QString &name);
    Connection *createConnection(const QString &name, const QJsonObject &config);
    Connector *createConnector(const QString &name);
    void releaseConnection(Connection *database);

    void addConnection(const QString &name, const QJsonObject &config);
    QStringList availableDrivers() const;
    QJsonObject configuration(const QString &name) const;
    QString defaultConnection() const;

private:
    QScopedPointer<ConnectionProviderPrivate> d_ptr;
};

#endif // DATABASEPROVIDER_H
