#ifndef USER_H
#define USER_H

#include "Model.h"

class User : public Model
{
    Q_OBJECT
    Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool verified)
public:
    explicit User(QObject *parent = nullptr);

    int id() const;
    QString name() const;
    QString email() const;
    QString password() const;
    bool verified() const { return m_verified; }

signals:
    void idChanged(int id);
    void nameChanged(QString name);
    void emailChanged(QString email);
    void passwordChanged(QString password);

public slots:
    void setId(int id);
    void setName(QString name);
    void setEmail(QString email);
    void setPassword(QString password);

private:
    int m_id = 1;
    QString m_name{"Test"};
    QString m_email;
    QString m_password;
    bool m_verified;
};

#endif // USER_H
