#include "Clause.h"
#include "QueryBuilder.h"
#include "QueryGrammar.h"

#include <QSharedData>
#include <QMetaMethod>
#include <QDebug>

namespace  {
    struct WhereClauseLookupNames : public QHash<int, QByteArray>
    {
        WhereClauseLookupNames()
        {
            (*this)[WhereClause::Base] = "where";
            (*this)[WhereClause::In] = "whereIn";
            (*this)[WhereClause::NotIn] = "whereNotIn";
            (*this)[WhereClause::InRaw] = "whereInRaw";
            (*this)[WhereClause::NotInRaw] = "whereNotInRaw";
            (*this)[WhereClause::Null] = "whereNull";
            (*this)[WhereClause::NotNull] = "whereNotNull";
            (*this)[WhereClause::Between] = "whereBetween";
            (*this)[WhereClause::NotBetween] = "whereNotBetween";
            (*this)[WhereClause::Nested] = "whereNested";
            (*this)[WhereClause::Column] = "whereColumn";
            (*this)[WhereClause::Date] = "whereDate";
            (*this)[WhereClause::Time] = "whereTime";
            (*this)[WhereClause::Day] = "whereDay";
            (*this)[WhereClause::Month] = "whereMonth";
            (*this)[WhereClause::Year] = "whereYear";
            (*this)[WhereClause::Exists] = "whereExists";
            (*this)[WhereClause::NotExists] = "whereNotExists";
            (*this)[WhereClause::RowValues] = "whereRowValues";
            (*this)[WhereClause::Sub] = "whereSub";
            (*this)[WhereClause::JsonBoolean] = "whereJsonBoolean";
            (*this)[WhereClause::JsonContains] = "whereJsonContains";
        }
    };
}

Q_GLOBAL_STATIC(WhereClauseLookupNames, g_whereLookupNames)

class ClausePrivate : public QSharedData
{
public:
    ClausePrivate() { }
    ClausePrivate(const ClausePrivate &other) : QSharedData(other) { }
    virtual ~ClausePrivate() {}

public:
    QueryBuilder *parent = nullptr;
    QStringList columns;
    Clause *following = nullptr;
};

class FromClausePrivate : public ClausePrivate
{
public:
    QString table;
    QVariantMap bindings;
};

class AggregateClausePrivate : public ClausePrivate
{
public:
    QString function;
};

class ColumnClausePrivate : public ClausePrivate
{
public:
    // TODO: bindings ?
    QueryBuilder subQuery;
    QString as;
};

class JoinClausePrivate : public ClausePrivate
{
public:
    QString table; // The table the join clause is joining to.
    JoinClause::Type type = JoinClause::Inner; // The type of join being performed.
    bool isWhere = false;
    QList<JoinClause *> joins;
    QSharedPointer<QueryBuilder> subQuery;
};

class WhereClausePrivate : public ClausePrivate
{
public:
    int type = WhereClause::Base;
    QString method;
    QString op;
    QString boolean;
//    QString first;
//    QString second;
    QVariant value;
    bool isLeading = false;
    QSharedPointer<QueryBuilder> subQuery;
};

class HavingClausePrivate : public ClausePrivate
{
public:
    bool raw = false;
    QVariant value;
    QString boolean;
    QString op;
    bool betweenOrNot = true; // default is between
    bool _between = false;
    bool isLeading = false;
};

class GroupClausePrivate : public ClausePrivate
{
public:

};

class OrderClausePrivate : public ClausePrivate
{
public:
    QString direction;
    bool isLeading = false;
};

class UnionClausePrivate : public ClausePrivate
{
public:
    bool all = false;
};

class LimitClausePrivate : public ClausePrivate
{
public:
    int limit = -1;
};

class OffsetClausePrivate : public ClausePrivate
{
public:
    int offset = -1;
};

/*
 * ***********************************************
 * interfaces implementation
 * ***********************************************
 */
Clause::Clause(QueryBuilder *parent)
    : Clause(*new ClausePrivate, parent)
{

}

Clause::Clause(ClausePrivate &dd, QueryBuilder *parent)
    : d_ptr(&dd)
{
    d_ptr->parent = parent;
}

Clause::~Clause()
{
    qDebug() << "Clause::~Clause()";
}

void Clause::setColumns(const QString &columns)
{
    Q_UNUSED(columns)
    QStringList list = columns.split(",", QString::SkipEmptyParts);
    this->setColumns(list);
}

void Clause::setColumns(const QStringList &columns)
{
    Q_D(Clause);
//    if(columns.isEmpty())
//    {
//        d->columns = QStringList("*");
//        return ;
//    }

    d->columns.clear();
    foreach (auto &val, columns)
        d->columns << val.trimmed();
}

QStringList Clause::columns() const
{
    Q_D(const Clause);
    return d->columns;
}

QueryBuilder *Clause::query() const
{
    Q_D(const Clause);
    return d->parent;
}

/**
 * @brief FromClause::FromClause
 * @param table
 */
FromClause::FromClause(const QString &table, const QString &as,
                       const QVariantMap &bindings)
    : Clause(*new FromClausePrivate)
{
    Q_D(FromClause);
    d->table = as.isEmpty() ? table : table + " as " + as;
    d->bindings = bindings;
}

QString FromClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_UNUSED(leading)
    return grammar->clauseFrom(this);
}

QString FromClause::table() const
{
    Q_D(const FromClause);
    return d->table;
}

QVariantMap FromClause::bindings() const
{
    Q_D(const FromClause);
    return d->bindings;
}

/**
 * @brief AggregateClause::AggregateClause
 * @param function
 */
AggregateClause::AggregateClause(QueryBuilder *parent,
                                 const QString &function, const QString &columns)
    : Clause(*new AggregateClausePrivate(), parent)
{
    Q_D(AggregateClause);
    d->function = function;
    setColumns(columns);
}

AggregateClause::AggregateClause(QueryBuilder *parent,
                                 const QString &function, const QStringList &columns)
    : Clause(*new AggregateClausePrivate(), parent)
{
    Q_D(AggregateClause);
    d->function = function;
    setColumns(columns);
}

QString AggregateClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_UNUSED(leading)
    return grammar->clauseAggregate(this);
}

QString AggregateClause::function() const
{
    Q_D(const AggregateClause);
    return d->function;
}

/**
 * @brief ColumnClause::ColumnClause
 * @param columns
 */
ColumnClause::ColumnClause(const QString &columns, QueryBuilder *parent)
    : Clause(*new ColumnClausePrivate, parent)
{
    setColumns(columns);
}

ColumnClause::ColumnClause(const QStringList &columns, QueryBuilder *parent)
    : Clause(*new ColumnClausePrivate, parent)
{
    setColumns(columns);
}

ColumnClause::ColumnClause(const QueryBuilder &subQuery, const QString &as,
                           QueryBuilder *parent)
    : Clause(*new ColumnClausePrivate, parent)
{
    Q_D(ColumnClause);
    d->subQuery = subQuery;
    d->as = as;
}

QString ColumnClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_UNUSED(leading)
    return grammar->clauseColumn(this);
}

/**
 * @brief JoinClause::JoinClause
 * @param query
 * @param type
 * @param table
 */
JoinClause::JoinClause(QueryBuilder *parent, JoinClause::Type type,
                       const QString &table, bool where)
    : Clause(*new JoinClausePrivate, parent)
{
    Q_D(JoinClause);
    d->type = type;
    d->table = table;
    d->isWhere = where;
    d->subQuery = QSharedPointer<QueryBuilder>::create(parent->connection());
}

JoinClause::JoinClause(QueryBuilder *parent, const QString &type,
                       const QString &table, bool where)
    : Clause(*new JoinClausePrivate(), parent)
{
    Q_D(JoinClause);
    d->type = JoinClause::Type(QStringList({"inner", "left", "right", "cross"}).indexOf(type.toLower()));
    d->table = table;
    d->isWhere = where;
    d->subQuery = QSharedPointer<QueryBuilder>::create(parent->connection());
}

QString JoinClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_UNUSED(leading)
    return grammar->clauseJoin(this);
}

QueryBuilder &JoinClause::on(const QString &first, const QString &op,
                       const QString &second, const QString &boolean)
{
    Q_D(JoinClause);

    // TODO: (remove?) the table name provided by the user?
    const QString one = d->parent->table() + "." + first;
    const QString two = d->table + "." + second;

    return d->subQuery->whereColumn(one, op, two, boolean);
}

QueryBuilder &JoinClause::on(std::function<void (const QueryBuilder &)> column,
                             const QString &boolean)
{
    Q_D(JoinClause);
    return d->subQuery->where(column, boolean);
}

QueryBuilder &JoinClause::orOn(const QString &first, const QString &op,
                         const QString &second)
{
    return this->on(first, op, second, "or");
}

QueryBuilder &JoinClause::where(const QString &first, const QString &op,
                          const QString &second, const QString &boolean)
{
    Q_D(JoinClause);
    return d->subQuery->where(first, op, second, boolean);
}

QSharedPointer<QueryBuilder> JoinClause::subQuery() const
{
    Q_D(const JoinClause);
    return d->subQuery;
}

QString JoinClause::table() const
{
    Q_D(const JoinClause);
    return d->table;
}

QString JoinClause::joinType() const
{
    Q_D(const JoinClause);
    switch (d->type)
    {
    case Inner: return "inner";
    case Left: return "left";
    case Right: return "right";
    case Cross: return "cross";
    }

    return "inner";
}

bool JoinClause::isWhere() const
{
    Q_D(const JoinClause);
    return d->isWhere;
}

QList<JoinClause *> JoinClause::joins() const
{
    Q_D(const JoinClause);
    return d->joins;
}

/**
 * @brief WhereClause::WhereClause
 */
WhereClause::WhereClause(WhereClause::Type type,
                         const QString &column,
                         const QString &op,
                         const QueryBuilder &subQuery,
                         const QString &boolean)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    d->type = type;
    d->op = op;
    d->subQuery = QSharedPointer<QueryBuilder>::create(subQuery);
    d->boolean = boolean;
    setColumns(column);
}

WhereClause::WhereClause(Type type,
                         const QSharedPointer<QueryBuilder> &subQuery,
                         const QString &boolean)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    d->type = type;
    d->subQuery = subQuery;
    d->boolean = boolean;
}

WhereClause::WhereClause(WhereClause::Type type,
                         const QueryBuilder &subQuery,
                         const QString &boolean)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    d->type = type;
    d->subQuery = QSharedPointer<QueryBuilder>::create(subQuery);
    d->boolean = boolean;
}

WhereClause::WhereClause(WhereClause::Type type, const QString &column,
                         const QString &op, const QVariant &value,
                         const QString &boolean)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    d->type = type;
    d->value = value;
    d->op = op;
    d->boolean = boolean;
    setColumns(column);
}

WhereClause::WhereClause(WhereClause::Type type, const QString &column,
                         const QVariant &value, const QString &boolean)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    d->type = type;
    d->value = value;
    d->boolean = boolean;
    setColumns(column);
}

/*
WhereClause::WhereClause(QSharedPointer<QueryBuilder> query, const QString &boolean)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    d->query = query.data();
    d->boolean = boolean;
    d->op = "=";
}

WhereClause::WhereClause(const QString &column, const QVariant &value,
                         const QString &op, const QString &boolean)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    d->value = value;
    d->op = op;
    d->boolean = boolean;
    setColumns(column);
}

WhereClause::WhereClause(const QString &column, const QVariant &value,
                         const QString &boolean)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    d->value = value;
    d->boolean = boolean;
    setColumns(column);
}

WhereClause::WhereClause(const QString &first, const QString &op,
                         const QString &second, const QString &boolean)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    d->first = first;
    d->op = op;
    d->second = second;
    d->boolean = boolean;
}

WhereClause::WhereClause(const QString &column, const QVariant &value,
                         const QString &boolean, bool betweenOrNotBetween)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    setColumns(column);
    d->value = value;
    d->boolean = boolean;
    d->betweenOrNotBetween = betweenOrNotBetween;
}
*/

QString WhereClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_D(WhereClause);
    d->isLeading = leading;
    return grammar->clauseWhere(this);
}

void WhereClause::setParentQuery(QueryBuilder *parent)
{
    Q_D(WhereClause);
    d->parent = parent;
}

QString WhereClause::whereMethod() const
{
    Q_D(const WhereClause);
    return g_whereLookupNames->value(d->type);
}

int WhereClause::type() const
{
    Q_D(const WhereClause);
    return d->type;
}

bool WhereClause::leading() const
{
    Q_D(const WhereClause);
    return d->isLeading;
}

QVariant WhereClause::value() const
{
    Q_D(const WhereClause);
    return d->value;
}

QString WhereClause::op() const
{
    Q_D(const WhereClause);
    return d->op;
}

QString WhereClause::boolean()
{
    Q_D(const WhereClause);
    return d->boolean;
}

QString WhereClause::first() const
{
    Q_D(const WhereClause);
    return d->columns.first();
}

QString WhereClause::second() const
{
    Q_D(const WhereClause);
    return d->value.toString();
}

QSharedPointer<QueryBuilder> WhereClause::subQuery() const
{
    Q_D(const WhereClause);
    return d->subQuery;
}

/**
 * @brief HavingClause::HavingClause
 * @param column
 * @param value
 * @param op
 * @param boolean
 */
HavingClause::HavingClause(const QString &column, const QVariant &value,
                           const QString &op, const QString &boolean)
    : Clause(*new HavingClausePrivate)
{
    Q_D(HavingClause);
    setColumns(column);
    d->value = value;
    d->boolean = boolean;
    d->op = op;
}

HavingClause::HavingClause(const QString &column, const QVariant &value,
                           const QString &boolean, bool betweenOrNotBetween)
    : Clause(*new HavingClausePrivate)
{
    Q_D(HavingClause);
    setColumns(column);
    d->value = value;
    d->boolean = boolean;
    d->_between = true;
    d->betweenOrNot = betweenOrNotBetween;
}

QString HavingClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_D(HavingClause);
    d->isLeading = leading;

    if(d->_between)
    {
         return grammar->clauseHavingBetween(this);
    }
    else
    {
        return grammar->clauseHaving(this);
    }
}

QVariant HavingClause::value() const
{
    Q_D(const HavingClause);
    return d->value;
}

QString HavingClause::boolean() const
{
    Q_D(const HavingClause);
    return d->boolean;
}

QString HavingClause::op() const
{
    Q_D(const HavingClause);
    return d->op;
}

bool HavingClause::betweenOrNot() const
{
    Q_D(const HavingClause);
    return d->betweenOrNot;
}

bool HavingClause::leading() const
{
    Q_D(const HavingClause);
    return d->isLeading;
}

/**
 * @brief GroupClause::GroupClause
 * @param columns
 */
GroupClause::GroupClause(const QString &columns)
    : Clause(*new GroupClausePrivate)
{
    setColumns(columns);
}

GroupClause::GroupClause(const QStringList &columns)
    : Clause(*new GroupClausePrivate)
{
    setColumns(columns);
}

QString GroupClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_UNUSED(leading)
    return grammar->clauseGroup(this);
}

/**
 * @brief OrderClause::OrderClause
 * @param columns
 * @param direction
 */
OrderClause::OrderClause(const QString &columns, const QString &direction)
    : Clause(*new OrderClausePrivate)
{
    Q_D(OrderClause);
    setColumns(columns);
    d->direction = direction;
}

OrderClause::OrderClause(const QStringList &columns, const QString &direction)
    : Clause(*new OrderClausePrivate)
{
    Q_D(OrderClause);
    setColumns(columns);
    d->direction = direction;
}

QString OrderClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_D(OrderClause);
    d->isLeading = leading;
    return grammar->clauseOrder(this);
}

QString OrderClause::direction() const
{
    Q_D(const OrderClause);
    return d->direction;
}

bool OrderClause::leading() const
{
    Q_D(const OrderClause);
    return d->isLeading;
}

/**
 * @brief UnionClause::UnionClause
 * @param query
 * @param all
 */
UnionClause::UnionClause(QueryBuilder *query, bool all)
    : Clause(*new UnionClausePrivate, query)
{
    Q_D(UnionClause);
    d->all = all;
}

QString UnionClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_UNUSED(leading)
    return grammar->clauseUnion(this);
}

bool UnionClause::all() const
{
    Q_D(const UnionClause);
    return d->all;
}

/**
 * @brief LimitClause::LimitClause
 * @param limit
 */
LimitClause::LimitClause(int limit)
    : Clause(*new LimitClausePrivate)
{
    Q_D(LimitClause);
    d->limit = qMax(0, limit);
}

QString LimitClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_UNUSED(leading)
    return grammar->clauseLimit(this);
}

int LimitClause::value() const
{
    Q_D(const LimitClause);
    return d->limit;
}

/**
 * @brief OffsetClause::OffsetClause
 * @param offset
 */
OffsetClause::OffsetClause(int offset)
    : Clause(*new OffsetClausePrivate)
{
    Q_D(OffsetClause);
    d->offset = qMax(0, offset);
}

QString OffsetClause::interpret(QueryGrammar *grammar, bool leading)
{
    Q_UNUSED(leading)
    return grammar->clauseOffset(this);
}

int OffsetClause::value() const
{
    Q_D(const OffsetClause);
    return d->offset;
}
