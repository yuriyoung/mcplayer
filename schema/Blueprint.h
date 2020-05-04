#ifndef BLUEPRINT_H
#define BLUEPRINT_H

#include "ColumnDefinition.h"
#include "Command.h"

#include <QObject>
#include <QVariant>
#include <QHash>

/**
 * test:
 *  SQLiteSchemaGrammar grammar;
 *  Blueprint blueprint("users", [](Blueprint *blueprint){
 *      blueprint->increments("id").comment("primary key");
 *      blueprint->string("name").unique();
 *      blueprint->string("password");
 *      blueprint->string("gender").nullable();
 *      blueprint->integer("post_id");
 *      blueprint->timestamp("verified_at").useCurrent();
 *      blueprint->timestamps();
 *      blueprint->softDeletes();
 *
 *      blueprint->foreign({"post_id"}).references("id").on("posts").onDelete("cascade").onUpdate("cascade");
 *  });
 *
 *  blueprint.create();
 *  blueprint.build(nullptr, &grammar);
 */

class Grammar;
class Connection ;
class BlueprintPrivate;
class Blueprint
{
    Q_DECLARE_PRIVATE(Blueprint)
    QScopedPointer<BlueprintPrivate> d_ptr;
public:
    enum DataRole
    {
        IsCreatingRole = 0,
        IsTemporaryRole,
        CreateColumnsRole,
        ChangeColumnsRole,
        CommandsRole,
    };

    using Closure = std::function<void(Blueprint*)>;

    explicit Blueprint(const QString &table, Closure callback = nullptr);
    virtual ~Blueprint();

    bool isTemporary() const;
    QList<Command> allCommands() const;
    QList<Command> commands(Command::Type type) const;
    Command command(Command::Type type) const;
    QList<ColumnDefinition> creatingColumns() const;
    QList<ColumnDefinition> columns(ColumnDefinition::AttributeKey key) const;

    void build(const Connection *connection, const  Grammar *grammar);
    QStringList toSql();

    QString table() const;

    bool creating() const;

    // Indicate that the table needs to be created.
    void create();
    // Indicate that the table needs to be temporary.
    void temporary();

    // Indicate that the table should be dropped.
    void drop();
    // Indicate that the table should be dropped if it exists.
    void dropIfExists();
    // Indicate that the given columns should be dropped.
    void dropColumn(const QStringList &columns);
    // Indicate that the given primary key should be dropped.
    void dropPrimary(const QString &index = QString());
    // Indicate that the given unique key should be dropped.
    void dropUnique(const QString &index);
    // Indicate that the given index should be dropped.
    void dropIndex(const QString &index);
    // Indicate that the given foreign key should be dropped.
    void dropForeign(const QString &index);
    // Indicate that the given spatial index should be dropped.
    void dropSpatialIndex(const QString &index);
    // Indicate that the timestamp columns should be dropped.
    void dropTimestamps();
    void dropTimestampsTz();
    // Indicate that the soft delete column should be dropped.
    void dropSoftDeletes();
    void dropSoftDeletesTz();
    // Indicate that the remember token column should be dropped.
    void dropRememberToken();

    // Indicate that the given columns should be renamed.
    void renameColumn(const QString &from, const QString &to);
    // Indicate that the given indexes should be renamed.
    void renameIndex(const QString &from, const QString &to);
    // Rename the table to a given name.
    void rename(const QString &from, const QString &to);

    /*!
      * @brief index commands
      */
    // Specify the primary key(s) for the table.
    Command &primary(const QStringList &columns, const QString &name = "");
    // Specify a unique index for the table.
    Command &unique(const QStringList &columns, const QString &name = "");
    // Specify an index for the table.
    Command &index(const QStringList &columns, const QString &name = "");
    // Specify a spatial index for the table.
    Command &spatialIndex(const QStringList &columns, const QString &name = "");
    /**
     * @brief Specify a foreign key for the table.
     * @param columns
     * @param name index name
     * @param references "user_id" foreign table primary key id
     * @param on "users" table
     * @param action "on delete|update 'cascade'|'restrict'|'set null'|'no action"
     */
    Command &foreign(const QStringList &columns, const QString &name = "");

    /*!
      * @brief column defined
      */
    // Create a new auto-incrementing integer (4-byte) column on the table.
    ColumnDefinition &increments(const QString &column);
    // Create a new auto-incrementing tiny integer (1-byte) column on the table.
    ColumnDefinition &tinyIncrements(const QString &column);
    // Create a new auto-incrementing small integer (2-byte) column on the table.
    ColumnDefinition &smallIncrements(const QString &column);
    // Create a new auto-incrementing medium integer (3-byte) column on the table.
    ColumnDefinition &mediumIncrements(const QString &column);
    // Create a new auto-incrementing big integer (8-byte) column on the table.
    ColumnDefinition &bigIncrements(const QString &column);

    // Create a new char column on the table.
    ColumnDefinition &character(const QString &column, int length = 255);
    // Create a new string column on the table.
    ColumnDefinition &string(const QString &column, int length = 255);
    // Create a new text column on the table.
    ColumnDefinition &text(const QString &column);
    // Create a new medium text column on the table.
    ColumnDefinition &mediumText(const QString &column);
    // Create a new long text column on the table.
    ColumnDefinition &longText(const QString &column);

    // Create a new integer (4-byte) column on the table.
    ColumnDefinition &integer(const QString &column, bool autoIncrement = false, bool isUnsigned = false);
    // Create a new tiny integer (1-byte) column on the table.
    ColumnDefinition &tinyInteger(const QString &column, bool autoIncrement = false, bool isUnsigned = false);
    // Create a new small integer (2-byte) column on the table.
    ColumnDefinition &smallInteger(const QString &column, bool autoIncrement = false, bool isUnsigned = false);
    // Create a new medium integer (3-byte) column on the table.
    ColumnDefinition &mediumInteger(const QString &column, bool autoIncrement = false, bool isUnsigned = false);
    // Create a new big integer (8-byte) column on the table.
    ColumnDefinition &bigInteger(const QString &column, bool autoIncrement = false, bool isUnsigned = false);
    // Create a new unsigned integer (4-byte) column on the table.
    ColumnDefinition &unsignedInteger(const QString &column, bool autoIncrement = false);
    // Create a new unsigned tiny integer (1-byte) column on the table.
    ColumnDefinition &unsignedTinyInteger(const QString &column, bool autoIncrement = false);
    // Create a new unsigned small integer (2-byte) column on the table.
    ColumnDefinition &unsignedSmallInteger(const QString &column, bool autoIncrement = false);
    // Create a new unsigned medium integer (3-byte) column on the table.
    ColumnDefinition &unsignedMediumInteger(const QString &column, bool autoIncrement = false);
    // Create a new unsigned big integer (8-byte) column on the table.
    ColumnDefinition &unsignedBigInteger(const QString &column, bool autoIncrement = false);

    // Create a new float column on the table.
    ColumnDefinition &floatNumeric(const QString &column, int total = 8, int places = 2);
    // Create a new double column on the table.
    ColumnDefinition &doubleNumeric(const QString &column, int total = 0, int places = 0);
    // Create a new decimal column on the table.
    ColumnDefinition &decimal(const QString &column, int total = 8, int places = 2);
    // Create a new unsigned decimal column on the table.
    ColumnDefinition &unsignedDecimal(const QString &column, int total = 8, int places = 2);
    // Create a new boolean column on the table.
    ColumnDefinition &boolean(const QString &column);
    // Create a new enum column on the table.
    ColumnDefinition &enumerate(const QString &column, const QStringList &allowed);

    // Create a new json column on the table.
    ColumnDefinition &json(const QString &column);
    ColumnDefinition &jsonb(const QString &column);

    // Create a new date-time column on the table.
    ColumnDefinition &date(const QString &column);
    ColumnDefinition &dateTime(const QString &column, int precision = 0);
    ColumnDefinition &dateTimeTz(const QString &column, int precision = 0);
    ColumnDefinition &time(const QString &column, int precision = 0);
    ColumnDefinition &timeTz(const QString &column, int precision = 0);
    ColumnDefinition &timestamp(const QString &column, int precision = 0);
    ColumnDefinition &timestampTz(const QString &column, int precision = 0);

    // Add nullable creation and update timestamps to the table.
    // default column name is created_at or updated_at
    void timestamps(int precision = 0);
    // Alias for timestamps().
    void nullableTimestamps(int precision = 0);
    void timestampsTz(int precision = 0);

    // Add a "deleted at" timestamp for the table.
    ColumnDefinition &softDeletes(const QString &column = "deleted_at", int precision = 0);
    ColumnDefinition &softDeletesTz(const QString &column = "deleted_at", int precision = 0);

    // Create a new year column on the table.
    ColumnDefinition &year(const QString &column);
    // Create a new binary column on the table.
    ColumnDefinition &binary(const QString &column);
    // Create a new uuid column on the table.
    ColumnDefinition &uuid(const QString &column);
    // Create a new IP address column on the table.
    ColumnDefinition &ipAddress(const QString &column);
    // Create a new MAC address column on the table.
    ColumnDefinition &macAddress(const QString &column);
    // Create a new geometry column on the table.
    ColumnDefinition &geometry(const QString &column);
    // Create a new point column on the table.
    ColumnDefinition &point(const QString &column, int srid = 0);

    // Create a new linestring column on the table.
    ColumnDefinition &lineString(const QString &column);
    // Create a new polygon column on the table.
    ColumnDefinition &polygon(const QString &column);
    // Create a new geometrycollection column on the table.
    ColumnDefinition &geometryCollection(const QString &column);
    // Create a new multipoint column on the table.
    ColumnDefinition &multiPoint(const QString &column);
    // Create a new multilinestring column on the table.
    ColumnDefinition &multiLineString(const QString &column);
    // Create a new multipolygon column on the table.
    ColumnDefinition &multiPolygon(const QString &column);

    // Add the proper columns for a polymorphic table.
    void morphs(const QString &name, const QString &indexName = QString());
    // Add nullable columns for a polymorphic table.
    void nullableMorphs(const QString &name, const QString &indexName = QString());

    // Adds the `remember_token` column to the table.
    ColumnDefinition &rememberToken();
};

#endif // BLUEPRINT_H
