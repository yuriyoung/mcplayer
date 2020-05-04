#include "Model.h"
#include "Connection.h"
#include "EloquentBuilder.h"

#include <QMetaProperty>
#include <QMetaMethod>
#include <QJsonDocument>
#include <QDebug>

class ModelPrivate : public QSharedData
{
    Q_DECLARE_PUBLIC(Model)
public:
    ModelPrivate(Model *q) : q_ptr(q), relationship(q) {}

    QJsonObject propertyToJson(const QMetaProperty &property) const
    {
        Q_Q(const Model);
        const QMetaObject *meta = q->metaObject();
        QString slot;
        for(int i = 0; i < meta->methodCount(); ++i)
        {
            slot = meta->method(i).name();
            if(slot.toLower().endsWith(property.name()))
                break;

            slot = "";
        }

        return QJsonObject
        {
            { "index", property.propertyIndex() },
            { "name", property.name() },
            { "type", property.type() },
            { "typeName", property.typeName() },
            { "value", QJsonValue::fromVariant(property.read(q)) },
//            { "defaultValue", "NULL" }, //!
            { "auto", q->primaryKey() == property.name() }, //!
            { "table", q->table() }, //!
            { "signal", QString(property.notifySignal().name())  },
            { "slot", slot },
            { "method", property.name() }, //  TODO: find correct method
            { "readable", property.isReadable() },
            { "writable", property.isWritable() },
            { "resetable", property.isResettable() },
            { "valid", property.isValid() },
            { "revision", property.revision() },
            { "constant", property.isConstant() },
            { "stored", property.isStored() },
            { "class", meta->className() }, //!
            { "superClass", meta->superClass()->className() } //!
        };
    }

    bool tapProperty(Model const *model, std::function<void(const QMetaProperty &property)> method) const
    {
        const QMetaObject *meta = model->metaObject();
         // skip properties in the class's superclasses
        const int offset = meta->propertyOffset();
        const int count = meta->propertyCount();

        for (int i = offset; i < count; ++i)
            method(meta->property(i));

        return true;
    }

    Model *q_ptr = nullptr;
    Connection *connection = nullptr;
    HasRelationship relationship;

    QString table; // The table associated with the model.
    QString primaryKey = "id"; // The primary key for the model.
    bool incrementing = true; // Indicates if the IDs are auto-incrementing.
    QStringList with; // The relations to eager load on every query.
    bool exists = false; // Indicates if the model exists.
    const QString createdAt = "created_at";
    const QString updatedAt = "updated_at";

    bool saved = false;
    bool unguarded = false;
    QStringList fillable = {};
    QVariantMap attributes;
    QVariantMap originalAttributes;
};

Model::Model(QObject *parent)
    : QObject(parent), d_ptr(new ModelPrivate(this))
{

}

Model::Model(const Relation &relation, QObject *parent)
    : QObject(parent), d_ptr(new ModelPrivate(this))
{
    relation.get(); // returns a Collection(QObject)
}

Model::~Model()
{
    qDebug() << "~Model(): free";
}

QString Model::table() const
{
    QString name = this->metaObject()->className();

    // TODO: singular name to plural name
    // ...

    return name.toLower();
}

QString Model::primaryKey() const
{
    return "id";
}

QVariant Model::primaryValue() const
{
    return attribute(this->primaryKey());
}

QString Model::foreignKey() const
{
    QString name = this->metaObject()->className();
    return name.toLower() + "_" + this->primaryKey();
}

bool Model::save()
{
    Q_D(Model);
    if(d->attributes.isEmpty())
        return false;

    // TODO:
    // query->insert(d->attributes);

    return true;
}

bool Model::update(const QVariantMap &attributes)
{
    Q_D(const Model);
    if(d->saved)
        return false;

    return this->fill(attributes)->save();
}

bool Model::del()
{
    if(!this->primaryValue().isValid())
        return false;

    // TODO: query->where(primaryKey, "=", ...);

    return true;
}

QStringList Model::fillable() const
{
    Q_D(const Model);
    return d->fillable;
}

void Model::setFillable(const QStringList &fillable)
{
    Q_D(Model);
    d->fillable = fillable;
}

bool Model::isFillable(const QString &key) const
{
    Q_D(const Model);
    if(d->unguarded)
        return true;

    return this->fillable().contains(key);
}

Model *Model::fill(const QVariantMap &attributes)
{
    QVariantMap::const_iterator it = attributes.constBegin();
    while (it != attributes.constEnd())
    {
        if(this->isFillable(it.key()))
            this->setAttribute(it.key(), it.value());
        ++it;
    }

    return this;
}

Model *Model::foreFill(const QVariantMap &attributes)
{
    Q_D(Model);
    d->unguarded = true;
    this->fill(attributes);
    d->unguarded = false;

    return this;
}

QVariantMap Model::attributes() const
{
    Q_D(const Model);
    return d->attributes;
}

QVariant Model::attribute(const QString &key) const
{
    Q_D(const Model);
    return d->attributes.value(key);
}

void Model::setAttribute(const QString &key, const QVariant &value)
{
    Q_D(Model);
    d->attributes.insert(key, value);
}

QJsonObject Model::attributesToJson() const
{
    return QJsonObject();
}

QHash<QString, QMetaProperty> Model::metaProperty() const
{
    Q_D(const Model);
    QHash<QString, QMetaProperty> properties;
    d->tapProperty(this, [&properties](const QMetaProperty &property)
    {
        properties.insert(property.name(), property);
    });

    return properties;
}

QJsonObject Model::metaPropertyToJson() const
{
    Q_D(const Model);
    QJsonObject result;
    d->tapProperty(this, [&result, d](const QMetaProperty &property)
    {
        QJsonObject json = d->propertyToJson(property);
        result.insert(property.name(), QJsonValue(json));
    });

    return result;
}

Connection *Model::connection() const
{
    Q_D(const Model);
    return d->connection;
}

void Model::setConnection(Connection *connection)
{
    Q_D(Model);
    d->connection = connection;
}

EloquentBuilder Model::newQuery() const
{
    Q_D(const Model);
    return EloquentBuilder(d->connection->queryBuilder(), this);
}

void Model::dump()
{
    QJsonDocument doc(this->metaPropertyToJson());
    qDebug() << doc.toJson(QJsonDocument::Compact);
}
