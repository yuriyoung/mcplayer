#include "ColumnDefinition.h"

#include <QVariantHash>

class ColumnDefinitionPrivate
{
public:
    ColumnDefinitionPrivate()
        : ref(1), attributes(QHash<int, QVariant>())
    {

    }

    ColumnDefinitionPrivate(const ColumnDefinitionPrivate &other)
        : ref(other.ref), attributes(other.attributes)
    {

    }

    bool operator==(const ColumnDefinitionPrivate& other) const
    {
        return attributes == other.attributes;
    }

    QAtomicInt ref;
//    QVariantHash attributes;
    QHash<int, QVariant> attributes;
};

ColumnDefinition::ColumnDefinition()
    : d(new ColumnDefinitionPrivate)
{

}

ColumnDefinition::ColumnDefinition(const ColumnDefinition &other)
    : d(other.d)
{
    d->ref.ref();
}

ColumnDefinition::~ColumnDefinition()
{
    if(!d->ref.deref())
        delete d;
}

ColumnDefinition &ColumnDefinition::operator=(const ColumnDefinition &other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

bool ColumnDefinition::operator==(const ColumnDefinition &other) const
{
    return (d == other.d || *d == *other.d);
}

QVariant &ColumnDefinition::operator[](int key)
{
    return d->attributes[key];
}

const QVariant ColumnDefinition::operator[](int key) const
{
    return d->attributes[key];
}

QHash<int, QVariant> &ColumnDefinition::attributes() const
{
    return d->attributes;
}

QVariant ColumnDefinition::value(ColumnDefinition::AttributeKey key, const QVariant &defaultValue) const
{
    return d->attributes.value(key, defaultValue);
}

void ColumnDefinition::setValue(ColumnDefinition::AttributeKey key, const QVariant &value)
{
    d->attributes.insert(key, value);
}

ColumnDefinition &ColumnDefinition::after(const QString &column)
{
    d->attributes[After] = column;
    return *this;
}

ColumnDefinition &ColumnDefinition::autoIncrement()
{
    QStringList list{"bigInteger", "integer", "mediumInteger", "smallInteger", "tinyInteger"};
    if(list.contains(d->attributes[Type].toString(), Qt::CaseInsensitive))
        d->attributes[AutoIncrement] = true;

    return *this;
}

ColumnDefinition &ColumnDefinition::change()
{
    d->attributes[Change] = true;
    return *this;
}

ColumnDefinition &ColumnDefinition::charset(const QString &charset)
{
    d->attributes[Charset] = charset;
    return *this;
}

ColumnDefinition &ColumnDefinition::comment(const QString &comment)
{
    d->attributes[Comment] = comment;
    return *this;
}

ColumnDefinition &ColumnDefinition::defaultValue(const QVariant &value)
{
    d->attributes[DefaultValue] = value;
    return *this;
}

ColumnDefinition &ColumnDefinition::first()
{
    d->attributes[First] = true;
    return *this;
}

ColumnDefinition &ColumnDefinition::index(const QString &name)
{
    d->attributes[Index] = name;
    return *this;
}

ColumnDefinition &ColumnDefinition::nullable(bool value)
{
    d->attributes[Nullable] = value;
    return *this;
}

ColumnDefinition &ColumnDefinition::primary()
{
    d->attributes[Primary] = true;
    return *this;
}

ColumnDefinition &ColumnDefinition::unique()
{
    d->attributes[Unique] = true;
    return *this;
}

ColumnDefinition &ColumnDefinition::useCurrent()
{
    d->attributes[UseCurrent] = true;
    return *this;
}

int ColumnDefinition::length() const
{
    return d->attributes[Length].toInt();
}

int ColumnDefinition::total() const
{
    return d->attributes[Total].toInt();
}

int ColumnDefinition::places() const
{
    return d->attributes[Places].toInt();
}

int ColumnDefinition::precision() const
{
    return d->attributes[Precision].toInt();
}

bool ColumnDefinition::isUseCurrent() const
{
    return d->attributes[UseCurrent].toBool();
}

QStringList ColumnDefinition::allowed() const
{
    return d->attributes[Allowed].toStringList();
}

bool ColumnDefinition::isNullable() const
{
    return d->attributes[Nullable].toBool();
}
