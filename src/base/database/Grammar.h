#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <QObject>

class GrammarPrivate;
class Grammar : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Grammar)
public:
    Grammar(QObject *parent = nullptr);
    virtual ~Grammar();

    // virtual QHash<int, QString> components() = 0;

    // Compile a select query into SQL.
    virtual QStringList compile(void *data) = 0;

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

    //  Create query parameter place-holders for an array.
    QString parameterize(const QStringList &values) const;
    // Get the appropriate query parameter place-holder for a value.
    QString parameter(const QString &value) const; // return "?";

    // Quote the given string literal.
    QString quoteString(const QStringList &values) const;
    QString quoteString(const QString &value) const;

    // Get the format for database stored dates.
    virtual QString dataFormat() const { return "Y-m-d H:i:s"; }

    // Set the grammar's table prefix.
    void setTablePrefix(const QString &prefix);
    // Get the grammar's table prefix.
    QString tablePrefix() const;

protected:
    Grammar(GrammarPrivate &dd, QObject *parent = nullptr);
    QScopedPointer<GrammarPrivate> d_ptr;
};

#endif // GRAMMAR_H
