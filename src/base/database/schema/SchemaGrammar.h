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
        MultiPolygon
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
    ~SchemaGrammar() override;

    virtual QStringList compile(void *data, int type) override;

    // wrap column definition (name varchar(191) not null default ...)
    QStringList wrapColumns(Blueprint *blueprint);

    // get column data type eg: ingeter/char/datetime
    QString columnType(const ColumnDefinition &column);

    // get all lookup names of types supported
    QHash<int, QByteArray> typeNames() const;

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
    virtual QString compileTableExists(const Command &command) = 0;
    virtual QString compileColumnListing(const Command &command) = 0;

    virtual QString compileEnableForeignKeyConstraints(const Command &command) = 0;
    virtual QString compileDisableForeignKeyConstraints(const Command &command) = 0;

    virtual QString compileCreate(const Command &command) = 0;
    virtual QString compileAdd(const Command &command) = 0;
    virtual QString compileChange(const Command &command) = 0;
    virtual QString compileForeign(const Command &command) = 0;

    virtual QString compileRename(const Command &command) = 0;
    virtual QString compileRenameColumn(const Command &command);
    virtual QString compileRenameIndex(const Command &command) = 0;

    virtual QString compilePrimary(const Command &command) = 0;
    virtual QString compileUnique(const Command &command) = 0;
    virtual QString compileIndex(const Command &command) = 0;
    virtual QString compileSpatialIndex(const Command &command) = 0;

    virtual QString compileDrop(const Command &command) = 0;
    virtual QString compileDropIfExists(const Command &command) = 0;
    virtual QString compileDropColumn(const Command &command) = 0;
    virtual QString compileDropPrimary(const Command &command);
    virtual QString compileDropUnique(const Command &command) = 0;
    virtual QString compileDropIndex(const Command &command) = 0;
    virtual QString compileDropSpatialIndex(const Command &command) = 0;
    virtual QString compileDropForeign(const Command &command);
    virtual QString compileDropAllTables(const Command &command) = 0;
    virtual QString compileDropAllViews(...);
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
    virtual QString applyModifiers(const ColumnDefinition &column) const = 0;
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
