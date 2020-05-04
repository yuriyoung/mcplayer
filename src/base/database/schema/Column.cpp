#include "Column.h"

class ColumnPrivate
{
public:
    ColumnPrivate(const QString &name, int type, const QString &tableName);
    ColumnPrivate(const ColumnPrivate &other);
    bool operator==(const ColumnPrivate& other) const;

    QAtomicInt ref;
    QString name;
    QString table;
    QVariant defaultValue;

    int type;
    bool required;
    int length;
    int precision;
    bool readOnly: 1;
    bool autoValue: 1;
    bool nullable: 1;
    int total;
    int places;
};

ColumnPrivate::ColumnPrivate(const QString &name, int type, const QString &tableName)
    : ref(1), name(name), table(tableName), defaultValue(QVariant()), type(type),
      required(false), length(-1), precision(-1), readOnly(false), autoValue(false),
      total(-1), places(-1)
{

}

ColumnPrivate::ColumnPrivate(const ColumnPrivate &other)
    : ref(1), name(other.name), table(other.table), defaultValue(other.defaultValue), type(other.type),
      required(other.required), length(other.length), precision(other.precision),
      readOnly(other.readOnly), autoValue(other.autoValue), nullable(other.nullable),
      total(other.total), places(other.places)
{

}

bool ColumnPrivate::operator==(const ColumnPrivate &other) const
{
    return (name == other.name
            && table == other.table
            && defaultValue == other.defaultValue
            && type == other.type
            && required == other.required
            && length == other.length
            && precision == other.precision
            && readOnly == other.readOnly
            && autoValue == other.autoValue
            && nullable == other.nullable
            && total == other.total
            && places == other.places);
}

/**
 * @brief Column::Column
 * @param name
 * @param type
 */
Column::Column(const QString &name, int type)
    : d(new ColumnPrivate(name, type, QString()))
{
    val = QVariant::fromValue(type);
}

Column::Column(const QString &name, int type, const QString &table)
    : d(new ColumnPrivate(name, type, table))
{
    val = QVariant::fromValue(type);
}

Column::Column(const Column &other)
{
    d = other.d;
    d->ref.ref();
    val = other.val;
}

Column::~Column()
{
    if (!d->ref.deref())
        delete d;
}

Column &Column::operator=(const Column &other)
{
    qAtomicAssign(d, other.d);
    val = other.val;
    return *this;
}

bool Column::operator==(const Column &other) const
{
    return ((d == other.d || *d == *other.d) && val == other.val);
}

void Column::setValue(const QVariant &value)
{
    if (isReadOnly())
        return;
    val = value;
}

QVariant Column::value() const
{
    return val;
}

void Column::setName(const QString &name)
{
    qAtomicDetach(d);
    d->name = name;
}

QString Column::name() const
{
    return d->name;
}

void Column::setTableName(const QString &tableName)
{
    qAtomicDetach(d);
    d->table = tableName;
}

QString Column::tableName() const
{
    return d->table;
}

void Column::setType(int type)
{
    qAtomicDetach(d);
    d->type = type;
    if(!val.isValid())
        val = QVariant::fromValue(type);
}

int Column::type() const
{
    return d->type;
}

QString Column::typeName() const
{
    switch (d->type)
    {
        case QMetaType::Bool:           return "BOOLEAN";
        case QMetaType::QBitArray:
        case QMetaType::QByteArray:     return "BLOB";
        case QMetaType::QDate:          return "DATE";
        case QMetaType::QDateTime:      return "DATETIME";
        case QMetaType::QTime:          return "TIME";
        case QMetaType::Double:         return "DOUBLE";
        case QMetaType::Float:          return "FLOAT";

        case QMetaType::SChar:
        case QMetaType::Char:           return "TINYINT";
        case QMetaType::UChar:          return "TINYINT UNSIGNED";
        case QMetaType::Short:          return "SMALLINT";
        case QMetaType::UShort:         return "SMALLINT UNSIGNED";
        case QMetaType::Int:            return "INT";
        case QMetaType::UInt:           return "INT UNSIGNED";
        case QMetaType::Long:           return "MEDIUMINT";
        case QMetaType::ULong:          return "MEDIUMINT UNSIGNED";
        case QMetaType::LongLong:       return "BIGINT";
        case QMetaType::ULongLong:      return "BIGINT UNSIGNED";

        case QMetaType::QChar:          return "NCHAR";

        case QMetaType::QUrl:
        case QMetaType::QJsonArray:
        case QMetaType::QJsonValue:
        case QMetaType::QJsonObject:
        case QMetaType::QJsonDocument:
        case QMetaType::QPoint:
        case QMetaType::QPointF:
        case QMetaType::QSize:
        case QMetaType::QSizeF:
        case QMetaType::QLine:
        case QMetaType::QLineF:
        case QMetaType::QRect:
        case QMetaType::QRectF:
        case QMetaType::QPolygon:
        case QMetaType::QPolygonF:
        case QMetaType::QStringList:
        case QMetaType::QColor:
        case QMetaType::QUuid:          return "TEXT";
        case QMetaType::QString:
            if(d->length > 0)
                return QString("VARCHAR(%1)").arg(d->length);
            else
                return "TEXT";
        default:
            return QString();
        }
}

void Column::setReadOnly(bool readOnly)
{
    qAtomicDetach(d);
    d->readOnly = readOnly;
}

bool Column::isReadOnly() const
{
    return d->readOnly;
}

void Column::setAutoIncreament(bool autoVal)
{
    this->setAutoValue(autoVal);
}

bool Column::isAutoIncreament() const
{
    return this->isAutoValue();
}

void Column::setAutoValue(bool autoVal)
{
    qAtomicDetach(d);

    if(d->type == QMetaType::SChar
        || d->type == QMetaType::Char
        || d->type == QMetaType::UChar
        || d->type == QMetaType::Short
        || d->type == QMetaType::UShort
        || d->type == QMetaType::Int
        || d->type == QMetaType::UInt
        || d->type == QMetaType::Long
        || d->type == QMetaType::ULong
        || d->type == QMetaType::LongLong
        || d->type == QMetaType::ULongLong)
    {
        d->autoValue = autoVal;
    }
}

bool Column::isAutoValue() const
{
    return d->autoValue;
}

void Column::setRequired(bool required)
{
    qAtomicDetach(d);
    d->required = required;
}

bool Column::isRequired() const
{
    return d->required;
}

void Column::setDefaultValue(const QVariant &value)
{
    qAtomicDetach(d);
    d->defaultValue = value;
}

QVariant Column::defaultValue() const
{
    return d->defaultValue;
}

void Column::setLength(int fieldLength)
{
    qAtomicDetach(d);
    d->length = fieldLength;
}

int Column::length() const
{
    return d->length;
}

void Column::setPrecision(int precision)
{
    qAtomicDetach(d);
    d->precision = precision;
}

int Column::precision() const
{
    return d->precision;
}

void Column::setTotal(int total)
{
    qAtomicDetach(d);
    d->total = total;
}

int Column::total() const
{
    return d->total;
}

void Column::setPlaces(int places)
{
    qAtomicDetach(d);
    d->places = places;
}

int Column::places() const
{
    return d->places;
}

void Column::setNullable(bool nullable)
{
    qAtomicDetach(d);
    d->nullable = nullable;
}

bool Column::nullable() const
{
    return d->nullable;
}

void Column::setNull()
{
    if(isReadOnly())
        return;

    val = QVariant::fromValue(type());
}

bool Column::isNull() const
{
    return val.isNull();
}

bool Column::isValid() const
{
    return d->type != QMetaType::UnknownType;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const Column &col)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg << "Column(" << col.name() << ", " << QMetaType::typeName(col.type());
    dbg << ", tableName: " << (col.tableName().isEmpty() ? QStringLiteral("(not specified)") : col.tableName());
    if (col.length() >= 0)
        dbg << ", length: " << col.length();
    if (col.precision() >= 0)
        dbg << ", precision: " << col.precision();
    dbg << ", required: " << (col.isRequired() ? "yes" : "no");
    if (col.type() >= 0)
        dbg << ", type: " << col.type();
    if (!col.defaultValue().isNull())
        dbg << ", defaultValue: \"" << col.defaultValue() << '\"';
    dbg << ", autoValue: " << col.isAutoValue()
        << ", readOnly: " << col.isReadOnly() << ')';
    return dbg;
}
#endif
