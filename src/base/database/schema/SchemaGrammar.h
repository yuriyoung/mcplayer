#ifndef SCHEMAGRAMMAR_H
#define SCHEMAGRAMMAR_H

#include "Grammar.h"
#include "Blueprint.h"
#include <QObject>

namespace Schema {
    enum Type
    {
        Char = 0,
        String,
        Text,
        MediumText,
        LongText,

        Integer,
        SmallInteger,
        TinyInteger,
        MediumInteger,
        BigInteger,

        Float,
        Double,
        Decimal,
        Boolean,
        Enum,
        Set, // `set("{allowed}")`

        Json,
        Jsonb,

        Year,
        Time,
        Date,
        DataTime,
        TimeTz,
        DateTimeTz,
        Timestamp,
        TimestampTz,

        Binary,
        Uuid,
        IpAddress,
        MacAddress,

        Point,
        LineString,
        Geometry,
        Polygon,
        GeometryCollection,
        MultiPoint,
        MultiLineString,
        MultiPolygon,

        UserType = 0x0100
    };
}

class ColumnDefinition;
class SchemaGrammarPrivate;

// TODO: move to namespace Schema
// sub class from QObject for using Qt Meta Object system to invok type*() methods
class SchemaGrammar : public Grammar
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SchemaGrammar)
public:
    explicit SchemaGrammar(QObject *parent = nullptr);
    ~SchemaGrammar();

    virtual QStringList compile(void *data);

    // wrap column definition(type, length...)
    QStringList wrapColumns(Blueprint *blueprint);

    QString getType(const ColumnDefinition &column);

    virtual QHash<int, QByteArray> typeNames() const;

/**
 * slots: set to invokable or using Q_INVOKABLE
 */
protected slots:
    virtual QString typeChar(const ColumnDefinition &column) const;
    virtual QString typeString(const ColumnDefinition &column) const;
    virtual QString typeText(const ColumnDefinition &column) const;
    virtual QString typeMediumText(const ColumnDefinition &column) const;
    virtual QString typeLongText(const ColumnDefinition &column) const;

    virtual QString typeInteger(const ColumnDefinition &column) const;
    virtual QString typeSmallInteger(const ColumnDefinition &column) const;
    virtual QString typeTinyInteger(const ColumnDefinition &column) const;
    virtual QString typeMediumInteger(const ColumnDefinition &column) const;
    virtual QString typeBigInteger(const ColumnDefinition &column) const;

    virtual QString typeFloat(const ColumnDefinition &column) const;
    virtual QString typeDouble(const ColumnDefinition &column) const;
    virtual QString typeDecimal(const ColumnDefinition &column) const;
    virtual QString typeBoolean(const ColumnDefinition &column) const;
    virtual QString typeEnum(const ColumnDefinition &column) const;
    virtual QString typeSet(const ColumnDefinition &column) const;

    virtual QString typeJson(const ColumnDefinition &column) const;
    virtual QString typeJsonb(const ColumnDefinition &column) const;

    virtual QString typeYear(const ColumnDefinition &column) const;
    virtual QString typeTime(const ColumnDefinition &column) const;
    virtual QString typeDate(const ColumnDefinition &column) const;
    virtual QString typeDataTime(const ColumnDefinition &column) const;
    virtual QString typeTimeTz(const ColumnDefinition &column) const;
    virtual QString typeDataTimeTz(const ColumnDefinition &column) const;
    virtual QString typeTimestamp(const ColumnDefinition &column) const;
    virtual QString typeTimestampTz(const ColumnDefinition &column) const;

    virtual QString typeBinary(const ColumnDefinition &column) const;
    virtual QString typeUuid(const ColumnDefinition &column) const;
    virtual QString typeIpAddress(const ColumnDefinition &column) const;
    virtual QString typeMacAddress(const ColumnDefinition &column) const;

    virtual QString typePoint(const ColumnDefinition &column) const;
    virtual QString typeLineString(const ColumnDefinition &column) const;
    virtual QString typeGeometry(const ColumnDefinition &column) const;
    virtual QString typePolygon(const ColumnDefinition &column) const;
    virtual QString typeGeometryCollection(const ColumnDefinition &column) const;
    virtual QString typeMultiPoint(const ColumnDefinition &column) const;
    virtual QString typeMultiLineString(const ColumnDefinition &column) const;
    virtual QString typeMultiPolygon(const ColumnDefinition &column) const;


protected:
    virtual QString compileTableExists(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileColumnListing(Blueprint *blueprint, const Command &command) = 0;

    virtual QString compileEnableForeignKeyConstraints(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileDisableForeignKeyConstraints(Blueprint *blueprint, const Command &command) = 0;

    virtual QString compileCreate(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileAdd(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileChange(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileForeign(Blueprint *blueprint, const Command &command) = 0;

    virtual QString compileRename(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileRenameColumn(Blueprint *blueprint, const Command &command);
    virtual QString compileRenameIndex(Blueprint *blueprint, const Command &command) = 0;

    virtual QString compilePrimary(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileUnique(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileIndex(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileSpatialIndex(Blueprint *blueprint, const Command &command) = 0;

    virtual QString compileDrop(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileDropIfExists(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileDropColumn(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileDropPrimary(Blueprint *blueprint, const Command &command);
    virtual QString compileDropUnique(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileDropIndex(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileDropSpatialIndex(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileDropForeign(Blueprint *blueprint, const Command &command);
    virtual QString compileDropAllTables(Blueprint *blueprint, const Command &command) = 0;
    virtual QString compileDropAllViews(...) = 0;
    virtual QString compileRebuild();

    /**
     * @brief to implement apply column modifier for any engines:
     *  SQLite:
     *  - 'nullable', 'default', 'increment'
     *
     *  MySql:
     *  - 'Unsigned', 'Charset', 'Collate', 'VirtualAs', 'StoredAs', 'Nullable',
     *  - 'Default', 'Increment', 'Comment', 'After', 'First', 'Srid',
     *
     *  Postgres:
     *  - 'Collate', 'Increment', 'Nullable', 'Default', 'VirtualAs', 'StoredAs'
     *
     *  SqlServer:
     *  - 'Increment', 'Collate', 'Nullable', 'Default', 'Persisted'
     */
    virtual QString applyModifiers(Blueprint *blueprint, const ColumnDefinition &column) const = 0;
    /*
    virtual QString modifyVirtualAs(Blueprint *blueprint, const ColumnDefinition &column) const;
    virtual QString modifyStoredAs(Blueprint *blueprint, const ColumnDefinition &column) const;
    virtual QString modifyUnsiged(Blueprint *blueprint, const ColumnDefinition &column) const;
    virtual QString modifyCharset(Blueprint *blueprint, const ColumnDefinition &column) const;
    virtual QString modifyCollate(Blueprint *blueprint, const ColumnDefinition &column) const;
    virtual QString modifyFirst(Blueprint *blueprint, const ColumnDefinition &column) const;
    virtual QString modifyAfter(Blueprint *blueprint, const ColumnDefinition &column) const;
    virtual QString modifyComment(Blueprint *blueprint, const ColumnDefinition &column) const;
    virtual QString modifyNullable(Blueprint *blueprint, const ColumnDefinition &column) const = 0;
    virtual QString modifyDefault(Blueprint *blueprint, const ColumnDefinition &column) const = 0;
    virtual QString modifyIncrement(Blueprint *blueprint, const ColumnDefinition &column) const = 0;
    */
protected:
    SchemaGrammar(SchemaGrammarPrivate &dd, QObject *parent = nullptr);
};

#endif // SCHEMAGRAMMAR_H
