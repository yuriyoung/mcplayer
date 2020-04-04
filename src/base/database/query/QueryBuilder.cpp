#include "QueryBuilder.h"
#include "Grammar.h"
#include "Clause.h"

#include <QMap>
#include <QDate>
#include <QDateTime>
#include <QLoggingCategory>

static const QStringList ClauseOperators =
{
    "=", "<", ">", "<=", ">=", "<>", "!=", "<=>",
    "like", "like binary", "not like", "ilike",
    "&", "|", "^", "<<", ">>",
    "rlike", "regexp", "not regexp",
    "~", "~*", "!~", "!~*", "similar to",
    "not similar to", "not ilike", "~~*", "!~~*",
};

static bool invalidOperator(const QString &op)
{
    return !ClauseOperators.contains(op.toLower());
}

static bool invalidOperatorAndValue(const QString &op, const QVariant &value)
{
    QString lowerOp = op.toLower();
    QStringList ops{"=", "<>", "!="};
    return value.isNull() && ClauseOperators.contains(lowerOp)
            && !ops.contains(lowerOp);
}

class QueryBuilderPrivate
{
    Q_DECLARE_PUBLIC(QueryBuilder)
public:
    QueryBuilderPrivate(QueryBuilder *q) : q_ptr(q) {}

    void setAggregate(const QString &funciton, const QString &columns)
    {
        isAggregated = true;
        AggregateClause *agg = new AggregateClause(q_ptr, funciton, columns);
        clauses[Clause::Aggregate] = {agg};

        if(clauses[Clause::GroupBy].isEmpty())
        {
            removeClause(Clause::Order);
            removeBindings(QueryBuilder::OrderBinding);
        }
    }

    void removeClause(Clause::ClauseType type)
    {
        qDeleteAll(clauses[type]);
        clauses.remove(type);
    }

    void removeBindings(QueryBuilder::BindingType type)
    {
        bindings.remove(type);
    }

    QueryBuilder *q_ptr = nullptr;
    Connection *connection = nullptr;
    Grammar *grammar = nullptr;
    QueryBuilder::StatementType statementType;

    QMap<int, QList<Clause*>> clauses;
    QList<QueryBuilder *> subQueries; // joins
    QHash<int, QList<Record> > bindings; // The current query value bindings.

    QString from; // The table which the query is targeting.
    bool isDistincted = false;
    bool isAggregated = false;
};

QueryBuilder::QueryBuilder(Connection *connection)
    : d_ptr(new QueryBuilderPrivate(this))
{
    Q_D(QueryBuilder);
    d->connection = connection;
    d->grammar = connection->queryGrammar();
    d->grammar->setTablePrefix(connection->tablePrefix());
}

QueryBuilder::QueryBuilder(Connection *connection, Grammar *grammar)
    : d_ptr(new QueryBuilderPrivate(this))
{
    Q_D(QueryBuilder);
    d->connection = connection;
    d->grammar = grammar ? grammar : connection->queryGrammar();
}

QueryBuilder::~QueryBuilder()
{
    Q_D(const QueryBuilder);
    foreach (auto val, d->clauses) {
        qDeleteAll(val);
    }
    qDebug() << "QueryBuilder::~QueryBuilder()";
}

int QueryBuilder::statementType() const
{
    Q_D(const QueryBuilder);
    return d->statementType;
}

Connection *QueryBuilder::connection() const
{
    Q_D(const QueryBuilder);
    return d->connection;
}

Grammar *QueryBuilder::grammar() const
{
    Q_D(const QueryBuilder);
    return d->grammar;
}

QMap<int, QList<Clause *> > QueryBuilder::clauses() const
{
    Q_D(const QueryBuilder);
    return d->clauses;
}

QList<Clause *> QueryBuilder::clauses(int type) const
{
    Q_D(const QueryBuilder);
    return d->clauses.value(type);
}

void QueryBuilder::removeClause(int type)
{
    Q_D(QueryBuilder);
    d->removeClause(Clause::ClauseType(type));
}

QString QueryBuilder::toSql()
{
    Q_D(const QueryBuilder);
    return d->grammar->compile(this).join(" ");
}

QueryBuilder &QueryBuilder::setBindings(int bindingType, const QList<Record> &binding)
{
    Q_D(QueryBuilder);
    if(bindingType < InserBinding || bindingType > UnionBinding)
    {
        qWarning() << "Invalid binding type: " << bindingType;
        return *this;
    }

    d->bindings[bindingType] = binding;

    return *this;
}

/*!
 * \brief FROM => ["value1", "value2"]
 * \param value
 * \param type
 * \return
 */
QueryBuilder &QueryBuilder::addBinding(int bindingType, const Record &value)
{
    Q_D(QueryBuilder);
    d->bindings[bindingType].append(value);

//    QVariantMap::const_iterator it = value.constBegin();
//    while (it != value.constEnd())
//    {
//        // If there is already an item with the key,
//        // that item's value is replaced with value.
//        d->bindings[bindingType].insert(it.key(), it.value());
//        ++it;
//    }

    return *this;
}

bool QueryBuilder::insert(const Record &value)
{
    Q_D(QueryBuilder);

    d->statementType = InsertStatement;
    this->setBindings(InserBinding, {value});
    QStringList statements = d->grammar->compile(this);
    QString query = statements.join("; ");
    qDebug() << query;

    // TODO: connectin insert

    return true;
}

bool QueryBuilder::insert(const QList<Record> &values)
{
    Q_D(QueryBuilder);

    if(values.isEmpty())
        return true;

    d->statementType = InsertStatement;
    this->setBindings(InserBinding, values);
    QStringList statements = d->grammar->compile(this);
    qDebug() << statements.join("; ");

    // TODO: connection insert

    return true;
}

qint64 QueryBuilder::update(const Record &value)
{
    Q_D(QueryBuilder);

    d->statementType = UpdateStatement;
    // TODO: prepare bindings [except select and join clause?]
    this->setBindings(UpdateBinding, {value});
    QStringList statements = d->grammar->compile(this);

    // TODO: connection update

    return 0;
}

QueryBuilder &QueryBuilder::select(const QString &columns)
{
    Q_D(QueryBuilder);
    d->statementType = SelectStatement;
    ColumnClause *col = new ColumnClause(this, columns);
    d->clauses[Clause::Column] = {col};

    return *this;
}

QueryBuilder &QueryBuilder::select(const QStringList &columns)
{
    Q_D(QueryBuilder);
    d->statementType = SelectStatement;
    // TODO: any column of columns is queryable
    ColumnClause *col = new ColumnClause(this, columns);
    d->clauses[Clause::Column] = {col};

    return *this;
}

QueryBuilder &QueryBuilder::selectRaw(const QString &expression, const QVariantMap &bindings)
{
    Q_D(QueryBuilder);

    return *this;
}

/*!
 * \brief SELECT col1, (SELECT * FROM subtable ) AS subName, ... FROM table
 * \param query
 * \param as
 * \return
 */
QueryBuilder &QueryBuilder::selectSub(QueryBuilder *query, const QString &as)
{
    Q_D(QueryBuilder);

    // get sub select sql and bindings and
    // insert a sub select with as alias into columns

    // add sub select columns
//    if(as.isEmpty())
//    {
//        d->columns << QString("%1").arg(query->toSql());
//    }
//    else
//    {
//        d->columns << QString("(%1) as %2").arg(query->toSql()).arg(d->grammar->wrap(as));
//    }

    // add bindings
    auto bindgins = query->bindings();
//    QStringList columnBindings = bindgins[SelectBinding];
//    foreach (auto value, columnBindings)
//    {
//        this->addBinding(SelectBinding, value);
//    }

    return *this;
}

QueryBuilder &QueryBuilder::addSelect(const QString &column)
{
    Q_D(QueryBuilder);

    return *this;
}

QueryBuilder &QueryBuilder::from(const QString &table)
{
    Q_D(QueryBuilder);
    FromClause *fc = new FromClause(table);
    d->clauses[Clause::From] = {fc};
    d->from = table;

    return *this;
}

/**
 * @brief SELECT * FROM (SELECT * FROM subtable {bindins}) as subName
 * @param query
 * @param as
 * @return
 */
QueryBuilder &QueryBuilder::fromSub(QueryBuilder *query, const QString &as)
{
    Q_D(QueryBuilder);
    if(as.isEmpty())
    {
        d->from = QString("%1").arg(query->toSql());
    }
    else
    {
        d->from = QString("(%1) as %2").arg(query->toSql()).arg(d->grammar->wrap(as));
    }

    auto bindgins = query->bindings();
//    QStringList fromBindings = bindgins[FromBinding];
//    foreach (auto value, fromBindings)
//    {
//        this->addBinding(FromBinding, value);
//    }

    return *this;
}

QueryBuilder &QueryBuilder::distinct()
{
    Q_D(QueryBuilder);
    d->isDistincted = true;

    return *this;
}

QSqlQuery QueryBuilder::get(const QString &columns)
{
    Q_D(QueryBuilder);

    QList<Clause *> orignal = d->clauses.value(Clause::Column);
    if(d->clauses[Clause::Column].isEmpty())
    {
        ColumnClause *col = new ColumnClause(this, columns);
        d->clauses[Clause::Column] = {col};
    }

    // TODO: do execute query
    QSqlQuery query = d->connection->select(this->toSql());

    d->clauses[Clause::Column] = orignal;

    return query;
}

QSqlQuery QueryBuilder::get(const QStringList &columns)
{
    return this->get(columns.join(", "));
}

bool QueryBuilder::exists()
{
    // TODO: impl

    return false;
}

QueryBuilder &QueryBuilder::join(const QString &table, const QString &first,
                                 const QString &op, const QString &second,
                                 const QString &type, bool where)
{
    Q_D(QueryBuilder);

    // TODO: IMPL
    JoinClause *join = new JoinClause(this, type, table);
    where ? join->where(first, op, second) : join->on(first, op, second);
    d->clauses[Clause::Join].append(join);

    return *this;
}

QueryBuilder &QueryBuilder::where(const QString &column, const QString &op,
                                  const QVariant &value, const QString &boolean)
{
    Q_D(QueryBuilder);

    // TODO: value is a closure

    WhereClause *where = nullptr;
    if(invalidOperator(op))
    {
        where = new WhereClause(column, QVariant(op), QString("="), boolean);
    }
    else
    {
        where = new WhereClause(column, value, op, boolean);
    }
    where->setInvokableMethod("where");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhere(const QString &column, const QString &op, const QVariant &value)
{
    return this->where(column, op, value, "or");
}

QueryBuilder &QueryBuilder::whereColumn(const QString &first, const QString &op,
                                        const QString &second, const QString &boolean)
{
    Q_D(QueryBuilder);
    WhereClause *where = nullptr;
    if(invalidOperator(op))
    {
        where = new WhereClause(first, QString("="), op, boolean);
    }
    else
    {
        where = new WhereClause(first, op, second, boolean);
    }

    where->setInvokableMethod("whereColumn");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereColumn(const QString &first, const QString &op,
                                          const QString &second)
{
    return this->whereColumn(first, op, second, "or");
}

QueryBuilder &QueryBuilder::whereIn(const QString &column, const QVariant &value, const QString &boolean)
{
    Q_D(QueryBuilder);

    // TODO: create sub query if value is queryable and add bindings

    WhereClause *where = new WhereClause(column, value, boolean);
    where->setInvokableMethod("whereIn");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereIn(const QString &column, const QVariant &value)
{
    return this->whereIn(column, value, "or");
}

QueryBuilder &QueryBuilder::whereNotIn(const QString &column, const QVariant &value, const QString boolean)
{
    Q_D(QueryBuilder);
    WhereClause *where = new WhereClause(column, value, boolean);
    where->setInvokableMethod("whereNotIn");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereNotIn(const QString &column, const QVariant &value)
{
    return this->whereNotIn(column, value, "or");
}

QueryBuilder &QueryBuilder::whereNull(const QString &column, const QString &boolean)
{
    Q_D(QueryBuilder);
    WhereClause *where = new WhereClause(column, QVariant(), boolean);
    where->setInvokableMethod("whereNull");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereNull(const QString &column)
{
    return this->whereNull(column, "or");
}

QueryBuilder &QueryBuilder::whereNotNull(const QString &column, const QString boolean)
{
    Q_D(QueryBuilder);
    WhereClause *where = new WhereClause(column, QVariant(), boolean);
    where->setInvokableMethod("whereNotNull");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereNotNull(const QString &column)
{
    return this->whereNotNull(column, "or");
}

QueryBuilder &QueryBuilder::whereBetween(const QString &column, const QVariant &value,
                                         const QString boolean, bool negate)
{
    Q_D(QueryBuilder);
    WhereClause *where = new WhereClause(column, value, boolean, !negate);
    where->setInvokableMethod("whereBetween");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereBetween(const QString &column, const QVariant &value)
{
    return this->whereBetween(column, value, "or", false);
}

QueryBuilder &QueryBuilder::whereNotBetween(const QString &column, const QVariant &value,
                                            const QString boolean)
{
    return this->whereBetween(column, value, boolean, true);
}

QueryBuilder &QueryBuilder::orWhereNotBetween(const QString &column, const QVariant &value)
{
    return this->whereBetween(column, value, "or", true);
}

QueryBuilder &QueryBuilder::whereDate(const QString &column, const QString &op,
                                      const QVariant &value, const QString boolean)
{
    Q_D(QueryBuilder);
    WhereClause *where = nullptr;
    if(value.type() == QVariant::String)
    {
        QDate val = QDate::fromString(value.toString(), d->grammar->dateFormat());
        where = new WhereClause(column, val, op, boolean);
    }
    else
    {
        where = new WhereClause(column, value, op, boolean);
    }

    where->setInvokableMethod("whereDate");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereDate(const QString &column, const QString &op,
                                        const QVariant &value)
{
    return this->whereDate(column, op, value, "or");
}

QueryBuilder &QueryBuilder::whereTime(const QString &column, const QString &op,
                                      const QVariant &value, const QString boolean)
{
    Q_D(QueryBuilder);
    WhereClause *where = nullptr;
    if(value.type() == QVariant::String)
    {
        QDate val = QDate::fromString(value.toString(), d->grammar->datetimeFormat());
        where = new WhereClause(column, val, op, boolean);
    }
    else
    {
        where = new WhereClause(column, value, op, boolean);
    }

    where->setInvokableMethod("whereTime");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereTime(const QString &column,
                                        const QString &op, const QVariant &value)
{
    return this->whereTime(column, op, value, "or");
}

QueryBuilder &QueryBuilder::whereDay(const QString &column, const QString &op,
                                     const QVariant &value, const QString boolean)
{
    Q_D(QueryBuilder);
    WhereClause *where = nullptr;
    if(value.type() == QVariant::String)
    {
        QDate val = QDate::fromString(value.toString(), Qt::ISODate);
        where = new WhereClause(column, val.day(), op, boolean);
    }
    else
    {
        QVariant val = value;
        if(value.type() == QVariant::Date || value.type() == QVariant::DateTime)
            val = value.toDate().day();

        where = new WhereClause(column, val, op, boolean);
    }

    where->setInvokableMethod("whereTime");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereDay(const QString &column, const QString &op, const QVariant &value)
{
    return this->whereDay(column, op, value, "or");
}

QueryBuilder &QueryBuilder::whereMonth(const QString &column, const QString &op,
                                       const QVariant &value, const QString boolean)
{
    Q_D(QueryBuilder);
    WhereClause *where = nullptr;
    if(value.type() == QVariant::String)
    {
        QDate val = QDate::fromString(value.toString(), Qt::ISODate);
        where = new WhereClause(column, val.month(), op, boolean);
    }
    else
    {
        QVariant val = value;
        if(value.type() == QVariant::Date || value.type() == QVariant::DateTime)
            val = value.toDate().month();

        where = new WhereClause(column, val, op, boolean);
    }

    where->setInvokableMethod("whereMonth");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereMonth(const QString &column, const QString &op,
                                         const QVariant &value)
{
    return this->whereMonth(column, op, value, "or");
}

QueryBuilder &QueryBuilder::whereYear(const QString &column, const QString &op, const QVariant &value, const QString boolean)
{
    Q_D(QueryBuilder);
    WhereClause *where = nullptr;
    if(value.type() == QVariant::String)
    {
        QDate val = QDate::fromString(value.toString(), Qt::ISODate);
        where = new WhereClause(column, val.year(), op, boolean);
    }
    else
    {
        QVariant val = value;
        if(value.type() == QVariant::Date || value.type() == QVariant::DateTime)
            val = value.toDate().year();

        where = new WhereClause(column, val, op, boolean);
    }

    where->setInvokableMethod("whereYear");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereYear(const QString &column, const QString &op,
                                        const QVariant &value)
{
    return this->whereYear(column, op, value, "or");
}

QueryBuilder &QueryBuilder::whereExists(std::function<void (QueryBuilder *)> callback,
                                        const QString &boolean)
{
    Q_D(QueryBuilder);
    QueryBuilder *query = new QueryBuilder(d->connection, d->grammar);
    callback(query);

    WhereClause *where = new WhereClause(query, boolean);
    where->setInvokableMethod("whereExists");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereExists(std::function<void (QueryBuilder *)> callback)
{
    return this->whereExists(callback, "or");
}

QueryBuilder &QueryBuilder::whereNotExists(std::function<void (QueryBuilder *)> callback, const QString &boolean)
{
    Q_D(QueryBuilder);
    QueryBuilder *query = new QueryBuilder(d->connection, d->grammar);
    callback(query);

    WhereClause *where = new WhereClause(query, boolean);
    where->setInvokableMethod("whereNotExists");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereNotExists(std::function<void (QueryBuilder *)> callback)
{
    return this->whereNotExists(callback, "or");
}

QueryBuilder &QueryBuilder::whereRowValues(const QStringList &columns, const QString &op,
                                           const QVariantList &values, const QString &boolean)
{
    Q_D(QueryBuilder);
    if(columns.count() != values.count())
    {
        // TODO: throw a invalid argument exception
        qWarning() << "The number of columns must match the number of values";
        return *this;
    }

    WhereClause *where = new WhereClause(columns.join(", "), values, op, boolean);
    where->setInvokableMethod("whereRowValues");
    d->clauses[Clause::Where].append(where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereRowValues(const QStringList &columns, const QString &op, const QVariantList &values)
{
    return this->whereRowValues(columns, op, values);
}

QueryBuilder &QueryBuilder::groupBy(const QStringList &groups)
{
    Q_D(QueryBuilder);
    GroupClause *group = new GroupClause(groups);
    d->clauses[Clause::GroupBy] = {group};

    return *this;
}

QueryBuilder &QueryBuilder::groupBy(const QString &groups)
{
    Q_D(QueryBuilder);
    GroupClause *group = new GroupClause(groups);
    d->clauses[Clause::GroupBy] = {group};

    return *this;
}

QueryBuilder &QueryBuilder::having(const QString &column, const QString &op,
                                   const QVariant &value, const QString boolean)
{
    Q_D(QueryBuilder);
    HavingClause *having = new HavingClause(column, value, op, boolean);
    d->clauses[Clause::Having] = {having};

    return *this;
}

QueryBuilder &QueryBuilder::orHaving(const QString &column, const QString &op,
                                     const QVariant &value)
{
    return this->having(column, op, value, "or");
}

QueryBuilder &QueryBuilder::havingBetween(const QString &column, const QVariant &value,
                                          const QString boolean, bool negate)
{
    Q_D(QueryBuilder);
    HavingClause *having = new HavingClause(column, value, boolean, !negate);
    d->clauses[Clause::Having] = {having};

    return *this;
}

QueryBuilder &QueryBuilder::orHavingBetween(const QString &column, const QVariant &value)
{
    return this->havingBetween(column, value, "or");
}

QueryBuilder &QueryBuilder::havingNotBetween(const QString &column, const QVariant &value,
                                             const QString boolean)
{
    Q_D(QueryBuilder);
    HavingClause *having = new HavingClause(column, value, boolean, false);
    d->clauses[Clause::Having] = {having};

    return *this;
}

QueryBuilder &QueryBuilder::orHavingNotBetween(const QString &column, const QVariant &value)
{
    return this->havingNotBetween(column, value, "or");
}

QueryBuilder &QueryBuilder::orderBy(const QString &column, const QString &direction)
{
    Q_D(QueryBuilder);
    QString sort = direction.toLower();
    if(QStringList({"asc", "desc"}).contains(sort))
        sort = "asc";

    OrderClause *order = new OrderClause(column, direction);

    Clause::ClauseType type = Clause::Order;
    if(!d->clauses[Clause::Union].isEmpty())
        type = Clause::UnionOrder;

    d->clauses[type] = {order};

    return *this;
}

QueryBuilder &QueryBuilder::orderByDesc(const QString &column)
{
    return this->orderBy(column, "desc");
}

QueryBuilder &QueryBuilder::latest(const QString &column)
{
    return this->orderBy(column, "desc");
}

QueryBuilder &QueryBuilder::oldest(const QString &column)
{
    return this->orderBy(column, "asc");
}

QueryBuilder &QueryBuilder::offset(int value)
{
    Q_D(QueryBuilder);
    OffsetClause *offset = new OffsetClause(value);
    d->clauses[Clause::Offset] = {offset};

    return *this;
}

QueryBuilder &QueryBuilder::limit(int value)
{
    Q_D(QueryBuilder);
    LimitClause *limit = new LimitClause(value);
    d->clauses[Clause::Limit] = {limit};

    return *this;
}

QueryBuilder &QueryBuilder::forPage(int page, int perPage)
{
    return this->offset((page - 1) * perPage).limit(perPage);
}

QueryBuilder &QueryBuilder::UnionAt(QueryBuilder *query, bool all)
{
    Q_D(QueryBuilder);
    UnionClause *_union = new UnionClause(query, all);
    d->clauses[Clause::Union] = {_union};

    return *this;
}

QueryBuilder &QueryBuilder::UnionAll(QueryBuilder *query)
{
    return this->UnionAt(query, true);
}

int QueryBuilder::aggregate(const QString &function, const QString &column)
{
    Q_D(QueryBuilder);
    if(d->clauses[Clause::Union].isEmpty())
    {
        d->removeClause(Clause::Column);
        d->removeBindings(QueryBuilder::SelectBinding);
    }

    d->setAggregate(function, column);

    // TODO: IMPL
    this->get(column);

    return 0;
}

int QueryBuilder::aggregate(const QString &function, const QStringList &columns)
{
    return this->aggregate(function, columns.join(", "));
}

int QueryBuilder::count(const QString &columns)
{
    return this->aggregate("count", columns);
}

int QueryBuilder::min(const QString &column)
{
    return this->aggregate("min", column);
}

int QueryBuilder::max(const QString &column)
{
    return this->aggregate("max", column);
}

int QueryBuilder::sum(const QString &column)
{
    return this->aggregate("sum", column);
}

int QueryBuilder::avg(const QString &column)
{
    return this->aggregate("avg", column);
}

int QueryBuilder::average(const QString &column)
{
    return this->avg(column);
}

#define DEFINED_MEMBER(name, type) \
    type QueryBuilder::name() const \
    { \
        Q_D(const QueryBuilder); \
        return d->name; \
    }

DEFINED_MEMBER(bindings, BindingsHash);
DEFINED_MEMBER(from, QString);
DEFINED_MEMBER(isDistincted, bool);
DEFINED_MEMBER(isAggregated, bool);

#undef DEFINED_MEMBER
