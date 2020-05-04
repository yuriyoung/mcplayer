#include "User.h"

#include <QDebug>

User::User(QObject *parent) : Model(parent)
{

}

int User::id() const
{
    return m_id;
}

QString User::name() const
{
    return m_name;
}

QString User::email() const
{
    return m_email;
}

QString User::password() const
{
    return m_password;
}

void User::setId(int id)
{
    if (m_id == id)
        return;

    m_id = id;
    emit idChanged(m_id);
}

void User::setName(QString name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged(m_name);
}

void User::setEmail(QString email)
{
    if (m_email == email)
        return;

    m_email = email;
    emit emailChanged(m_email);
}

void User::setPassword(QString password)
{
    if (m_password == password)
        return;

    m_password = password;
    emit passwordChanged(m_password);
}
