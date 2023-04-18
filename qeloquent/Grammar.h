#ifndef GRAMMAR_H
#define GRAMMAR_H
#include "QEloquentMacro.h"
#include <QObject>

class GrammarPrivate;
// derive QObject for invoke methods
class QELOQUENT_EXPORT Grammar : public QObject
{
    Q_DECLARE_PRIVATE(Grammar)
public:
    Grammar(QObject *parent = nullptr);
    virtual ~Grammar();

    // virtual QHash<int, QString> components() = 0;

    // Compile a select query into SQL.
    virtual QStringList compile(void *data, int type = 0) = 0;

    // Wrap an array of values.
    virtual QStringList wrapArray(const QStringList &values) const;

    // Wrap a table in keyword identifiers.
    virtual QString wrapTable(const QString &table) const;

    // Wrap a value in keyword identifiers.
    virtual QString wrap(const QString &value, bool prefixAlias = false) const; // as

    // Wrap a value that has an alias.
    virtual QString wrapAliasedValue(const QString &value, bool prefixAlias = false) const;

    // Wrap the given value segments.
    virtual QString wrapSegments(const QStringList &segments) const;

    // Wrap a single string in keyword identifiers.
    virtual QString wrapValue(const QString &value) const;

    //  Convert an array of column names into a delimited string.
    QString columnize(const QString &columns) const;
    QString columnize(const QStringList &columns) const;

    // Create query parameter place-holders for an array.
    QString parameterize(const QVariantList &values, bool prepared = false) const;
    // Get the appropriate query parameter place-holder for a value.
    QString parameter(const QVariant &value, bool prepared = false) const;
    // Prepares the appropriate query place-holder for columns
    QString preparize(const QStringList &columns) const;

    // Quote the given string literal.
    QString quoteString(const QStringList &values) const;
    QString quoteString(const QString &value) const;

    // Get the format for database stored dates.
    virtual QString dateFormat() const;
    virtual QString datetimeFormat() const;

    // Set the grammar's table prefix.
    void setTablePrefix(const QString &prefix);
    // Get the grammar's table prefix.
    QString tablePrefix() const;

    QString fromValue(const QVariant &value) const;

protected:
    Grammar(GrammarPrivate &dd, QObject *parent = nullptr);
    QScopedPointer<GrammarPrivate> d_ptr;
};

#endif // GRAMMAR_H
