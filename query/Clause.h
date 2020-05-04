#ifndef CLAUSE_H
#define CLAUSE_H

#include <QObject>
#include <QVariant>
#include <QExplicitlySharedDataPointer>

class QueryBuilder;
class QueryGrammar;
QT_FORWARD_DECLARE_CLASS(ClausePrivate)
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

    explicit Clause(QueryBuilder *parent = nullptr);
    virtual ~Clause();

    virtual QString interpret(QueryGrammar *grammar, bool leading = false) = 0;

    void setColumns(const QString &columns);
    void setColumns(const QStringList &columns);
    QStringList columns() const;
    QueryBuilder *query() const;

protected:
    Clause(ClausePrivate &dd, QueryBuilder *parent = nullptr);
    QExplicitlySharedDataPointer<ClausePrivate> d_ptr; // only one ref instance of
};

class FromClausePrivate;
class FromClause : public Clause
{
    Q_DECLARE_PRIVATE(FromClause)
public:
    FromClause(const QString &table, const QString &as = {}, const QVariantMap &bindings = {});

    QString interpret(QueryGrammar *grammar, bool leading) override;
    QString table() const;
    QVariantMap bindings() const;
};

class AggregateClausePrivate;
class AggregateClause : public Clause
{
    Q_DECLARE_PRIVATE(AggregateClause)
public:
    AggregateClause(QueryBuilder *parent, const QString &function,
                    const QString &columns = "*");
    AggregateClause(QueryBuilder *parent, const QString &function,
                    const QStringList &columns = { "*" });

    QString interpret(QueryGrammar *grammar, bool leading) override;
    QString function() const;
};

class ColumnClausePrivate;
class ColumnClause : public Clause
{
    Q_DECLARE_PRIVATE(ColumnClause)
public:
    ColumnClause(const QString &columns, QueryBuilder *parent);
    ColumnClause(const QStringList &columns, QueryBuilder *parent);

    // TODO: add select sub for column
    ColumnClause(const QueryBuilder &subQuery, const QString &as, QueryBuilder *parent);

    QString interpret(QueryGrammar *grammar, bool leading) override;
};

/**
 * TODO: handle nested joins
 */
class JoinClausePrivate;
class JoinClause : public Clause
{
    Q_DECLARE_PRIVATE(JoinClause)
public:
    enum Type
    {
        Inner,
        Left,
        Right,
        Cross
    };

    JoinClause(QueryBuilder *parent, JoinClause::Type type,
               const QString &table, bool where = false);

    JoinClause(QueryBuilder *parent, const QString &type,
               const QString &table, bool where = false);

    QString interpret(QueryGrammar *grammar, bool leading) override;


    /*!
     * \brief Add an "on" clause to the join.
     * On clauses can be chained, e.g.
     *
     *  $join->on('contacts.user_id', '=', 'users.id')
     *       ->on('contacts.info_id', '=', 'info.id')
     *
     * will produce the following SQL:
     *
     * on `contacts`.`user_id` = `users`.`id` and `contacts`.`info_id` = `info`.`id`
     */
    QueryBuilder &on(const QString &first, const QString &op = "",
               const QString &second = "", const QString &boolean = "and");

    QueryBuilder &on(std::function<void(const QueryBuilder &)> column,
                     const QString &boolean = "and");

    QueryBuilder &orOn(const QString &first, const QString &op = "",
                 const QString &second = "");

    QueryBuilder &where(const QString &first, const QString &op = "",
                  const QString &second = "", const QString &boolean = "and");

    QSharedPointer<QueryBuilder> subQuery() const;
    QString table() const;
    QString joinType() const;
    bool isWhere() const;
    QList<JoinClause *> joins() const;
};

class WhereClausePrivate;
class WhereClause : public Clause
{
    Q_DECLARE_PRIVATE(WhereClause)
public:
    enum Type
    {
        Base,
        In,
        NotIn,
        InRaw,
        NotInRaw,
        Null,
        NotNull,
        Between,
        NotBetween,
        Nested,
        Column,
        Date,
        Time,
        Day,
        Month,
        Year,
        Exists,
        NotExists,
        RowValues,
        Sub,
        JsonBoolean,
        JsonContains,
    };

    // sub query (nest where select), eg: where foo=(select * from ...)
    WhereClause(WhereClause::Type type,
                const QString &column,
                const QString &op,
                const QueryBuilder &subQuery,
                const QString &boolean = "and");

    // nested where
    WhereClause(WhereClause::Type type,
                const QSharedPointer<QueryBuilder> &subQuery,
                const QString &boolean);
    // nested where
    WhereClause(WhereClause::Type type,
                const QueryBuilder &subQuery,
                const QString &boolean);

    WhereClause(WhereClause::Type type,
                const QString &column,
                const QString &op,
                const QVariant &value,
                const QString &boolean);

    WhereClause(WhereClause::Type type,
                const QString &column,
                const QVariant &value,
                const QString &boolean);

/*
    WhereClause(QSharedPointer<QueryBuilder> query, const QString &boolean);

    WhereClause(const QString &column, const QVariant &value,
                const QString &op, const QString &boolean);

    WhereClause(const QString &column, const QVariant &value,
                const QString &boolean);

    WhereClause(const QString &first, const QString &op,
                const QString &second, const QString &boolean);

    // betweenOrNotBetween equal true return between, false returns not between
    WhereClause(const QString &column, const QVariant &value,
                const QString &boolean, bool betweenOrNotBetween);
*/

    QString interpret(QueryGrammar *grammar, bool leading) override;
    void setParentQuery(QueryBuilder *parent);
    QString whereMethod() const;

    int type() const;
    bool leading() const;
    QVariant value() const;
    QString op() const ;
    QString boolean();
    QString first() const;
    QString second() const;
    QSharedPointer<QueryBuilder> subQuery() const;
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

    QString interpret(QueryGrammar *grammar, bool leading) override;

    QVariant value() const;
    QString boolean() const;
    QString op() const;
    bool betweenOrNot() const;
    bool leading() const;
};

class GroupClausePrivate;
class GroupClause : public Clause
{
    Q_DECLARE_PRIVATE(GroupClause)
public:
    GroupClause(const QString &columns);
    GroupClause(const QStringList &columns);

    QString interpret(QueryGrammar *grammar, bool leading) override;
};

class OrderClausePrivate;
class OrderClause : public Clause
{
    Q_DECLARE_PRIVATE(OrderClause)
public:
    OrderClause(const QString &columns, const QString &direction = "asc");
    OrderClause(const QStringList &columns, const QString &direction = "asc");

    QString interpret(QueryGrammar *grammar, bool leading) override;
    QString direction() const;
    bool leading() const;
};

class UnionClausePrivate;
class UnionClause : public Clause
{
    Q_DECLARE_PRIVATE(UnionClause)
public:
    UnionClause(QueryBuilder *query, bool all = false);

    QString interpret(QueryGrammar *grammar, bool leading) override;

    bool all() const;
};

class LimitClausePrivate;
class LimitClause : public Clause
{
    Q_DECLARE_PRIVATE(LimitClause)
public:
    LimitClause(int limit);

    QString interpret(QueryGrammar *grammar, bool leading) override;
    int value() const;
};

class OffsetClausePrivate;
class OffsetClause : public Clause
{
    Q_DECLARE_PRIVATE(OffsetClause)
public:
    OffsetClause(int offset);

    QString interpret(QueryGrammar *grammar, bool leading) override;
    int value() const;
};

#endif // CLAUSE_H
