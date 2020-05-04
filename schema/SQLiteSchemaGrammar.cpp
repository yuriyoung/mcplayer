#include "SQLiteSchemaGrammar.h"
#include "SchemaGrammar_p.h"
#include "ColumnDefinition.h"

#include <QDebug>

class SQLiteSchemaGrammarPrivate : public SchemaGrammarPrivate
{
    Q_DECLARE_PUBLIC(SQLiteSchemaGrammar)
public:
    SQLiteSchemaGrammarPrivate(Grammar *q) : SchemaGrammarPrivate(q) { }

    QString getForeignKeys(Blueprint *blueprint)
    {
        QList<Command> commands = blueprint->commands(Command::Foreign);
        QStringList foreignsSql;
        foreach (auto &foreign, commands)
        {
            QString sql = generateForeignKey(foreign);
            if(foreign["onDelete"].toBool())
                sql += " on delete " + foreign["onDelete"].toString();

            if(foreign["onUpdate"].toBool())
                sql += " on update " + foreign["onUpdate"].toString();

            foreignsSql << sql;
        }

        return foreignsSql.join("");
    }

    QString getPrimaryKey(Blueprint *blueprint)
    {
        Q_Q(SQLiteSchemaGrammar);
        QList<Command> commands = blueprint->commands(Command::Primary);
        if(commands.isEmpty())
            return QString();

        Command primary = commands.last();
        if(!primary.isValid())
            return QString();

        return QString(", primary key (%1)").arg(q->columnize(primary.columns()));
    }

private:
    QString generateForeignKey(const Command &foreign)
    {
        Q_Q(SQLiteSchemaGrammar);
        QString cols = q->columnize(foreign.columns());
        QString onTable = q->wrapTable(foreign["on"].toString());
        QString refernces = q->columnize(foreign["references"].toStringList());

        return QString(", foreign key(%1) references %2(%3)")
                .arg(cols).arg(onTable).arg(refernces);
    }
};

SQLiteSchemaGrammar::SQLiteSchemaGrammar(QObject *parent)
    : SchemaGrammar(*new SQLiteSchemaGrammarPrivate(this), parent)
{

}

QString SQLiteSchemaGrammar::typeChar(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "varchar";
}

QString SQLiteSchemaGrammar::typeString(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "varchar";
}

QString SQLiteSchemaGrammar::typeMediumText(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "text";
}

QString SQLiteSchemaGrammar::typeLongText(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "text";
}

QString SQLiteSchemaGrammar::typeInteger(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "integer";
}

QString SQLiteSchemaGrammar::typeSmallInteger(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "integer";
}

QString SQLiteSchemaGrammar::typeTinyInteger(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "integer";
}

QString SQLiteSchemaGrammar::typeMediumInteger(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "integer";
}

QString SQLiteSchemaGrammar::typeBigInteger(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "integer";
}

QString SQLiteSchemaGrammar::typeFloat(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "float";
}

QString SQLiteSchemaGrammar::typeDouble(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "float";
}

QString SQLiteSchemaGrammar::typeDecimal(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "numeric";
}

QString SQLiteSchemaGrammar::typeEnum(const ColumnDefinition &column) const
{
    QString name = column.value(ColumnDefinition::Name).toString();
    QVariant allowedValue = column.value(ColumnDefinition::Allowed);
    QStringList allowed;
    if(allowedValue.canConvert(QVariant::StringList))
    {
        allowed = allowedValue.toStringList();
    }
    else
    {
        allowed << allowedValue.toString();
    }

    return QString("varchar check (%1 in (%2))").arg(name).arg(quoteString(allowed));
}

QString SQLiteSchemaGrammar::typeJson(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "text";
}

QString SQLiteSchemaGrammar::typeJsonb(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "text";
}

QString SQLiteSchemaGrammar::typeTime(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "time";
}

QString SQLiteSchemaGrammar::typeTimestamp(const ColumnDefinition &column) const
{
    return column.value(ColumnDefinition::UseCurrent).toBool()
            ? "datetime default CURRENT_TIMESTAMP"
            : "datetime";
}

QString SQLiteSchemaGrammar::typeUuid(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "varchar";
}

QString SQLiteSchemaGrammar::typeIpAddress(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "varchar";
}

QString SQLiteSchemaGrammar::typeMacAddress(const ColumnDefinition &column) const
{
    Q_UNUSED(column)
    return "varchar";
}

QString SQLiteSchemaGrammar::compileTableExists(const Command &command)
{
    Q_UNUSED(command)
    return "select * from sqlite_master where type = 'table' and name = ?";
}

QString SQLiteSchemaGrammar::compileColumnListing(const Command &command)
{
    QString table = command.blueprint()->table().replace(".", "__");

    return QString("pragma table_info(%1)").arg(wrapTable(table));
}

QString SQLiteSchemaGrammar::compileEnableForeignKeyConstraints(const Command &command)
{
    Q_UNUSED(command)
    return "PRAGMA foreign_keys = ON;";
}

QString SQLiteSchemaGrammar::compileDisableForeignKeyConstraints(const Command &command)
{
    Q_UNUSED(command)
    return "PRAGMA foreign_keys = OFF;";
}

QString SQLiteSchemaGrammar::compileCreate(const Command &command)
{
    Q_D(SQLiteSchemaGrammar);

    qDebug() << "SQLiteSchemaGrammar::compileCreate";

    Blueprint *blueprint = command.blueprint();
    QString create = blueprint->isTemporary() ? "create temporary" : "create";
    QString table = wrapTable(blueprint->table());
    QString columns = wrapColumns(blueprint).join(",");
    QString foreignKeys = d->getForeignKeys(blueprint);
    QString primaryKey = d->getPrimaryKey(blueprint);

    return QString("%1 table %2 (%3%4%5)")
            .arg(create).arg(table).arg(columns).arg(foreignKeys).arg(primaryKey);
}

QString SQLiteSchemaGrammar::compileAdd(const Command &command)
{
    QStringList statements;
    QStringList columns = wrapColumns(command.blueprint());
    foreach(auto &column, columns)
    {
        QString prifixCol = "add column " + column;
        statements << QString("alter table %1 %2")
                      .arg(wrapTable(command.blueprint()->table()))
                      .arg(prifixCol);
    }
    // TODO: SQLite not support add multiple columns.
    return statements.join(";");
}

QString SQLiteSchemaGrammar::compileChange(const Command &command)
{
    Q_UNUSED(command)
    return "";
}

QString SQLiteSchemaGrammar::compileForeign(const Command &command)
{
    Q_UNUSED(command)
    // handled on table creation...
    return "";
}

QString SQLiteSchemaGrammar::compileRename(const Command &command)
{
    Q_UNUSED(command)
    if(!command["to"].toBool())
    {
        qWarning() << "The destination table name is invalid:" << command["to"];
        return "";
    }

    QString from = wrapTable(command.blueprint()->table());
    QString to = command["to"].toString();
    return "alter table " + from + " rename to " + wrapTable(to);
}

QString SQLiteSchemaGrammar::compileRenameIndex(const Command &command)
{
    Q_UNUSED(command)

    // TODO: compile rename index

    return "";
}

QString SQLiteSchemaGrammar::compilePrimary(const Command &command)
{
    Q_UNUSED(command)
    // handled on table creation...
    return "";
}

QString SQLiteSchemaGrammar::compileUnique(const Command &command)
{
    return QString("create unique index %1 on %2 (%3)")
            .arg(wrap(command.indexName()))
            .arg(wrapTable(command.blueprint()->table()))
            .arg(columnize(command.columns()));
}

QString SQLiteSchemaGrammar::compileIndex(const Command &command)
{
    return QString("create index %1 on %2 (%3)")
            .arg(wrap(command.indexName()))
            .arg(wrapTable(command.blueprint()->table()))
            .arg(columnize(command.columns()));
}

QString SQLiteSchemaGrammar::compileSpatialIndex(const Command &command)
{
    Q_UNUSED(command)
    // TODO: throw a database runtime exception
    qCritical() << "The database driver in use does not support spatial indexes.";
    return "";
}

QString SQLiteSchemaGrammar::compileDrop(const Command &command)
{
    Q_UNUSED(command)
    return "drop table " + wrapTable(command.blueprint()->table());
}

QString SQLiteSchemaGrammar::compileDropIfExists(const Command &command)
{
    Q_UNUSED(command)
    return "drop table if exists " + wrapTable(command.blueprint()->table());
}

QString SQLiteSchemaGrammar::compileDropColumn(const Command &command)
{
    Q_UNUSED(command)

    // TODO: compileDropColumn

    return "";
}

QString SQLiteSchemaGrammar::compileDropPrimary(const Command &command)
{
    Q_UNUSED(command)
    // unsupport
    return "";
}

QString SQLiteSchemaGrammar::compileDropUnique(const Command &command)
{
    QString index = wrap(command.indexName());
    return "drop index " + index;
}

QString SQLiteSchemaGrammar::compileDropIndex(const Command &command)
{
    QString index = wrap(command.indexName());
    return "drop index " + index;
}

QString SQLiteSchemaGrammar::compileDropSpatialIndex(const Command &command)
{
    Q_UNUSED(command)
    // TODO: throw a database runtime exception.
    qCritical() << "The database driver in use does not support spatial indexes.";
    return "";
}

QString SQLiteSchemaGrammar::compileDropForeign(const Command &command)
{
    Q_UNUSED(command)
    // unsupport
    // TODO: throw a exception
    qCritical() << "SQLite doesn't support dropping foreign keys (you would need to re-create the table).";
    return "";
}

QString SQLiteSchemaGrammar::compileDropAllTables(const Command &command)
{
    Q_UNUSED(command)
    return "delete from sqlite_master where type in ('table', 'index', 'trigger')";
}

QString SQLiteSchemaGrammar::compileDropAllViews(...)
{
    return "delete from sqlite_master where type in ('view')";
}

QString SQLiteSchemaGrammar::applyModifiers(const ColumnDefinition &column) const
{
    QStringList modifiers;

    // nullable modifier
    modifiers << (column.value(ColumnDefinition::Nullable).toBool() ? " null" : " not null");

    // default value modifier
    // TODO: value is Expresson
    QVariant value = column.value(ColumnDefinition::DefaultValue);
    modifiers << (value.toBool() ? " default " + quoteString(value.toString()) : "");


    // auto increment modifier
    modifiers << (column.value(ColumnDefinition::AutoIncrement).toBool() ? " primary key autoincrement" : "");

    return modifiers.join("");
}
