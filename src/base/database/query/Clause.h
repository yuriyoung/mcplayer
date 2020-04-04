#ifndef CLAUSE_H
#define CLAUSE_H

#include "QueryBuilder.h"
#include "QueryGrammar.h"

#include <QObject>
#include <QVariant>
#include <QDebug>
#include <QExplicitlySharedDataPointer>

//QT_FORWARD_DECLARE_CLASS(ClausePrivate)

// issue: compile failed forward declare while using QExplicitlySharedDataPointer (damn!!!)
class ClausePrivate : public QSharedData
{
public:
    ClausePrivate() { }
    ClausePrivate(const QStringList &columns) : columns(columns) { }
    ClausePrivate(const ClausePrivate &other) : QSharedData(other) { }
    virtual ~ClausePrivate() {}

public:
    QStringList columns = {};
    QVariantMap bindings;
};

class Clause
{
    Q_DECLARE_PRIVATE(Clause)
public:
    enum ClauseType
    {
        Aggregate = 0,
        Column,
        From,
        Join,
        Where,
        GroupBy,
        Having,
        Order,
        Union,
        UnionOrder,
        Limit,
        Offset,
    };

    Clause();
    Clause(const QStringList &columns);

    virtual ~Clause();

    virtual int type() const = 0;
    virtual QString interept(QueryGrammar *grammar) { Q_UNUSED(grammar) return ""; }
    virtual void setBindings(const QVariantMap &bindings);
    virtual QVariantMap bindings() const;

    virtual void setColumns(const QString &columns);
    virtual void setColumns(const QStringList &columns);
    virtual QStringList columns() const;

protected:
    Clause(ClausePrivate &dd);
    QExplicitlySharedDataPointer<ClausePrivate> d_ptr; // only one ref instance of
};

class FromClausePrivate;
class FromClause : public Clause
{
    Q_DECLARE_PRIVATE(FromClause)
public:
    FromClause(const QString &table, const QString &as = {}, const QVariantMap &bindings = {});
    FromClause(const FromClause &other);

    QString interept(QueryGrammar *grammar) override;
    int type() const override { return From; }
    QString table() const;
    QString as() const;
    QVariantMap bindings() const override;
};

class AggregateClausePrivate;
class AggregateClause : public Clause
{
    Q_DECLARE_PRIVATE(AggregateClause)
public:
    AggregateClause(QueryBuilder *query, const QString &function, const QString &columns = "*");
    AggregateClause(QueryBuilder *query, const QString &function, const QStringList &columns = { "*" });

    int type() const override { return Aggregate; }
    QString function() const;
    QueryBuilder *query() const;
};

class ColumnClausePrivate;
class ColumnClause : public Clause
{
    Q_DECLARE_PRIVATE(ColumnClause)
public:
    ColumnClause(QueryBuilder *query, const QString &columns);
    ColumnClause(QueryBuilder *query, const QStringList &columns);
    ColumnClause(const ColumnClause &other);

    int type() const override { return Column; }
    QString interept(QueryGrammar *grammar) override;
    QueryBuilder *query() const;
};

class JoinClausePrivate;
class JoinClause : public Clause
{
    Q_DECLARE_PRIVATE(JoinClause)
public:
    enum JoinType
    {
        Left,
        Inner,
        Right,
        Cross
    };

    JoinClause(QueryBuilder *parent, const QString &type, const QString &table);

    int type() const override { return Join; }
    QString interept(QueryGrammar *grammar) override;

    QString table() const;
    QString joinType() const;

    QueryBuilder &on(const QString &first, const QString &op = "",
               const QString &second = "", const QString &boolean = "and");
    QueryBuilder &orOn(const QString &first, const QString &op = "",
                 const QString &second = "");
    QueryBuilder &where(const QString &first, const QString &op = "",
                  const QString &second = "", const QString &boolean = "and");

    QueryBuilder *query() const;
    QList<JoinClause *> joins() const;
};

class WhereClausePrivate;
class WhereClause : public Clause
{
    Q_DECLARE_PRIVATE(WhereClause)
public:

    WhereClause(QueryBuilder *query, const QString &boolean);

    WhereClause(const QString &column, const QVariant &value,
                const QString &op, const QString &boolean);

    WhereClause(const QString &column, const QVariant &value,
                const QString &boolean);

    WhereClause(const QString &first, const QString &op,
                const QString &second, const QString &boolean);

    // betweenOrNotBetween equal true return between, false returns not between
    WhereClause(const QString &column, const QVariant &value,
                const QString &boolean, bool betweenOrNotBetween);

    int type() const override { return Where; }
    QString interept(QueryGrammar *grammar) override;

    void setInvokableMethod(const QString &method);
    QString invokableMethod() const;

    bool betweenOrNot() const;
    QVariant value() const;
    QString op() const ;
    QString boolean();
    QString first() const;
    QString second() const;
    QueryBuilder *query() const; // TODO: add a sub-select
};

class HavingClausePrivate;
class HavingClause : public Clause
{
    Q_DECLARE_PRIVATE(HavingClause)
public:
    HavingClause(const QString &column, const QVariant &value,
                 const QString &op, const QString &boolean);

    // having between/not between
    HavingClause(const QString &column, const QVariant &value,
                 const QString &boolean, bool betweenOrNotBetween);

    int type() const override { return Having; }
    QString interept(QueryGrammar *grammar) override;

    QVariant value() const;
    QString boolean() const;
    QString op() const;
    bool betweenOrNot() const;
};

class GroupClausePrivate;
class GroupClause : public Clause
{
    Q_DECLARE_PRIVATE(GroupClause)
public:
    GroupClause(const QString &columns);
    GroupClause(const QStringList &columns);

    int type() const override { return GroupBy; }
    QString interept(QueryGrammar *grammar) override;
};

class OrderClausePrivate;
class OrderClause : public Clause
{
    Q_DECLARE_PRIVATE(OrderClause)
public:
    OrderClause(const QString &columns, const QString &direction = "asc");
    OrderClause(const QStringList &columns, const QString &direction = "asc");

    int type() const override { return Order; }
    QString interept(QueryGrammar *grammar) override;
    QString direction() const;
};

class UnionClausePrivate;
class UnionClause : public Clause
{
    Q_DECLARE_PRIVATE(UnionClause)
public:
    UnionClause(QueryBuilder *query, bool all = false);

    int type() const override { return Union; }
    QString interept(QueryGrammar *grammar) override;

    QueryBuilder *query() const;
    bool all() const;
};

class LimitClausePrivate;
class LimitClause : public Clause
{
    Q_DECLARE_PRIVATE(LimitClause)
public:
    LimitClause(int limit);

    int type() const override { return Limit; }
    QString interept(QueryGrammar *grammar) override;
    int limit() const;
};

class OffsetClausePrivate;
class OffsetClause : public Clause
{
    Q_DECLARE_PRIVATE(OffsetClause)
public:
    OffsetClause(int offset);

    int type() const override { return Offset; }
    QString interept(QueryGrammar *grammar) override;
    int offset() const;
};



#endif // CLAUSE_H
