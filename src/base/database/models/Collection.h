#ifndef COLLECTION_H
#define COLLECTION_H

#include <QObject>
#include <QVector>
#include <QJsonObject>

class Collection : public QVector<QObject *>
{
public:
    Collection();
    Collection(QObject *obj);

    Collection &operator<<(QObject *obj);

    QJsonObject toJson() const;

};

#endif // COLLECTION_H
