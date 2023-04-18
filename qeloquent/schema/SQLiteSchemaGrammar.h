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
    virtual QString compileTableExists(const Command &command);
    virtual QString compileColumnListing(const Command &command);
    virtual QString compileEnableForeignKeyConstraints(const Command &command);
    virtual QString compileDisableForeignKeyConstraints(const Command &command);

    // Compile a create table command.
    virtual QString compileCreate(const Command &command);
    // Compile alter table commands for adding columns.
    virtual QString compileAdd(const Command &command);
    virtual QString compileChange(const Command &command);
    virtual QString compileForeign(const Command &command);

    virtual QString compileRename(const Command &command);
    virtual QString compileRenameIndex(const Command &command);

    virtual QString compilePrimary(const Command &command);
    virtual QString compileUnique(const Command &command);
    virtual QString compileIndex(const Command &command);
    virtual QString compileSpatialIndex(const Command &command);

    virtual QString compileDrop(const Command &command);
    virtual QString compileDropIfExists(const Command &command);
    virtual QString compileDropColumn(const Command &command); // TODO:remove
    virtual QString compileDropPrimary(const Command &command);
    virtual QString compileDropUnique(const Command &command);
    virtual QString compileDropIndex(const Command &command);
    virtual QString compileDropSpatialIndex(const Command &command); // TODO:remove
    virtual QString compileDropForeign(const Command &command); // TODO:remove
    virtual QString compileDropAllTables(const Command &command);
    virtual QString compileDropAllViews(...);
    virtual QString compileRebuild() { return "vacuum"; }

    virtual QString applyModifiers(const ColumnDefinition &column) const;
};

#endif // SQLITESCHEMAGRAMMAR_H
