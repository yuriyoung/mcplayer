#include "SchemaGrammar.h"
#include "SchemaGrammar_p.h"
#include "Blueprint.h"
#include "ColumnDefinition.h"

#include <QMetaMethod>
#include <QMetaObject>
#include <QDebug>

namespace  {
    struct DefaultTypeNames : public QHash<int, QByteArray>
    {
        DefaultTypeNames()
        {
            (*this)[Schema::Char] = "Char";
            (*this)[Schema::String] = "String";
            (*this)[Schema::Text] = "Text";
            (*this)[Schema::MediumText] = "MediumText";
            (*this)[Schema::LongText] = "LongText";

            (*this)[Schema::Integer] = "Integer";
            (*this)[Schema::TinyInteger] = "TinyInteger";
            (*this)[Schema::MediumInteger] = "MediumInteger";
            (*this)[Schema::BigInteger] = "BigInteger";

            (*this)[Schema::Float] = "Float";
            (*this)[Schema::Double] = "Double";
            (*this)[Schema::Decimal] = "Decimal";
            (*this)[Schema::Boolean] = "Boolean";
            (*this)[Schema::Enum] = "Enum";
            (*this)[Schema::Set] = "Set";

            (*this)[Schema::Json] = "Json";
            (*this)[Schema::Jsonb] = "Jsonb";

            (*this)[Schema::Year] = "Year";
            (*this)[Schema::Time] = "Time";
            (*this)[Schema::Date] = "Date";
            (*this)[Schema::DataTime] = "DataTime";
            (*this)[Schema::TimeTz] = "TimeTz";
            (*this)[Schema::DateTimeTz] = "DataTimeTz";
            (*this)[Schema::Timestamp] = "Timestamp";
            (*this)[Schema::TimestampTz] = "TimestampTz";

            (*this)[Schema::Binary] = "Binary";
            (*this)[Schema::Uuid] = "Uuid";
            (*this)[Schema::IpAddress] = "IpAddress";
            (*this)[Schema::MacAddress] = "MacAddress";

            (*this)[Schema::Point] = "Point";
            (*this)[Schema::LineString] = "LineString";
            (*this)[Schema::Geometry] = "Geometry";
            (*this)[Schema::Polygon] = "Polygon";
            (*this)[Schema::GeometryCollection] = "GeometryCollection";
            (*this)[Schema::MultiPoint] = "MultiPoint";
            (*this)[Schema::MultiLineString] = "MultiLineString";
            (*this)[Schema::MultiPolygon] = "MultiPolygon";
        }
    };
}

Q_GLOBAL_STATIC(DefaultTypeNames, g_defaultTypeNames)

SchemaGrammarPrivate::SchemaGrammarPrivate(Grammar *q)
    : GrammarPrivate(q), typeNames(defaultTypeNames())
{

}

const QHash<int, QByteArray> &SchemaGrammarPrivate::defaultTypeNames()
{
    return *g_defaultTypeNames();
}

/**
 * @brief SchemaGrammar::SchemaGrammar
 */
SchemaGrammar::SchemaGrammar(QObject *parent)
    : Grammar(*new SchemaGrammarPrivate(this), parent)
{

}

SchemaGrammar::SchemaGrammar(SchemaGrammarPrivate &dd, QObject *parent)
    : Grammar(dd, parent)
{

}

SchemaGrammar::~SchemaGrammar()
{

}

QStringList SchemaGrammar::compile(void *data)
{
    qDebug() << "SchemaGrammar::compile()";

    QStringList statements;

    Blueprint *blueprint = static_cast<Blueprint *>(data);
    if(!blueprint)
    {
        qWarning() << "parameter data can not coverte to Blueprint";
        return statements;
    }

    foreach(const Command &command, blueprint->commands())
    {
        if(!command.isValid())
            continue;

        switch (command.type())
        {
        case Command::Primary: statements << compilePrimary(blueprint, command) ;break;
        case Command::Unique: statements << compileUnique(blueprint, command); break;
        case Command::Index: statements << compileIndex(blueprint, command) ;break;
        case Command::SpatialIndex: statements << compileSpatialIndex(blueprint, command) ;break;

        case Command::Add: statements << compileAdd(blueprint, command); break;
        case Command::Change: statements << compileChange(blueprint, command); break;
        case Command::Create: statements << compileCreate(blueprint, command); break;
        case Command::Foreign: statements << compileForeign(blueprint, command) ;break;

        case Command::Rename: statements << compileRename(blueprint, command); break;
        case Command::RenameIndex: statements << compileRenameIndex(blueprint, command); break;
        case Command::RenameColumn: statements << compileRenameColumn(blueprint, command); break;

        case Command::Drop: statements << compileDrop(blueprint, command); break;
        case Command::DropIfExists: statements << compileDropIfExists(blueprint, command); break;
        case Command::DropColumn: statements << compileDropColumn(blueprint, command); break;
        case Command::DropPrimary: statements << compileDropPrimary(blueprint, command); break;
        case Command::DropUnique: statements <<  compileDropUnique(blueprint, command); break;
        case Command::DropIndex: statements <<  compileDropIndex(blueprint, command); break;
        case Command::DropSpatialIndex: statements << compileDropSpatialIndex(blueprint, command); break;
        case Command::DropForeign: statements << compileDropForeign(blueprint, command); break;
        default: break;
        }
    }

    // TODO: erase emtpy item ?
    return statements;
}

QStringList SchemaGrammar::wrapColumns(Blueprint *blueprint)
{
    QStringList columns;
    foreach(auto &column, blueprint->addedColumns())
    {
        QString colName = column[ColumnDefinition::Name].toString();
        QString sql = wrap(colName) + " " + getType(column);

        // add column modifier
        sql += this->applyModifiers(blueprint, column);

        columns << sql;
    }

    return columns;
}

QString SchemaGrammar::getType(const ColumnDefinition &column)
{
    Q_D(SchemaGrammar);
    Schema::Type t = Schema::Type(column.value(ColumnDefinition::Type).toInt());
    QString type = d->typeNames[t];
    QString typeMethod = "type" + type.replace(0, 1, type[0].toUpper());
    QByteArray text = typeMethod.toLocal8Bit();
    const char *methed = text.constData();

    QString retVal;
    QMetaObject::invokeMethod(this,
        methed,
        Qt::DirectConnection,
        Q_RETURN_ARG(QString, retVal),
        Q_ARG(ColumnDefinition, column)
    );

    return retVal;
}

QHash<int, QByteArray> SchemaGrammar::typeNames() const
{
    Q_D(const SchemaGrammar);
    return d->typeNames;
}

QString SchemaGrammar::typeChar(const ColumnDefinition &column) const
{
    return QString("char(%1)").arg(column.length());
}

QString SchemaGrammar::typeString(const ColumnDefinition &column) const
{
    return QString("varchar(%1)").arg(column.length());
}

QString SchemaGrammar::typeText(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "text";
}

QString SchemaGrammar::typeMediumText(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "mediumtext";
}

QString SchemaGrammar::typeLongText(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "longtext";
}


QString SchemaGrammar::typeInteger(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "int";
}

QString SchemaGrammar::typeSmallInteger(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "smallint";
}

QString SchemaGrammar::typeTinyInteger(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "tinyint";
}

QString SchemaGrammar::typeMediumInteger(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "mediumint";
}

QString SchemaGrammar::typeBigInteger(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "bigint";
}

QString SchemaGrammar::typeFloat(const ColumnDefinition &column) const
{
    return typeDouble(column);
}

QString SchemaGrammar::typeDouble(const ColumnDefinition &column) const
{
    if (column.total() && column.places())
    {
        return QString("double(%1, %2)").arg(column.total()).arg(column.places());
    }

    return "double";
}

QString SchemaGrammar::typeDecimal(const ColumnDefinition &column) const
{
    return QString("decimal(%1, %2)").arg(column.total()).arg(column.places());
}

QString SchemaGrammar::typeBoolean(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "tinyint(1)";
}

QString SchemaGrammar::typeEnum(const ColumnDefinition &column) const
{
    return QString("enum(%1)").arg(quoteString(column.allowed()));
}

QString SchemaGrammar::typeSet(const ColumnDefinition &column) const
{
    return QString("set(%1)").arg(quoteString(column.allowed()));
}

QString SchemaGrammar::typeJson(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "json";
}

QString SchemaGrammar::typeJsonb(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "json";
}

QString SchemaGrammar::typeYear(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "year";
}

QString SchemaGrammar::typeTime(const ColumnDefinition &column) const
{
    return column.precision() > 0
            ? QString("time(%1)").arg(column.precision())
            : "time";
}

QString SchemaGrammar::typeDate(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "date";
}

QString SchemaGrammar::typeDataTime(const ColumnDefinition &column) const
{
    return column.precision() > 0
            ? QString("datetime(%1)").arg(column.precision())
            : "datetime";
}

QString SchemaGrammar::typeTimeTz(const ColumnDefinition &column) const
{
    return typeTime(column);
}

QString SchemaGrammar::typeDataTimeTz(const ColumnDefinition &column) const
{
    return typeDataTime(column);
}

QString SchemaGrammar::typeTimestamp(const ColumnDefinition &column) const
{
    QString type = "timestamp";
    if (column.precision() > 0)
        type = QString("timestamp(%1)").arg(column.precision());

    return column.isUseCurrent()
            ? QString("%1 default CURRENT_TIMESTAMP").arg(type)
            : type;
}

QString SchemaGrammar::typeTimestampTz(const ColumnDefinition &column) const
{
    return typeTimestamp(column);
}

QString SchemaGrammar::typeBinary(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "blob";
}

QString SchemaGrammar::typeUuid(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "char(36)";
}

QString SchemaGrammar::typeIpAddress(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "varchar(45)";
}

QString SchemaGrammar::typeMacAddress(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "varchar(17))";
}

QString SchemaGrammar::typePoint(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "point";
}

QString SchemaGrammar::typeLineString(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "linestring";
}

QString SchemaGrammar::typeGeometry(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "geometry";
}

QString SchemaGrammar::typePolygon(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "polygon";
}

QString SchemaGrammar::typeGeometryCollection(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "geometrycollection";
}

QString SchemaGrammar::typeMultiPoint(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "multipoint";
}

QString SchemaGrammar::typeMultiLineString(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "multilinestring";
}

QString SchemaGrammar::typeMultiPolygon(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "multipolygon";
}

QString SchemaGrammar::compileRenameColumn(Blueprint *blueprint, const Command &command)
{
    Q_UNUSED(blueprint)
    Q_UNUSED(command)
    // TODO: create a RenameColumn class to rename

    return "";
}

QString SchemaGrammar::compileDropPrimary(Blueprint *blueprint, const Command &command)
{
    Q_UNUSED(blueprint)
    Q_UNUSED(command)
    return "";
}

QString SchemaGrammar::compileDropForeign(Blueprint *blueprint, const Command &command)
{
    Q_UNUSED(blueprint)
    Q_UNUSED(command)
    return "";
}

QString SchemaGrammar::compileRebuild()
{
    return "";
}

