#include "Collection.h"

Collection::Collection()
{

}

Collection::Collection(QObject *obj)
{
    *this << obj;
}

Collection &Collection::operator<<(QObject *obj)
{
    QVector<QObject *>::operator<<(obj);
    return *this;
}

QJsonObject Collection::toJson() const
{
//    for (const auto &o : *this)
//    {

//    }

    return QJsonObject();
}
