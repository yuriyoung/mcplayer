#include "Relation.h"
#include "Relation_p.h"
#include "../Model.h"

RelationPrivate::RelationPrivate(Relation *q) : q_ptr(q)
{

}

RelationPrivate::~RelationPrivate()
{

}

Relation::Relation() {}

Relation::Relation(EloquentBuilder *builder, Model *parent)
    : Relation(*new RelationPrivate(this), builder, parent)
{

}

Relation::Relation(Model *related, Model *parent)
    : Relation(*new RelationPrivate(this), related, parent)
{

}

Relation::Relation(const Relation &other)
{
    d_ptr.reset(other.d_ptr.get());
}

Relation::Relation(const Relation &&other)
{
    d_ptr.reset(other.d_ptr.get());
}

Relation::Relation(RelationPrivate &dd, EloquentBuilder *builder, Model *parent)
    : d_ptr(&dd)
{
    Q_UNUSED(builder)
//    d_ptr->related = builder->model();
    d_ptr->parent = parent;
}

Relation::Relation(RelationPrivate &dd, Model *related, Model *parent)
    : d_ptr(&dd)
{
    d_ptr->related = related;
    d_ptr->parent = parent;
}

Relation::~Relation()
{

}

Relation &Relation::operator=(const Relation &other)
{
    d_ptr.reset(other.d_ptr.get());
    return *this;
}

bool Relation::operator==(const Relation &other)
{
    // TODO: compare primary key and connection

    return d_ptr->related == other.d_ptr->related
            && d_ptr->parent == other.d_ptr->parent;
}

void Relation::createConstraints()
{
    // Set the base constraints on the relation query
}

QVariant Relation::results() const
{
    return QVariant();
}

void Relation::get() const
{

}

void Relation::touch()
{

}
