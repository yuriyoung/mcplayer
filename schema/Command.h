#ifndef COMMAND_H
#define COMMAND_H

#include <QStringList>
#include <QHash>
#include <QVariant>
#include <QString>

/**
 * class ForeignKeyDefinition
 * - references      // specify the referenced column(s)
 * - on             // specify the refernced table
 * - onDelete       // add on ON DELETE action
 * - onUpdate       // add on ON UPDATE action
 * - deferrable         // set the foreign key as deferrable (for PastgreSQL)
 * - initiallyImmediate // set the default time to check the canstraint (PastgreSQL)
 *
 */
class Blueprint;
class CommandPrivate;
class Command
{
public:
    enum Type
    {
        UnknownType = -1,
        Primary = 0,
        Unique,
        Index,
        SpatialIndex,

        Add,
        Change,
        Create,
        Foreign,

        Rename,
        RenameIndex,
        RenameColumn,

        Drop,
        DropIfExists,
        DropColumn,
        DropPrimary,
        DropUnique,
        DropIndex,
        DropSpatialIndex,
        DropForeign,
    };

    explicit Command(Command::Type command, Blueprint *blueprint = nullptr,
                     const QStringList &columns = QStringList(),
                     const QString &index = QString());
    Command(const Command &other);
    ~Command();

    Command &operator=(const Command& other);
    bool operator==(const Command& other) const;
    inline bool operator!=(const Command &other) const { return !operator==(other); }

    QVariant &operator[](const QString &key);
    const QVariant operator[](const QString &key) const;

    // Specify the referenced column(s)
    Command &references(const QString &column);
    Command &references(const QStringList &columns);

    // Specify the referenced table
    Command &on(const QString &table);

    // Add an ON DELETE action
    Command &onDelete(const QString &cation);

    // Add an ON UPDATE action
    Command &onUpdate(const QString &cation);

    // Set the foreign key as deferrable (PostgreSQL)
    Command &deferrable(bool value = true);

    // Set the default time to check the constraint (PostgreSQL)
    Command &initiallyImmediate(bool value = true);

    Blueprint *blueprint() const;
    QVariantHash &attributes() const;
    Command::Type type() const;
    QString name() const;
    QStringList columns() const;
    QString indexName() const;
    bool isValid() const;

private:
    CommandPrivate *d = nullptr;
};

#endif // COMMAND_H
