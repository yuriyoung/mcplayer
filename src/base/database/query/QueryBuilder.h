#ifndef QUERYBUILDER_H
#define QUERYBUILDER_H

#include <QObject>
#include <QSqlQuery>
#include <QVariant>

typedef QHash<int, QList<QVariantMap> > BindingsHash;

class Connection;
class Grammar;
class Clause;
class QueryBuilderPrivate;
class QueryBuilder
{
    Q_DECLARE_PRIVATE(QueryBuilder)
    QScopedPointer<QueryBuilderPrivate> d_ptr;
    friend class QueryGrammar;
public:
    enum CompileStatement
    {
        SelectStatement,
        InsertStatement,
        UpdateStatement,
        DeleteStatement,
        ExistsStatement,
        RandomStatement,
        TruncateStatement,
        RollBackStatement
    };

    enum BindingType
    {
        InsertBinding = 0,
        UpdateBinding,
        DeleteBinding,
        SelectBinding,
        FromBinding,
        JoinBinding,
        WhereBinding,
        GroupByBinding,
        HavingBinding,
        OrderBinding,
        UnionBinding,
        UnionOrderBinding,
    };

    QueryBuilder();
    QueryBuilder(const Connection *connection);
    QueryBuilder(const QueryBuilder &other);
    QueryBuilder(const QueryBuilder &&other);
    QueryBuilder& operator=(const QueryBuilder &other);
    ~QueryBuilder();

    void setConnection(const Connection *connection);
    Connection *connection() const;
    Grammar *grammar() const;

    QString table() const;
    bool isDistincted() const;
    bool isAggregated() const;

    QMap<int, QList<Clause *> > clauses() const;
    QList<Clause*> clauses(int type) const;

    QueryBuilder &setBindings(int bindingType, const QList<QVariantMap> &bindings);
    QueryBuilder &addBinding(int bindingType, const QVariantMap &value);
    QList<QVariantMap> bindings(QueryBuilder::BindingType type) const;
    BindingsHash bindings() const;

    QString toSql() const;
    bool exists();
    bool insert(const QVariantMap &value);
    bool insert(const QList< QVariantMap> &values);
    qint64 update(const QVariantMap &value);
    bool updateOrInsert(const QVariantMap &attribute, const QVariantMap &value);
    int destroy(const QVariant &id = QVariant());

    QueryBuilder &select(const QString &columns = "*");
    QueryBuilder &select(const QStringList &columns);
    QueryBuilder &selectRaw(const QString &expression, const QVariantMap &bindings = {});

    QueryBuilder &selectSub(const QueryBuilder &query, const QString &as);
    QueryBuilder &addSelect(const QString &column);
    QueryBuilder &from(const QString &table, const QString &as = "");
    QueryBuilder &fromSub(const QueryBuilder &query, const QString &as);
    QueryBuilder &distinct();

    void find(int id, const QString &columns = "*");
    void find(int id, const QStringList &columns = {"*"});

    QSqlQuery get(const QString &columns = "*");
    QSqlQuery get(const QStringList &columns = {"*"});

    QueryBuilder &join(const QString &table, const QString &first, const QString &op = "",
                       const QString &second = "", const QString &type = "inner", bool where = false);

    QueryBuilder &joinWhere(const QString &table, const QString &first, const QString &op = "",
                       const QString &second = "", const QString &type = "inner");

    QueryBuilder &joinSub(const QueryBuilder &query, const QString &as,
                          const QString &first, const QString &op = "", const QString &second = "",
                          const QString &type = "inner", bool where = false);

    // Add a basic where clause to the query.
    QueryBuilder &where(const QString &column, const QVariant &op,
                        const QVariant &value = QVariant(), const QString &boolean = "and");
    QueryBuilder &where(const QVariantMap &attributes, const QString &boolean = "and");
    // Add a nested where statement to the query.
    QueryBuilder &where(std::function<void(const QueryBuilder &)> column,
                        const QString &boolean = "and");
    // nested where select statement
    QueryBuilder &where(const QString &column, const QString &op,
                        std::function<void(const QueryBuilder &)> value,
                        const QString &boolean = "and");

    // Add an "or where" clause to the query.
    QueryBuilder &orWhere(const QString &column, const QString &op = "",
                          const QVariant &value = QVariant());

    // Add a "where" clause comparing two columns to the query.
    QueryBuilder &whereColumn(const QString &first, const QString &op = "",
                              const QString &second = "", const QString &boolean = "and");
    // Add an "or where" clause comparing two columns to the query.
    QueryBuilder &orWhereColumn(const QString &first, const QString &op = "",
                              const QString &second = "");

    // Add a "where in" clause to the query.
    QueryBuilder &whereIn(const QString &column, const QVariant &value = QVariant(),
                          const QString &boolean = "and");
    // Add an "or where in" clause to the query.
    QueryBuilder &orWhereIn(const QString &column, const QVariant &value = QVariant());
    // Add a "where not in" clause to the query.
    QueryBuilder &whereNotIn(const QString &column, const QVariant &value = QVariant(),
                             const QString boolean = "and");
    // Add an "or where not in" clause to the query.
    QueryBuilder &orWhereNotIn(const QString &column, const QVariant &value = QVariant());

    // Add a "where null" clause to the query.
    QueryBuilder &whereNull(const QString &column, const QString &boolean = "and");
    // Add an "or where null" clause to the query.
    QueryBuilder &orWhereNull(const QString &column);
    // Add a "where not null" clause to the query.
    QueryBuilder &whereNotNull(const QString &column, const QString boolean = "and");
    // Add an "or where not null" clause to the query.
    QueryBuilder &orWhereNotNull(const QString &column);

    // Add a where between statement to the query.
    QueryBuilder &whereBetween(const QString &column, const QVariant &value,
                               const QString boolean = "and", bool negate = false);
    // Add an or where between statement to the query.
    QueryBuilder &orWhereBetween(const QString &column, const QVariant &value);
    // Add a where not between statement to the query.
    QueryBuilder &whereNotBetween(const QString &column, const QVariant &value,
                                  const QString boolean = "and");
    // Add an or where not between statement to the query.
    QueryBuilder &orWhereNotBetween(const QString &column, const QVariant &value);

    // Add a "where date" statement to the query.
    QueryBuilder &whereDate(const QString &column, const QString &op = "",
                            const QVariant &value = QVariant(), const QString boolean = "and");
    // Add an "or where date" statement to the query.
    QueryBuilder &orWhereDate(const QString &column, const QString &op = "",
                            const QVariant &value = QVariant());
    // Add a "where time" statement to the query.
    QueryBuilder &whereTime(const QString &column, const QString &op = "",
                            const QVariant &value = QVariant(), const QString boolean = "and");
    // Add an "or where time" statement to the query.
    QueryBuilder &orWhereTime(const QString &column, const QString &op = "",
                              const QVariant &value = QVariant());
    // Add a "where day" statement to the query.
    QueryBuilder &whereDay(const QString &column, const QString &op = "",
                            const QVariant &value = QVariant(), const QString boolean = "and");
    QueryBuilder &orWhereDay(const QString &column, const QString &op = "",
                           const QVariant &value = QVariant());
    // Add a "where month" statement to the query.
    QueryBuilder &whereMonth(const QString &column, const QString &op = "",
                           const QVariant &value = QVariant(), const QString boolean = "and");
    QueryBuilder &orWhereMonth(const QString &column, const QString &op = "",
                             const QVariant &value = QVariant());
    // Add a "where year" statement to the query.
    QueryBuilder &whereYear(const QString &column, const QString &op = "",
                             const QVariant &value = QVariant(), const QString boolean = "and");
    QueryBuilder &orWhereYear(const QString &column, const QString &op = "",
                               const QVariant &value = QVariant());

    // Add a where exists/not exists clause to the query.
    QueryBuilder &whereExists(std::function<void(QueryBuilder *)> callback, const QString &boolean = "and");
    QueryBuilder &orWhereExists(std::function<void(QueryBuilder *)> callback);
    QueryBuilder &whereNotExists(std::function<void(QueryBuilder *)> callback, const QString &boolean = "and");
    QueryBuilder &orWhereNotExists(std::function<void(QueryBuilder *)> callback);

    // Adds a where/or where condition using row values.
    QueryBuilder &whereRowValues(const QStringList &columns, const QString &op,
                                 const QVariantList &values, const QString &boolean = "and");
    QueryBuilder &orWhereRowValues(const QStringList &columns, const QString &op,
                                 const QVariantList &values);

    // Add a "group by" clause to the query.
    QueryBuilder &groupBy(const QStringList &groups);
    QueryBuilder &groupBy(const QString &groups);

    // Add a "having/no having/having between" clause to the query.
    QueryBuilder &having(const QString &column, const QString &op = "",
                         const QVariant &value = QVariant(), const QString boolean = "and");
    QueryBuilder &orHaving(const QString &column, const QString &op = "",
                           const QVariant &value = QVariant());
    QueryBuilder &havingBetween(const QString &column, const QVariant &value,
                                const QString boolean = "and", bool negate = false);
    QueryBuilder &orHavingBetween(const QString &column, const QVariant &value);
    QueryBuilder &havingNotBetween(const QString &column, const QVariant &value, const QString boolean = "and");
    QueryBuilder &orHavingNotBetween(const QString &column, const QVariant &value);

    // Add an "order by" clause to the query. direction: asc/desc
    QueryBuilder &orderBy(const QString &column, const QString &direction = "asc");
    QueryBuilder &orderByDesc(const QString &column);
    QueryBuilder &latest(const QString &column = "created_at");
    QueryBuilder &oldest(const QString &column = "created_at");

    // Set the "offset/limit" value of the query.
    QueryBuilder &offset(int value);
    QueryBuilder &limit(int value);

    // Set the limit and offset for a given page.
    QueryBuilder &forPage(int page, int perPage = 15);

    // Add a union statement to the query.
    QueryBuilder &unionAt(QueryBuilder *query, bool all = false);
    QueryBuilder &unionAll(QueryBuilder *query);

    int aggregate(const QString &function, const QString &column = "*");
    int aggregate(const QString &function, const QStringList &columns = {"*"});
    int count(const QString &columns = "*");
    int min(const QString &column);
    int max(const QString &column);
    int sum(const QString &column);
    int avg(const QString &column);
    int average(const QString &column);

private:
    void removeClause(int type);
    void addClause(int type, Clause *cluase);
};

#endif // QUERYBUILDER_H
