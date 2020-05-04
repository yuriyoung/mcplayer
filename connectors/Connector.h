#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QObject>

/**
 * @brief The Connector class provided a connect for the database
 */

class QSqlDatabase;
class QJsonObject;
class ConnectorPrivate;
class Connector : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Connector)
public:
    explicit Connector(const QString &name, QObject *parent = nullptr);
    explicit Connector(const QString &name, const QJsonObject &config, QObject *parent = nullptr);
    ~Connector();

    virtual QSqlDatabase connect();
    virtual QSqlDatabase connect(const QJsonObject &config);
    virtual void disconnect();

    QString connectionName() const;
    QString driverName() const;

    QJsonObject config() const;
    void setConfig(const QJsonObject &config);

    QString options() const;
    void setOptions(const QString &options);

protected:
    QScopedPointer<ConnectorPrivate> d_ptr;
    QSqlDatabase createConnection(const QString &database, const QJsonObject &config, const QString &options = {});
};

#endif // CONNECTOR_H
