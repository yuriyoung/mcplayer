#include "Command.h"
#include "Column.h"

#include <QDebug>

class CommandPrivate
{
public:
    CommandPrivate(Command::Type command, const QStringList &columns, const QString &index);
    CommandPrivate(const CommandPrivate &other);
    bool operator==(const CommandPrivate& other) const;

    QAtomicInt ref;
    Command::Type type = Command::UnknownType;
    QStringList columns;
    QString index;

    QVariantHash attributes;
};

CommandPrivate::CommandPrivate(Command::Type command, const QStringList &columns, const QString &index)
    : ref(1), type(command), columns(columns), index(index)
{

}

CommandPrivate::CommandPrivate(const CommandPrivate &other)
    : ref(1), type(other.type), columns(other.columns), index(other.index)
{

}

bool CommandPrivate::operator==(const CommandPrivate &other) const
{
    return (type == other.type
            && index == other.index
            && columns == other.columns);
}

/**
 * @brief Command::Command
 * @param command
 * @param columns
 * @param index
 */
Command::Command(Command::Type command, const QStringList &columns, const QString &index)
    : d(new CommandPrivate(command, columns, index))
{

}

Command::Command(const Command &other)
{
    d = other.d;
    d->ref.ref();
}

Command::~Command()
{
    if (!d->ref.deref())
        delete d;
}

Command &Command::operator=(const Command &other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

bool Command::operator==(const Command &other) const
{
    return (d == other.d || *d == *other.d);
}

QVariant &Command::operator[](const QString &key)
{
    return d->attributes[key];
}

const QVariant Command::operator[](const QString &key) const
{
    return d->attributes[key];
}

Command &Command::references(const QString &columns)
{
    qAtomicDetach(d);
    d->attributes["references"] = columns;
    return *this;
}

Command &Command::references(const QStringList &columns)
{
    qAtomicDetach(d);
    d->attributes["references"] = columns;
    return *this;
}

Command &Command::on(const QString &table)
{
    qAtomicDetach(d);
    d->attributes["on"] = table;
    return *this;
}

Command &Command::onDelete(const QString &action)
{
    qAtomicDetach(d);
    d->attributes["onDelete"] = action;
    return *this;
}

Command &Command::onUpdate(const QString &action)
{
    qAtomicDetach(d);
    d->attributes["onUpdate"] = action;
    return *this;
}

Command &Command::deferrable(bool value)
{
    qAtomicDetach(d);
    d->attributes["deferrable"] = value;
    return *this;
}

Command &Command::initiallyImmediate(bool value)
{
    qAtomicDetach(d);
    d->attributes["initiallyImmediate"] = value;
    return *this;
}

QVariantHash &Command::attributes() const
{
    return d->attributes;
}

Command::Type Command::type() const
{
    return d->type;
}

QString Command::name() const
{
    QString name;
    switch (d->type)
    {
    case Primary:      name = "primary";        break;
    case Unique:       name = "unique";         break;
    case Index:        name = "index";          break;
    case SpatialIndex: name = "spatialIndex";   break;
    case Add:          name = "add";            break;
    case Create:       name = "create";         break;
    case Change:       name = "change";         break;
    case Foreign:      name = "foreign";        break;

    case Rename:       name = "rename";         break;
    case RenameIndex:  name = "renameIndex";    break;
    case RenameColumn: name = "renameColumn";   break;

    case Drop:         name = "drop";           break;
    case DropIfExists: name = "dropIfExists";   break;
    case DropColumn:   name = "dropColumn";     break;
    case DropPrimary:  name = "dropPrimary";    break;
    case DropUnique:   name = "dropUnique";     break;
    case DropIndex:    name = "dropIndex";      break;
    case DropSpatialIndex: name = "dropSpatialIndex"; break;
    case DropForeign:  name = "dropForeign";    break;
    default: break;
    }

    return name;
}

QStringList Command::columns() const
{
    return d->columns;
}

QString Command::indexName() const
{
    return d->index;
}

bool Command::isValid() const
{
    return d != nullptr && d->type != UnknownType;
}
