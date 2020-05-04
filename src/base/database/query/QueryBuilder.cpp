#include "QueryBuilder.h"
#include "Grammar.h"
#include "Clause.h"
#include "Connection.h"

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

/**
 * @brief The QueryBuilderPrivate class
 */
class QueryBuilderPrivate
{
    Q_DECLARE_PUBLIC(QueryBuilder)
public:
    QueryBuilderPrivate(QueryBuilder *query) : q_ptr(query) { }

    void setAggregate(const QString &funciton, const QString &columns)
    {
        Q_Q(QueryBuilder);
        isAggregated = true;
        AggregateClause *agg = new AggregateClause(q, funciton, columns);
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

    void setClause(Clause::ClauseType type, Clause *clause)
    {
        clauses[type] = {clause};
    }

    void setClause(Clause::ClauseType type, const QList<Clause*> &list)
    {
        clauses[type] = list;
    }

    void addClause(Clause::ClauseType type, Clause *clause)
    {
        clauses[type].append(clause);
    }

    bool hasClause(Clause::ClauseType type) const
    {
        return clauses.value(type).isEmpty();
    }

    void removeBindings(QueryBuilder::BindingType type)
    {
        bindings.remove(type);
    }

    QueryBuilder *whereNested(std::function<void(const QueryBuilder &)> callback,
                              const QString &boolean = "and")
    {
        Q_Q(QueryBuilder);
        QueryBuilder query(q->connection());
        query.from(table);
        callback(query);

        if(!query.clauses(Clause::Where).isEmpty())
        {
            this->addClause(Clause::Where, new WhereClause(WhereClause::Nested, query, boolean));
        }

        return q;
    }

    // a full sub-select to the query.
    QueryBuilder *whereSelect(const QString &column, const QString &op,
                              std::function<void(const QueryBuilder &)> select,
                              const QString &boolean = "and")
    {
        Q_Q(QueryBuilder);
        QueryBuilder query(q->connection());
        query.from(table);
        select(query);

        this->addClause(Clause::Where, new WhereClause(WhereClause::Sub, column, op, query, boolean));

        return q_ptr;
    }

    QueryBuilder *q_ptr = nullptr;
    Connection *connection = nullptr;
    Grammar *grammar = nullptr;
    QString table;
    bool isDistincted = false;
    bool isAggregated = false;

    QMap<int, QList<Clause*> > clauses;
    BindingsHash bindings;
};

/**
 * @brief QueryBuilder::QueryBuilder
 * @param connection
 */
QueryBuilder::QueryBuilder()
    : d_ptr(new QueryBuilderPrivate(this))
{

}

QueryBuilder::QueryBuilder(const Connection *connection)
    : d_ptr(new QueryBuilderPrivate(this))
{
    Q_D(QueryBuilder);
    d->connection = const_cast<Connection *>(connection);
    d->grammar = d->connection->queryGrammar().get();
}

QueryBuilder::QueryBuilder(const QueryBuilder &other)
    : d_ptr(new QueryBuilderPrivate(this))
{
    *d_ptr.data() = *other.d_ptr.data();
}

QueryBuilder::QueryBuilder(const QueryBuilder &&other)
    : d_ptr(new QueryBuilderPrivate(this))
{
    *d_ptr.data() = *other.d_ptr.data();
}

QueryBuilder &QueryBuilder::operator=(const QueryBuilder &other)
{
    *d_ptr.data() = *other.d_ptr.data();
    return *this;
}

QueryBuilder::~QueryBuilder()
{
    qDebug() << "QueryBuilder::~QueryBuilder()";
    Q_D(QueryBuilder);
    foreach (auto val, d->clauses) {
        qDeleteAll(val);
    }
    d->clauses.clear();
}

void QueryBuilder::setConnection(const Connection *connection)
{
    Q_D(QueryBuilder);
    d->connection = const_cast<Connection *>(connection);
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

void QueryBuilder::addClause(int type, Clause *cluase)
{
    Q_D(QueryBuilder);
    d->addClause(Clause::ClauseType(type), cluase);
}


QList<QVariantMap> QueryBuilder::bindings(QueryBuilder::BindingType type) const
{
    Q_D(const QueryBuilder);
    return d->bindings.value(type);
}

QueryBuilder &QueryBuilder::setBindings(int bindingType, const QList<QVariantMap> &binding)
{
    Q_D(QueryBuilder);
    if(bindingType < InsertBinding || bindingType > UnionBinding)
    {
        qWarning() << "Invalid binding type: " << bindingType;
        return *this;
    }

    d->bindings[bindingType] = binding;

    return *this;
}

QueryBuilder &QueryBuilder::addBinding(int bindingType, const QVariantMap &value)
{
    Q_D(QueryBuilder);
    if(bindingType < InsertBinding || bindingType > UnionBinding)
    {
        qWarning() << "Invalid binding type: " << bindingType;
        return *this;
    }

    int total = d->bindings[bindingType].size();
    if(total == 0)
    {
        d->bindings[bindingType].append(value);
        return *this;
    }

    // only one item in list
    QVariantMap::const_iterator it;
    for(int i = 0; i < total; ++i)
    {
        for(it = value.constBegin(); it != value.constEnd(); ++it)
        {
            // If there is already an item with the key,
            // that item's value is replaced with value.
            d->bindings[bindingType][i].insert(it.key(), it.value());
        }
    }

    return *this;
}

QString QueryBuilder::toSql() const
{
    Q_D(const QueryBuilder);
    return d->grammar->compile(const_cast<QueryBuilder *>(this), SelectStatement).join(" ");
}

bool QueryBuilder::exists()
{
    Q_D(QueryBuilder);
    QStringList statements = d->grammar->compile(this, ExistsStatement);
    QString query = statements.join("; ");
    QSqlQuery result = d->connection->select(query);

    return result.next() && result.value(0).toBool();
}

bool QueryBuilder::insert(const QVariantMap &value)
{
    Q_D(QueryBuilder);

    this->setBindings(InsertBinding, {value});
    QStringList statements = d->grammar->compile(this, InsertStatement);
    QString query = statements.join("; ");

    return d->connection->insert(query, value);
}

bool QueryBuilder::insert(const QList<QVariantMap> &values)
{
    Q_D(QueryBuilder);

    if(values.isEmpty())
        return true;

    this->setBindings(InsertBinding, values);
    QStringList statements = d->grammar->compile(this, InsertStatement);
    QString query = statements.join("; ");

    return d->connection->insert(query, values);
}

qint64 QueryBuilder::update(const QVariantMap &value)
{
    Q_D(QueryBuilder);

    this->addBinding(UpdateBinding, value); // TODO: binding sub query also
    QStringList statements = d->grammar->compile(this, UpdateStatement);
    QString query = statements.join("; ");

    return d->connection->update(query, value);
}

bool QueryBuilder::updateOrInsert(const QVariantMap &attribute, const QVariantMap &value)
{
    if(!this->where(attribute).exists())
    {
        QVariantMap merged = attribute;
        QMapIterator<QString, QVariant> it(value);
        while(it.hasNext())
        {
            it.next();
            merged.insert(it.key(), it.value());
        }
        return this->insert(merged);
    }

    if(value.isEmpty())
    {
        qWarning() << "No value to update";
        return true;
    }

    return this->limit(1).update(value);
}

int QueryBuilder::destroy(const QVariant &id)
{
    Q_D(QueryBuilder);
    if(id.isValid())
    {
        this->where(d->table + ".id", "=", id);
    }

    QString sql = d->grammar->compile(this, DeleteStatement).join("; ");
    QVariantMap binding =  this->bindings(DeleteBinding).isEmpty()
            ? QVariantMap() : this->bindings(DeleteBinding).first();
    return this->connection()->del(sql, binding);
}

QueryBuilder &QueryBuilder::select(const QString &columns)
{
    Q_D(QueryBuilder);
    ColumnClause *col = new ColumnClause(columns, this);
    d->setClause(Clause::Column, col);

    return *this;
}

QueryBuilder &QueryBuilder::select(const QStringList &columns)
{
    Q_D(QueryBuilder);

    // TODO: any column of columns is queryable (sub query)
    // selectSub(query, as)

    ColumnClause *col = new ColumnClause(columns, this);
    d->setClause(Clause::Column, col);

    return *this;
}

QueryBuilder &QueryBuilder::selectRaw(const QString &expression, const QVariantMap &bindings)
{
    this->addSelect(expression);

    if(!bindings.isEmpty())
        this->addBinding(SelectBinding, bindings);

    return *this;
}

/*!
 * \brief SELECT col1, (SELECT * FROM subtable ) AS subName, ... FROM table
 * \param query
 * \param as
 * \return
 */
QueryBuilder &QueryBuilder::selectSub(const QueryBuilder &query, const QString &as)
{
    Q_D(QueryBuilder);

    // get sub select sql and bindings and
    // insert a sub select with as alias into columns

    // add sub select columns
    if(as.isEmpty())
    {
        this->selectRaw(QString("%1").arg(query.toSql())/*, TODO: add select bindings */);
    }
    else
    {
        QString sql = QString("(%1) as %2").arg(query.toSql()).arg(d->grammar->wrap(as));
        this->selectRaw(sql/*, TODO: add select bindings */);
    }

    // add select bindings
    auto bindgins = query.bindings(SelectBinding);
    foreach (auto value, bindgins)
    {
        this->addBinding(SelectBinding, value);
    }

    return *this;
}

// TODO: addSelect(column, query builder)
QueryBuilder &QueryBuilder::addSelect(const QString &column)
{
    Q_D(QueryBuilder);

    d->addClause(Clause::Column, new ColumnClause(column, this));

    return *this;
}

QueryBuilder &QueryBuilder::from(const QString &table, const QString &as)
{
    Q_D(QueryBuilder);
    // remove this clause?
    FromClause *fc = new FromClause(table, as);
    d->setClause(Clause::From, fc);
    d->table = fc->table();

    return *this;
}

/**
 * @brief SELECT * FROM (SELECT * FROM subtable {bindins}) as subName
 * @param query
 * @param as
 * @return
 */
QueryBuilder &QueryBuilder::fromSub(const QueryBuilder &query, const QString &as)
{
    Q_D(QueryBuilder);

    d->table = QString("(%1) as %2").arg(query.toSql()).arg(d->grammar->wrap(as));

    auto bindgins = query.bindings(FromBinding);
    foreach (auto value, bindgins)
    {
        this->addBinding(FromBinding, value);
    }

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

    if(d->hasClause(Clause::Column))
    {
        d->setClause(Clause::Column, new ColumnClause(columns, this));
    }
    QSqlQuery query = d->connection->select(this->toSql());

    d->setClause(Clause::Column, orignal);

    return query;
}

QSqlQuery QueryBuilder::get(const QStringList &columns)
{
    return this->get(columns.join(", "));
}

QueryBuilder &QueryBuilder::join(const QString &table, const QString &first,
                                 const QString &op, const QString &second,
                                 const QString &type, bool where)
{
    Q_D(QueryBuilder);

    JoinClause *join = new JoinClause(this, type, table, where);
    where ? join->where(first, op, second) : join->on(first, op, second);
    d->addClause(Clause::Join, join);

    return *this;
}

QueryBuilder &QueryBuilder::joinWhere(const QString &table, const QString &first,
                                      const QString &op, const QString &second,
                                      const QString &type)
{
    return this->join(table, first, op, second, type, true);
}

QueryBuilder &QueryBuilder::joinSub(const QueryBuilder &query, const QString &as,
                                    const QString &first, const QString &op,
                                    const QString &second, const QString &type, bool where)
{
    Q_D(QueryBuilder);
    QString expression = QString("(%1) as %2").arg(query.toSql()).arg(d->grammar->wrap(as));
    auto bindgins = query.bindings(JoinBinding);
    foreach (auto value, bindgins)
    {
        this->addBinding(JoinBinding, value);
    }

    return this->join(expression, first, op, second, type, where);
}

QueryBuilder &QueryBuilder::where(const QString &column, const QVariant &op,
                                  const QVariant &value, const QString &boolean)
{
    Q_D(QueryBuilder);

    // TODO: value is a closure/lambda/subquery

    WhereClause *where = nullptr;
    if(invalidOperator(op.toString()))
    {
        where = new WhereClause(WhereClause::Base, column, QString("="), op, boolean);
    }
    else
    {
        where = new WhereClause(WhereClause::Base, column, op.toString(), value, boolean);
    }
    d->addClause(Clause::Where, where);

    return *this;
}

QueryBuilder &QueryBuilder::where(const QVariantMap &attributes, const QString &boolean)
{
    QVariantMap::const_iterator it = attributes.constBegin();
    for(; it != attributes.constEnd(); ++it)
    {
        if(it.value().canConvert(QVariant::Map))
            return where(it.value().toMap(), boolean);

        this->where(it.key(), "=", it.value(), boolean);
    }

    return *this;
}

QueryBuilder &QueryBuilder::where(std::function<void (const QueryBuilder &)> column,
                                  const QString &boolean)
{
    Q_D(QueryBuilder);
    return *d->whereNested(column, boolean);
}

QueryBuilder &QueryBuilder::where(const QString &column, const QString &op,
                                  std::function<void (const QueryBuilder &)> value,
                                  const QString &boolean)
{
    Q_D(QueryBuilder);
    return *d->whereSelect(column, op, value, boolean);
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
        where = new WhereClause(WhereClause::Column, QString("="), op, boolean);
    }
    else
    {
        where = new WhereClause(WhereClause::Column, first, op, second, boolean);
    }

    d->addClause(Clause::Where, where);

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
    WhereClause *where = new WhereClause(WhereClause::In, column, value, boolean);
    d->addClause(Clause::Where, where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereIn(const QString &column, const QVariant &value)
{
    return this->whereIn(column, value, "or");
}

QueryBuilder &QueryBuilder::whereNotIn(const QString &column, const QVariant &value, const QString boolean)
{
    Q_D(QueryBuilder);

    WhereClause *where = new WhereClause(WhereClause::NotIn, column, value, boolean);
    d->addClause(Clause::Where, where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereNotIn(const QString &column, const QVariant &value)
{
    return this->whereNotIn(column, value, "or");
}

QueryBuilder &QueryBuilder::whereNull(const QString &column, const QString &boolean)
{
    Q_D(QueryBuilder);

    WhereClause *where = new WhereClause(WhereClause::Null, column, QVariant(), boolean);
    d->addClause(Clause::Where, where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereNull(const QString &column)
{
    return this->whereNull(column, "or");
}

QueryBuilder &QueryBuilder::whereNotNull(const QString &column, const QString boolean)
{
    Q_D(QueryBuilder);

    WhereClause *where = new WhereClause(WhereClause::NotNull, column, QVariant(), boolean);
    d->addClause(Clause::Where, where);

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

    WhereClause *where = new WhereClause(negate ? WhereClause::NotBetween : WhereClause::Between,
                                         column, value, boolean);
    d->addClause(Clause::Where, where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereBetween(const QString &column, const QVariant &value)
{
    return this->whereBetween(column, value, "or", false);
}

QueryBuilder &QueryBuilder::whereNotBetween(const QString &column, const QVariant &value,
                                            const QString boolean)
{
    Q_D(QueryBuilder);
    WhereClause *where = new WhereClause(WhereClause::NotBetween, column, value, boolean);
    d->addClause(Clause::Where, where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereNotBetween(const QString &column, const QVariant &value)
{
    return this->whereNotBetween(column, value, "or");
}

QueryBuilder &QueryBuilder::whereDate(const QString &column, const QString &op,
                                      const QVariant &value, const QString boolean)
{
    Q_D(QueryBuilder);

    WhereClause *where = nullptr;
    if(value.type() == QVariant::String)
    {
        QDate val = QDate::fromString(value.toString(), d->grammar->dateFormat());
        where = new WhereClause(WhereClause::Date, column, op, val, boolean);
    }
    else
    {
        where = new WhereClause(WhereClause::Date, column, op, value, boolean);
    }

    d->addClause(Clause::Where, where);

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
        where = new WhereClause(WhereClause::Date, column, op, val, boolean);
    }
    else
    {
        where = new WhereClause(WhereClause::Date, column, op, value, boolean);
    }

    d->addClause(Clause::Where, where);

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
        where = new WhereClause(WhereClause::Day, column, op, val.day(), boolean);
    }
    else
    {
        QVariant val = value;
        if(value.type() == QVariant::Date || value.type() == QVariant::DateTime)
            val = value.toDate().day();

        where = new WhereClause(WhereClause::Day, column, op, val, boolean);
    }

    d->addClause(Clause::Where, where);

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
        where = new WhereClause(WhereClause::Month, column, op, val.month(), boolean);
    }
    else
    {
        QVariant val = value;
        if(value.type() == QVariant::Date || value.type() == QVariant::DateTime)
            val = value.toDate().month();

        where = new WhereClause(WhereClause::Month, column, op, val, boolean);
    }

    d->addClause(Clause::Where, where);

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
        where = new WhereClause(WhereClause::Year, column, op, val.year(), boolean);
    }
    else
    {
        QVariant val = value;
        if(value.type() == QVariant::Date || value.type() == QVariant::DateTime)
            val = value.toDate().year();

        where = new WhereClause(WhereClause::Year, column, op, val, boolean);
    }

    d->addClause(Clause::Where, where);

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

    QSharedPointer<QueryBuilder> query(new QueryBuilder(d->connection));
    callback(query.get());

    WhereClause *where = new WhereClause(WhereClause::Exists, query, boolean);
    d->addClause(Clause::Where, where);

    return *this;
}

QueryBuilder &QueryBuilder::orWhereExists(std::function<void (QueryBuilder *)> callback)
{
    return this->whereExists(callback, "or");
}

QueryBuilder &QueryBuilder::whereNotExists(std::function<void (QueryBuilder *)> callback, const QString &boolean)
{
    Q_D(QueryBuilder);

    QSharedPointer<QueryBuilder> query(new QueryBuilder(d->connection));
    callback(query.data());

    WhereClause *where = new WhereClause(WhereClause::Exists, query, boolean);
    d->addClause(Clause::Where, where);

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

    WhereClause *where = new WhereClause(WhereClause::RowValues, columns.join(", "),
                                         op, values, boolean);
    d->addClause(Clause::Where, where);

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
    d->setClause(Clause::GroupBy, group);

    return *this;
}

QueryBuilder &QueryBuilder::groupBy(const QString &groups)
{
    Q_D(QueryBuilder);
    GroupClause *group = new GroupClause(groups);
    d->setClause(Clause::GroupBy, group);

    return *this;
}

QueryBuilder &QueryBuilder::having(const QString &column, const QString &op,
                                   const QVariant &value, const QString boolean)
{
    Q_D(QueryBuilder);
    HavingClause *having = new HavingClause(column, value, op, boolean);
    d->addClause(Clause::Having, having);

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
    d->addClause(Clause::Having, having);

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
    d->addClause(Clause::Having, having);

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
    if(!d->hasClause(Clause::Union))
        type = Clause::UnionOrder;

    d->setClause(type, order);

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
    d->setClause(Clause::Offset, offset);

    return *this;
}

QueryBuilder &QueryBuilder::limit(int value)
{
    Q_D(QueryBuilder);
    LimitClause *limit = new LimitClause(value);
    d->setClause(Clause::Limit, limit);

    return *this;
}

QueryBuilder &QueryBuilder::forPage(int page, int perPage)
{
    return this->offset((page - 1) * perPage).limit(perPage);
}

QueryBuilder &QueryBuilder::unionAt(QueryBuilder *query, bool all)
{
    Q_D(QueryBuilder);
    UnionClause *_union = new UnionClause(query, all);
    d->setClause(Clause::Union, _union);

    return *this;
}

QueryBuilder &QueryBuilder::unionAll(QueryBuilder *query)
{
    return this->unionAt(query, true);
}

int QueryBuilder::aggregate(const QString &function, const QString &column)
{
    Q_D(QueryBuilder);
    if(d->hasClause(Clause::Union))
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
DEFINED_MEMBER(table, QString);
DEFINED_MEMBER(isDistincted, bool);
DEFINED_MEMBER(isAggregated, bool);

#undef DEFINED_MEMBER
