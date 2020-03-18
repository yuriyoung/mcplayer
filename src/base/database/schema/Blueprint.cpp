#include "Blueprint.h"
#include "Grammar.h"
#include "Database.h"
#include "support/array_helper.h"

#include <QVariant>
#include <QDebug>

using Fluent = QHash<int, QVariant>;

class BlueprintPrivate
{
    Q_DECLARE_PUBLIC(Blueprint)
public:
    BlueprintPrivate(Blueprint *q) : q_ptr(q)
    {
    }

    Command &addCommand(Command::Type type, const QVariantHash &parameters = {}, const QStringList &columns = {})
    {
        Command command(type, columns);
        for(auto it = parameters.begin(); it != parameters.end(); ++it)
        {
            // allow repalced common key with value
            command[it.key()] = it.value();
        }
        this->commands.append(command);
        return commands[commands.size() - 1];
    }

    Command createCommand(Command::Type type, const QVariantHash &parameters = {})
    {
        Command command(type);
        for(auto it = parameters.begin(); it != parameters.end(); ++it)
        {
            // allow repalced common key with value
            command[it.key()] = it.value();
        }

        return command;
    }

    Command &indexCommand(Command::Type type, const QStringList &columns, const QString &index = "")
    {
        QString indexName(index);
        if(indexName.isEmpty())
            indexName = createIndexName(type, columns);

        Command command(type, columns, indexName);
        this->commands.append(command);

        return commands[commands.size() - 1];
    }

    /**
     * @brief dropIndexCommand
     * @param command
     * @param type => primary, index, unique, spatialIndex, foreign
     * @param index
     */
    Command &dropIndexCommand(Command::Type command, const QStringList &columns)
    {
        return indexCommand(command, columns);
    }

    Command &dropIndexCommand(Command::Type command, const QString &index)
    {
        return indexCommand(command, {}, index);
    }

    /**
     * @brief createIndexName
     * @param type => primary, index, unique, spatialIndex, foreign
     * @param columns
     * @return
     */
    QString createIndexName(const QString &type, const QStringList &columns) const
    {
        QString index = table + "_" + columns.join("_") + "_" + type;
        index = index.toLower();
        return index.replace(QRegExp("-|\\."), "_");
    }

    QString createIndexName(Command::Type type, const QStringList &columns) const
    {
        QString indexType;
        switch (type)
        {
        case Command::Primary:      indexType = "primary";        break;
        case Command::Unique:       indexType = "unique";         break;
        case Command::Index:        indexType = "index";          break;
        case Command::SpatialIndex: indexType = "spatialIndex";   break;
        case Command::Foreign:      indexType = "foreign";        break;
        default: indexType = "index";
        }
        QString index = table + "_" + columns.join("_") + "_" + indexType;
        index = index.toLower();
        return index.replace(QRegExp("-|\\."), "_");
    }

    ColumnDefinition &addColumn(const QString &type, const QString &name, const QHash<int, QVariant> &parameters = {})
    {
        ColumnDefinition column
        {
            {ColumnDefinition::Type, type},
            {ColumnDefinition::Name, name},
        };
        for(auto it = parameters.begin(); it != parameters.end(); ++it)
        {
            // allow repalced common key with value
            column[it.key()] = it.value();
        }
        columns.append(column);

        return columns[columns.size() - 1];
    }

    ColumnDefinition &addColumn(Schema::Type type, const QString &name, const QHash<int, QVariant> &parameters = {})
    {
        ColumnDefinition column
        {
            {ColumnDefinition::Type, type},
            {ColumnDefinition::Name, name},
        };
        for(auto it = parameters.begin(); it != parameters.end(); ++it)
        {
            // allow repalced common key with value
            column[it.key()] = it.value();
        }
        columns.append(column);

        return columns[columns.size() - 1];
    }

    ColumnDefinition &addColumn(const QString &type, const QString &name, const QStringList &allowed)
    {
        ColumnDefinition column
        {
            {ColumnDefinition::Type, type},// eg: register a typeChar(ColumnDefinition) function?
            {ColumnDefinition::Name, name},
            {ColumnDefinition::Allowed, allowed},
        };

        columns.append(column);
        return columns[columns.size() - 1];
    }

    /**
     * @brief we should add a command index for the column. eg: table->integer("id")->index();
     */
    void addColumnIndexes()
    {
        Q_Q(Blueprint);
        for(ColumnDefinition &column : columns)
        {
            for(ColumnDefinition::AttributeKey index
                : {ColumnDefinition::Primary, ColumnDefinition::Unique,
                ColumnDefinition::Index, ColumnDefinition::SpatialIndex})
            {
                if(column.attributes().find(index) != column.attributes().end())
                {
                    // toBool() return true if nameValue is not one of empty or "0", "false"(lower-case)
                    QVariant nameValue = column[ColumnDefinition::Index];
                    QString indexName = column[index].toBool() ? nameValue.toString() : "";
                    QStringList columnNames = column[ColumnDefinition::Name].toStringList();

                    if(index == ColumnDefinition::Primary)
                        q->primary(columnNames, indexName);
                    else if(index == ColumnDefinition::Unique)
                        q->unique(columnNames, indexName);
                    else if(index == ColumnDefinition::Index)
                        q->index(columnNames, indexName);
                    else if(index == ColumnDefinition::SpatialIndex)
                        q->spatialIndex(columnNames, indexName);

                    // see above(if-else)
                    // indexCommand(index,  column.value(ColumnDefinition::Name).toStringList(), indexName);
                    break;
                }
            }
        }
    }

    void addImpliedCommands()
    {
        Q_Q(Blueprint);
        if(!q->creating())
        {
            if(getAddedColumns().size() > 0)
                commands.prepend(createCommand(Command::Add));

            if(getChangedColumns().size() > 0)
                commands.prepend(createCommand(Command::Change));
        }

        addColumnIndexes();

        // TODO: add commands on any columns
    }

    QList<ColumnDefinition> getAddedColumns()
    {
        return Arr::array_filter<ColumnDefinition>(columns, [](const ColumnDefinition &val)
        {
            return !val[ColumnDefinition::Change].toBool();
        });
    }

    QList<ColumnDefinition> getChangedColumns()
    {
        return Arr::array_filter<ColumnDefinition>(columns, [](const ColumnDefinition &val)
        {
            return val[ColumnDefinition::Change].toBool();
        });
    }

    void ensureCommandsValid(Database *db)
    {
        if(db->driverName() == "QSQLITE")
        {
            QList<int> list = {Command::DropColumn, Command::RenameColumn};
            QList<Command> results;
            for (const Command &item : commands)
            {
                if(item.type() == Command::DropForeign)
                {
                    // TODO: throw a exception
                    qCritical() << "SQLite doesn't support dropping foreign keys (you would need to re-create the table).";
                    break;
                }
                if(list.contains(item.type()))
                    results << item;
            }

            if(results.size() > 1)
            {
                // TODO: throw a exception
                qCritical() << "SQLite doesn't support multiple calls to dropColumn / renameColumn in a single modification.";
            }
        }
    }


public:
    Blueprint *q_ptr = nullptr;
    QString table;

    // [ {"key" => variant, "key2" => variant}, {"key" => variant, "key2" => variant}, ...]
    QList<Command> commands;
    QList<ColumnDefinition> columns;
    bool temporary = false;
};


/**
 * @brief Blueprint::Blueprint
 * @param table
 * @param callback
 */
Blueprint::Blueprint(const QString &table, Closure callback)
    : d_ptr(new BlueprintPrivate(this))
{
    d_ptr->table = table;
    if(callback)
    {
        callback(this);
    }
}

Blueprint::~Blueprint()
{
    qDebug() << "Blueprint::~Blueprint()";
}

void Blueprint::build(Database *db, Grammar *grammar)
{
    qDebug() << "Blueprint::build()\n";
    foreach(auto sql, this->toSql(db, grammar))
    {
//        if(sql.isEmpty())
//            continue;

//        db->statement(sql);
        qDebug() << sql;
    }

    qDebug() << "\n";
}

QStringList Blueprint::toSql(Database *db, Grammar *grammar)
{
    Q_D(Blueprint);
    Q_UNUSED(db)

    d->addImpliedCommands();
//    d->ensureCommandsValid(db);

    // compiling sql
    return grammar->compile(this);
}

QString Blueprint::table() const
{
    Q_D(const Blueprint);
    return d->table;
}

QList<Command> Blueprint::commands() const
{
    Q_D(const Blueprint);
    return d->commands;
}

QList<Command> Blueprint::commands(int type) const
{
    Q_D(const Blueprint);
    QList<Command> commands;
    foreach(auto cmd, d->commands)
    {
        if(cmd.type() == type)
            commands << cmd;
    }

    return commands;
}

Command Blueprint::command(int type) const
{
    Q_D(const Blueprint);
    foreach(auto cmd, d->commands)
    {
        if(cmd.type() == type)
            return cmd;
    }

    return Command(Command::UnknownType);
}

QList<ColumnDefinition> Blueprint::columns() const
{
    Q_D(const Blueprint);
    return d->columns;
}

QList<ColumnDefinition> Blueprint::columns(int key) const
{
    Q_D(const Blueprint);
    return Arr::array_filter<ColumnDefinition>(d->columns, [key](const ColumnDefinition &val)
    {
        return val[key].toBool();
    });
}

QList<ColumnDefinition> Blueprint::addedColumns() const
{
    Q_D(const Blueprint);
    return Arr::array_filter<ColumnDefinition>(d->columns, [](const ColumnDefinition &val)
    {
        return !val[ColumnDefinition::Change].toBool();
    });
}

bool Blueprint::creating() const
{
    Q_D(const Blueprint);
    foreach (auto cmd, d->commands)
    {
        if(cmd.type() == Command::Create)
            return true;
    }
    return false;
}

bool Blueprint::isTemporary() const
{
    Q_D(const Blueprint);
    return d->temporary;
}

void Blueprint::create()
{
    Q_D(Blueprint);
    d->addCommand(Command::Create);
}

void Blueprint::temporary()
{
    Q_D(Blueprint);
    d->temporary = true;
}

void Blueprint::drop()
{
    Q_D(Blueprint);
    d->addCommand(Command::Drop);
}

void Blueprint::dropIfExists()
{
    Q_D(Blueprint);
    d->addCommand(Command::DropIfExists);
}

void Blueprint::dropColumn(const QStringList &columns)
{
    Q_D(Blueprint);
    d->addCommand(Command::DropColumn, {}, columns);
}

void Blueprint::dropPrimary(const QString &index)
{
    Q_D(Blueprint);
    d->dropIndexCommand(Command::DropPrimary, index);
}

void Blueprint::dropUnique(const QString &index)
{
    Q_D(Blueprint);
    d->dropIndexCommand(Command::DropUnique, index);
}

void Blueprint::dropIndex(const QString &index)
{
    Q_D(Blueprint);
    d->dropIndexCommand(Command::DropIndex, index);
}

void Blueprint::dropForeign(const QString &index)
{
    Q_D(Blueprint);
    d->dropIndexCommand(Command::DropForeign, index);
}

void Blueprint::dropSpatialIndex(const QString &index)
{
    Q_D(Blueprint);
    d->dropIndexCommand(Command::DropSpatialIndex, index);
}

void Blueprint::dropTimestamps()
{
    this->dropColumn({"created_at", "updated_at"});
}

void Blueprint::dropTimestampsTz()
{
    this->dropTimestamps();
}

void Blueprint::dropSoftDeletes()
{
    this->dropColumn({"deleted_at"});
}

void Blueprint::dropSoftDeletesTz()
{
    this->dropSoftDeletes();
}

void Blueprint::dropRememberToken()
{
    this->dropColumn({"remember_token"});
}

void Blueprint::renameColumn(const QString &from, const QString &to)
{
    Q_D(Blueprint);
    QVariantHash params = {{"from", from}, {"to", to}};
    d->addCommand(Command::RenameColumn, params);
}

void Blueprint::renameIndex(const QString &from, const QString &to)
{
    Q_D(Blueprint);
    QVariantHash params = {{"from", from}, {"to", to}};
    d->addCommand(Command::RenameIndex, params);
}

void Blueprint::rename(const QString &from, const QString &to)
{
    Q_D(Blueprint);
    QVariantHash params = {{"from", from}, {"to", to}};
    d->addCommand(Command::Rename, params);
}

Command &Blueprint::primary(const QStringList &columns, const QString &name)
{
    Q_D(Blueprint);
    return d->indexCommand(Command::Primary, columns, name);
}

Command &Blueprint::unique(const QStringList &columns, const QString &name)
{
    Q_D(Blueprint);
    return d->indexCommand(Command::Unique, columns, name);
}

Command &Blueprint::index(const QStringList &columns, const QString &name)
{
    Q_D(Blueprint);
    return d->indexCommand(Command::Index, columns, name);
}

Command &Blueprint::spatialIndex(const QStringList &columns, const QString &name)
{
    Q_D(Blueprint);
    return d->indexCommand(Command::SpatialIndex, columns, name);
}

Command &Blueprint::foreign(const QStringList &columns, const QString &name)
{
    Q_D(Blueprint);
    return d->indexCommand(Command::Foreign, columns, name);
}

ColumnDefinition &Blueprint::increments(const QString &column)
{
    return this->unsignedInteger(column, true);
}

ColumnDefinition &Blueprint::character(const QString &column, int length)
{
    Q_D(Blueprint);
    qint32 len = -1 == length ? 255 : length;
    Fluent params{{ColumnDefinition::Length, len}, };
//    return d->addColumn("Char", column, params);
    return d->addColumn(Schema::Char, column, params);
}

ColumnDefinition &Blueprint::string(const QString &column, int length)
{
    Q_D(Blueprint);
    qint32 len = -1 == length ? 255 : length;
    Fluent params = {{ColumnDefinition::Length, len}, };
//    return d->addColumn("String", column, params);
    return d->addColumn(Schema::String, column, params);
}

ColumnDefinition &Blueprint::text(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::Text, column);
}

ColumnDefinition &Blueprint::mediumText(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::MediumText, column);
}

ColumnDefinition &Blueprint::longText(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::LongText, column);
}

ColumnDefinition &Blueprint::integer(const QString &column, bool autoIncrement, bool isUnsigned)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::AutoIncrement, autoIncrement},
        {ColumnDefinition::Unsigned, isUnsigned},
    };
    return d->addColumn(Schema::Integer, column, params);
}

ColumnDefinition &Blueprint::tinyInteger(const QString &column, bool autoIncrement, bool isUnsigned)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::AutoIncrement, autoIncrement},
        {ColumnDefinition::Unsigned, isUnsigned},
    };
    return d->addColumn(Schema::TinyInteger, column, params);
}

ColumnDefinition &Blueprint::smallInteger(const QString &column, bool autoIncrement, bool isUnsigned)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::AutoIncrement, autoIncrement},
        {ColumnDefinition::Unsigned, isUnsigned},
    };
    return d->addColumn(Schema::SmallInteger, column, params);
}

ColumnDefinition &Blueprint::mediumInteger(const QString &column, bool autoIncrement, bool isUnsigned)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::AutoIncrement, autoIncrement},
        {ColumnDefinition::Unsigned, isUnsigned},
    };
    return d->addColumn(Schema::MediumInteger, column, params);
}

ColumnDefinition &Blueprint::bigInteger(const QString &column, bool autoIncrement, bool isUnsigned)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::AutoIncrement, autoIncrement},
        {ColumnDefinition::Unsigned, isUnsigned},
    };
    return d->addColumn(Schema::BigInteger, column, params);
}

ColumnDefinition &Blueprint::unsignedInteger(const QString &column, bool autoIncrement)
{
    return integer(column, autoIncrement, true);
}

ColumnDefinition &Blueprint::unsignedTinyInteger(const QString &column, bool autoIncrement)
{
    return tinyInteger(column, autoIncrement, true);
}

ColumnDefinition &Blueprint::unsignedSmallInteger(const QString &column, bool autoIncrement)
{
    return smallInteger(column, autoIncrement, true);
}

ColumnDefinition &Blueprint::unsignedMediumInteger(const QString &column, bool autoIncrement)
{
    return mediumInteger(column, autoIncrement, true);
}

ColumnDefinition &Blueprint::unsignedBigInteger(const QString &column, bool autoIncrement)
{
    return bigInteger(column, autoIncrement, true);
}

ColumnDefinition &Blueprint::floatNumeric(const QString &column, int total, int places)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Total, total},
        {ColumnDefinition::Places, places},
    };
    return d->addColumn(Schema::Float, column, params);
}

ColumnDefinition &Blueprint::doubleNumeric(const QString &column, int total, int places)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Total, total},
        {ColumnDefinition::Places, places},
    };
    return d->addColumn(Schema::Double, column, params);
}

ColumnDefinition &Blueprint::decimal(const QString &column, int total, int places)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Total, total},
        {ColumnDefinition::Places, places},
    };
    return d->addColumn(Schema::Decimal, column, params);
}

ColumnDefinition &Blueprint::unsignedDecimal(const QString &column, int total, int places)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Total, total},
        {ColumnDefinition::Places, places},
        {ColumnDefinition::Unsigned, true},
    };
    return d->addColumn(Schema::Decimal, column, params);
}

ColumnDefinition &Blueprint::boolean(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::Boolean, column);
}

ColumnDefinition &Blueprint::enumerate(const QString &column, const QStringList &allowed)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Allowed, allowed},
    };
    return d->addColumn(Schema::Enum, column, params);
}

ColumnDefinition &Blueprint::json(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::Json, column);
}

ColumnDefinition &Blueprint::jsonb(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::Jsonb, column);
}

ColumnDefinition &Blueprint::date(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::Date, column);
}

ColumnDefinition &Blueprint::dateTime(const QString &column, int precision)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Precision, precision},
    };
    return d->addColumn(Schema::DataTime, column, params);
}

ColumnDefinition &Blueprint::dateTimeTz(const QString &column, int precision)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Precision, precision},
    };
    return d->addColumn(Schema::DateTimeTz, column, params);
}

ColumnDefinition &Blueprint::time(const QString &column, int precision)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Precision, precision},
    };
    return d->addColumn(Schema::Time, column, params);
}

ColumnDefinition &Blueprint::timeTz(const QString &column, int precision)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Precision, precision},
    };
    return d->addColumn(Schema::TimeTz, column, params);
}

ColumnDefinition &Blueprint::timestamp(const QString &column, int precision)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Precision, precision},
    };
    return d->addColumn(Schema::Timestamp, column, params);
}

ColumnDefinition &Blueprint::timestampTz(const QString &column, int precision)
{
    Q_D(Blueprint);
    Fluent params
    {
        {ColumnDefinition::Precision, precision},
    };
    return d->addColumn(Schema::TimestampTz, column, params);
}

void Blueprint::timestamps(int precision)
{
    this->timestamp("created_at", precision).nullable();
    this->timestamp("updated_at", precision).nullable();
}

void Blueprint::nullableTimestamps(int precision)
{
    this->timestamps(precision);
}

void Blueprint::timestampsTz(int precision)
{
    this->timestampTz("created_at", precision).nullable();
    this->timestampTz("updated_at", precision).nullable();
}

ColumnDefinition &Blueprint::softDeletes(const QString &column, int precision)
{
    return timestamp(column, precision).nullable();
}

ColumnDefinition &Blueprint::softDeletesTz(const QString &column, int precision)
{
    return  timestampTz(column, precision).nullable();
}

ColumnDefinition &Blueprint::year(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::Year, column);
}

ColumnDefinition &Blueprint::binary(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::Binary, column);
}

ColumnDefinition &Blueprint::uuid(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::Uuid, column);
}

ColumnDefinition &Blueprint::ipAddress(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::IpAddress, column);
}

ColumnDefinition &Blueprint::macAddress(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::MacAddress, column);
}

ColumnDefinition &Blueprint::geometry(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::Geometry, column);
}

ColumnDefinition &Blueprint::point(const QString &column, int srid)
{
    Q_D(Blueprint);
    Q_UNUSED(srid)

    return d->addColumn(Schema::Point, column);
}

ColumnDefinition &Blueprint::lineString(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::LineString, column);
}

ColumnDefinition &Blueprint::polygon(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::Polygon, column);
}

ColumnDefinition &Blueprint::geometryCollection(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::GeometryCollection, column);
}

ColumnDefinition &Blueprint::multiPoint(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::MultiPoint, column);
}

ColumnDefinition &Blueprint::multiLineString(const QString &column)
{
    Q_D(Blueprint);
   return d->addColumn(Schema::MultiLineString, column);
}

ColumnDefinition &Blueprint::multiPolygon(const QString &column)
{
    Q_D(Blueprint);
    return d->addColumn(Schema::MultiPolygon, column);
}

void Blueprint::morphs(const QString &name, const QString &indexName)
{
    QString morphName = name + "_type";
    QString morphKey = name + "_id";

    this->string(morphName);
    this->unsignedInteger(morphKey);
    this->index({morphKey, morphName}, indexName);
}

void Blueprint::nullableMorphs(const QString &name, const QString &indexName)
{
    QString morphName = name + "_type";
    QString morphKey = name + "_id";

    this->string(morphName).nullable();
    this->unsignedInteger(morphKey).nullable();
    this->index({morphKey, morphName}, indexName);
}

ColumnDefinition &Blueprint::rememberToken()
{
    // TODO: set null value
    return string("remember_token", 100).nullable();
}
