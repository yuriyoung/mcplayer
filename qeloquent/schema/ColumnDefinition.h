#ifndef COLUMNDEFINITION_H
#define COLUMNDEFINITION_H
#include "QEloquentMacro.h"
#include <QObject>
#include <QVariant>

/**
 * class ColumnDefinition
 * - after          // place the column "after" another column (MySql)
 * - always         // used as a modifier for generatedAs (PostgreSQL)
 * - autoIncrement  // set INTEGER column as auto-increment (primay key)
 * - change         // change the column
 * - charset        // Specify a character set for the column (MySQL)
 * - collation      // collation for the column (MySQL/PostgreSQL/SQL Server)
 * - comment        // Add a comment to the column (MySQL)
 * - default        // Specify a "default" value for the column
 * - first          // Place the column "first" in the table (MySQL)
 * - generateAs     // Create a SQL compliant identity column (PostgreSQL)
 * - index          // Add an index
 * - nullable       // Allow NULL values to be inserted into the column
 * - primary        // Add a primary index
 * - spatialIndex   // Add a spatial index
 * - storedAs       // Create a stored generated column (MySQL)
 * - unique         // Add a unique index
 * - unsigned       // Set the INTEGER column as UNSIGNED (MySQL)
 * - useCurrent     // Set the TIMESTAMP column to use CURRENT_TIMESTAMP as default value
 * - virtaulAs      // Create a virtual generated column (MySQL)
 * - persisted      // Mark the computed generated column as persistent (SQL Server)
 *
 */
class ColumnDefinitionPrivate;
class QELOQUENT_EXPORT ColumnDefinition
{
    friend class Blueprint;
    friend class SchemaGrammar;
public:
    enum AttributeKey
    {
        // column
        After = 0,
        Allowed,
        AutoIncrement,
        Change,
        Charset,
        Comment,
        DefaultValue,
        First,
        Length,
        Index,
        Name,   // the column name
        Places,
        Primary,
        Precision,
        Nullable,
        Unique,
        Unsigned,
        UseCurrent,
        SpatialIndex,
        Total,
        Type,   // the column type

        UserKey = 0x0100
    };

    ColumnDefinition();
    ColumnDefinition(const ColumnDefinition &other);
    inline ColumnDefinition(std::initializer_list<std::pair<int, QVariant> > list)
        : ColumnDefinition()
    {
        attributes().reserve(int(list.size()));
        for (typename std::initializer_list<std::pair<int, QVariant> >::const_iterator it = list.begin(); it != list.end(); ++it)
            attributes().insert(it->first, it->second);
    }

    ~ColumnDefinition();

    ColumnDefinition &operator=(const ColumnDefinition &other);
    bool operator==(const ColumnDefinition &other) const;

    QVariant &operator[](int key);
    const QVariant operator[](int key) const;

    QHash<int, QVariant> &attributes() const;

    QVariant value(AttributeKey key, const QVariant &defaultValue = QVariant()) const;
    void setValue(AttributeKey key, const QVariant &value);

    // Place the column "after" another column (MySQL)
    ColumnDefinition &after(const QString &column);

    // Set INTEGER columns as auto-increment (primary key)
    ColumnDefinition &autoIncrement();

    // Change the column
    ColumnDefinition &change();

     // Specify a character set for the column (MySQL)
    ColumnDefinition &charset(const QString &charset);

    // Add a comment to the column (MySQL)
    ColumnDefinition &comment(const QString &comment);

    // Specify a "default" value for the column
    ColumnDefinition &defaultValue(const QVariant &value);

    // Place the column "first" in the table (MySQL)
    ColumnDefinition &first();

    // Add an index
    ColumnDefinition &index(const QString &name);

    // Add a primary index
    ColumnDefinition &primary();

    // Allow NULL values to be inserted into the column
    ColumnDefinition &nullable(bool value = true);

    // Add a unique index
    ColumnDefinition &unique();

    // Set the TIMESTAMP column to use CURRENT_TIMESTAMP as default value
    ColumnDefinition &useCurrent();

protected:
    int length() const;
    int total() const;
    int places() const;
    int precision() const;
    bool isUseCurrent() const;
    QStringList allowed() const;
    bool isNullable() const;

private:
    ColumnDefinitionPrivate *d;
};


#endif // COLUMNDEFINITION_H
