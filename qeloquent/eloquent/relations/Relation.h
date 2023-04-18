#ifndef RELATION_H
#define RELATION_H

//#include "eloquent/EloquentBuilder.h"
#include <QObject>

class Model;
class EloquentBuilder;
class RelationPrivate;

// TODO: derive EloquentBuilder for call where/find/first ...
class Relation //: public EloquentBuilder
{
    Q_DECLARE_PRIVATE(Relation)
public:
    Relation(); // TODO: remove this
    Relation(EloquentBuilder *builder, Model *parent);
    Relation(Model *related, Model *parent);
    Relation(const Relation &other);
    Relation(const Relation &&other);
    virtual ~Relation();

    Relation &operator=(const Relation &other);
    bool operator==(const Relation &other);

    virtual void createConstraints();
    virtual QVariant results() const;

    // returns a QObject Collection
    virtual void get() const;

    virtual void touch();

protected:
    Relation(RelationPrivate &dd, EloquentBuilder *builder, Model *parent);
    Relation(RelationPrivate &dd, Model *related, Model *parent);
    QScopedPointer<RelationPrivate> d_ptr;
};

#endif // RELATION_H
