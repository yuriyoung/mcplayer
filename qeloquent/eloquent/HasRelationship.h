#ifndef HASRELATIONSHIP_H
#define HASRELATIONSHIP_H


#include "relations/Relation.h"
#include "relations/HasOne.h"
#include "relations/HasMany.h"
#include "relations/HasOneOrMany.h"
#include "relations/BelongsTo.h"
#include "relations/BelongsToMany.h"

#include <QObject>
#include <QSharedPointer>
class Model;
class Connection;
class Relation;

class HasRelationshipPrivate;
class HasRelationship
{
    Q_DECLARE_PRIVATE(HasRelationship)
public:
    explicit HasRelationship(Model *model);
    virtual ~HasRelationship();

    //! Define a one-to-one relationship.
    template<typename T>
    Relation hasOne(const QString &foreignKey = {}, const QString localKey = {}) const
    {
        if(std::is_base_of<Model, T>::value)
        {
            // TODO: input T meta data
//            QMetaObject meta = T::staticMetaObject;
            Model *related = new T(this->model()->parent());
            if(!related->connection())
                related->setConnection(this->model()->connection());
            return this->hasOne(related, foreignKey, localKey);
        }

        return Relation();
    }

    //! Define a one-to-many relationship.
    template<typename T>
    Relation hasMany(const QString &foreignKey = {}, const QString localKey = {}) const
    {
        if(std::is_base_of<Model, T>::value)
        {
            auto related = QSharedPointer<T>::create(model()->parent());
            related->setConnection(this->model()->connection());
            return this->hasMany(related.get(), foreignKey, localKey);
        }

        return Relation();
    }

    //! Define a has-one-through relationship.
    Relation hasOneThrough() const;
    //! Define a has-many-through relationship.
    Relation hasManyThrough() const;

    //! Define an inverse one-to-one or many relationship.
    Relation belongsTo(const QString &foreignKey = {},
                       const QString localKey = {}, const QString &relation = {}) const;
    //! Define a many-to-many relationship.
    Relation belongsToMany() const;

    //! Define a polymorphic one-to-one relationship.
    Relation morphOne() const;
    //! Define a polymorphic, inverse one-to-one or many relationship.
    Relation morphTo() const;
    //! Define a polymorphic one-to-many relationship.
    Relation morphMany() const;
    //!  Define a polymorphic many-to-many relationship.
    Relation morphToMany() const;
    //! Define a polymorphic, inverse many-to-many relationship.
    Relation morphedByMany() const;

private:
    Model *model() const;
    Relation hasOne(Model *related, const QString &foreignKey = {}, const QString localKey = {}) const;
    Relation hasMany(Model *related, const QString &foreignKey = {}, const QString localKey = {}) const;

private:
    QScopedPointer<HasRelationshipPrivate> d_ptr;
};

#endif // HASRELATIONSHIP_H
