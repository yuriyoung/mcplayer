#include "SchemaGrammar.h"
#include "SchemaGrammar_p.h"
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
    : GrammarPrivate(q)
    , typeNames(defaultTypeNames())
{

}

QString SchemaGrammarPrivate::compileCommand(const Command &command)
{
    Q_Q(SchemaGrammar);
    if(!command.isValid())
        return QString();

    QString name = command.name().trimmed();
    if(name.isEmpty())
        return QString();

    QString methodName = QString("compile") + name.replace(0, 1, name[0].toUpper());
    QByteArray text = methodName.toLocal8Bit();
    const char *method = text.constData();

    QString retVal;
    q->metaObject()->invokeMethod(
        q,
        method,
        Qt::DirectConnection,
        Q_RETURN_ARG(QString, retVal),
        Q_ARG(Command, command)
    );

    return retVal;
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
    qDebug() << "SchemaGrammar::~SchemaGrammar()";
}

QStringList SchemaGrammar::compile(void *data, int type)
{
    Q_D(SchemaGrammar);
    Q_UNUSED(type)

    Blueprint *blueprint =  static_cast<Blueprint *>(data);
    if(!blueprint)
        return QStringList();

    QList<Command> commands = blueprint->allCommands();
    QStringList statements;
    foreach(auto &cmd, commands)
    {
        QString data = d->compileCommand(cmd);
        if(!data.isEmpty())
            statements << data;
    }

    return statements;
}

QStringList SchemaGrammar::wrapColumns(Blueprint *blueprint)
{
    QStringList results;
    QList<ColumnDefinition> columns = blueprint->creatingColumns();
    foreach(auto &column, columns)
    {
        QString colName = column[ColumnDefinition::Name].toString();
        QString sql = this->wrap(colName) + " " + this->columnType(column);

        // apply column modifier
        sql += this->applyModifiers(column);

        results << sql;
    }

    return results;
}

QString SchemaGrammar::columnType(const ColumnDefinition &column)
{
    Q_D(SchemaGrammar);
    Schema::Type t = Schema::Type(column.value(ColumnDefinition::Type).toInt());
    QString type = d->typeNames[t];
    QString typeMethod = "type" + type.replace(0, 1, type[0].toUpper());
    QByteArray text = typeMethod.toLocal8Bit();
    const char *method = text.constData();

    QString retVal;
    QMetaObject::invokeMethod(this,
        method,
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

QString SchemaGrammar::compileRenameColumn(const Command &command)
{
    Q_UNUSED(command)

    // TODO: create a RenameColumn class to rename

    return "";
}

QString SchemaGrammar::compileDropPrimary(const Command &command)
{
    Q_UNUSED(command)
    return "";
}

QString SchemaGrammar::compileDropForeign(const Command &command)
{
    Q_UNUSED(command)
    return "";
}

QString SchemaGrammar::compileDropAllViews(...)
{
    return "";
}

QString SchemaGrammar::compileRebuild()
{
    return "";
}

