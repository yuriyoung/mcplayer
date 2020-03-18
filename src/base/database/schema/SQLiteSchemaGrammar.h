#ifndef SQLITESCHEMAGRAMMAR_H
#define SQLITESCHEMAGRAMMAR_H

#include "SchemaGrammar.h"

class SQLiteSchemaGrammarPrivate;
class SQLiteSchemaGrammar : public SchemaGrammar
{
    Q_DECLARE_PRIVATE(SQLiteSchemaGrammar)
public:
    explicit SQLiteSchemaGrammar(QObject *parent = nullptr);

protected slots:
    virtual QString typeChar(const ColumnDefinition &column) const;
    virtual QString typeString(const ColumnDefinition &column) const;
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

    virtual QString typeEnum(const ColumnDefinition &column) const;
    virtual QString typeJson(const ColumnDefinition &column) const;
    virtual QString typeJsonb(const ColumnDefinition &column) const;

    virtual QString typeTime(const ColumnDefinition &column) const;
    virtual QString typeTimestamp(const ColumnDefinition &column) const;

    virtual QString typeUuid(const ColumnDefinition &column) const;
    virtual QString typeIpAddress(const ColumnDefinition &column) const;
    virtual QString typeMacAddress(const ColumnDefinition &column) const;

protected:
    virtual QString compileTableExists(Blueprint *blueprint, const Command &command);
    virtual QString compileColumnListing(Blueprint *blueprint, const Command &command);
    virtual QString compileEnableForeignKeyConstraints(Blueprint *blueprint, const Command &command);
    virtual QString compileDisableForeignKeyConstraints(Blueprint *blueprint, const Command &command);

    // Compile a create table command.
    virtual QString compileCreate(Blueprint *blueprint, const Command &command);
    // Compile alter table commands for adding columns.
    virtual QString compileAdd(Blueprint *blueprint, const Command &command);
    virtual QString compileChange(Blueprint *blueprint, const Command &command);
    virtual QString compileForeign(Blueprint *blueprint, const Command &command);

    virtual QString compileRename(Blueprint *blueprint, const Command &command);
    virtual QString compileRenameIndex(Blueprint *blueprint, const Command &command);

    virtual QString compilePrimary(Blueprint *blueprint, const Command &command);
    virtual QString compileUnique(Blueprint *blueprint, const Command &command);
    virtual QString compileIndex(Blueprint *blueprint, const Command &command);
    virtual QString compileSpatialIndex(Blueprint *blueprint, const Command &command);

    virtual QString compileDrop(Blueprint *blueprint, const Command &command);
    virtual QString compileDropIfExists(Blueprint *blueprint, const Command &command);
    virtual QString compileDropColumn(Blueprint *blueprint, const Command &command); // TODO:remove
    virtual QString compileDropPrimary(Blueprint *blueprint, const Command &command);
    virtual QString compileDropUnique(Blueprint *blueprint, const Command &command);
    virtual QString compileDropIndex(Blueprint *blueprint, const Command &command);
    virtual QString compileDropSpatialIndex(Blueprint *blueprint, const Command &command); // TODO:remove
    virtual QString compileDropForeign(Blueprint *blueprint, const Command &command); // TODO:remove
    virtual QString compileDropAllTables(Blueprint *blueprint, const Command &command);
    virtual QString compileDropAllViews(...);
    virtual QString compileRebuild() { return "vacuum"; }

    virtual QString applyModifiers(Blueprint *blueprint, const ColumnDefinition &column) const;
};

#endif // SQLITESCHEMAGRAMMAR_H
