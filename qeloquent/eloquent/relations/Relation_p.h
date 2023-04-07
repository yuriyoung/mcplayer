#ifndef RELATION_P_H
#define RELATION_P_H

#include <QObject>

class EloquentBuilder;
class Relation;
class Model;

class RelationPrivate
{
    Q_DECLARE_PUBLIC(Relation)
public:
    explicit RelationPrivate(Relation *q);
    virtual ~RelationPrivate();

    Relation *q_ptr = nullptr;

    EloquentBuilder *query = nullptr;
    Model *related = nullptr;
    Model *parent = nullptr;
};

#endif // RELATION_P_H
