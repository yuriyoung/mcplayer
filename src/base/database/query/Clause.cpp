#include "Clause.h"

#include <QSharedData>
#include <QMetaMethod>
#include <QDebug>

class FromClausePrivate : public ClausePrivate
{
public:
    QString table;
    QString as;
    QVariantMap bindings;
};

class AggregateClausePrivate : public ClausePrivate
{
public:
    QueryBuilder *query = nullptr;
    QString function;
};

class ColumnClausePrivate : public ClausePrivate
{
public:
    QueryBuilder *query = nullptr;
    // TODO: bindings ?
};

class JoinClausePrivate : public ClausePrivate
{
public:
    QString table; // The table the join clause is joining to.
    QString type; // The type of join being performed.
    QSharedPointer<QueryBuilder> query = nullptr;
    QList<JoinClause *> joins;
};

class WhereClausePrivate : public ClausePrivate
{
public:
    QString method;
    QString op;
    QString boolean;
    QString first;
    QString second;
    QVariant value;
    bool betweenOrNotBetween = false;
    QSharedPointer<QueryBuilder> query = nullptr;
//    QueryBuilder *query = nullptr;
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
};

class GroupClausePrivate : public ClausePrivate
{
public:

};

class OrderClausePrivate : public ClausePrivate
{
public:
    QString direction;
};

class UnionClausePrivate : public ClausePrivate
{
public:
    QueryBuilder *query = nullptr;
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
//template<typename P>
Clause::Clause() : Clause(*new ClausePrivate)
{

}

Clause::Clause(const QStringList &columns)
    : Clause(*new ClausePrivate(columns))
{

}

Clause::Clause(ClausePrivate &dd): d_ptr(&dd)
{

}

Clause::~Clause()
{
    qDebug() << "Clause::~Clause()";
}

void Clause::setBindings(const QVariantMap &bindings)
{
    Q_D(Clause);
    d->bindings = bindings;
}

QVariantMap Clause::bindings() const
{
    Q_D(const Clause);
    return d->bindings;
}

void Clause::setColumns(const QString &columns)
{
    QStringList list = columns.split(",", QString::SkipEmptyParts);
    this->setColumns(list);
}

void Clause::setColumns(const QStringList &columns)
{
    Q_D(Clause);
    if(columns.isEmpty())
    {
        d->columns = QStringList("*");
        return ;
    }

    d->columns.clear();
    foreach (auto &val, columns)
        d->columns << val.trimmed();
}

QStringList Clause::columns() const
{
    Q_D(const Clause);
    return d->columns;
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
    d->table = table;
    d->as = as;
    d->bindings = bindings;
}

FromClause::FromClause(const FromClause &other)
{
    d_ptr = other.d_ptr;
}

QString FromClause::interept(QueryGrammar *grammar)
{
    return grammar->clauseFrom(this);
}

QString FromClause::table() const
{
    Q_D(const FromClause);
    return d->table;
}

QString FromClause::as() const
{
    Q_D(const FromClause);
    return d->as;
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
AggregateClause::AggregateClause(QueryBuilder *query, const QString &function, const QString &columns)
    : Clause(*new AggregateClausePrivate())
{
    Q_D(AggregateClause);
    d->query = query;
    d->function = function;
    setColumns(columns);
}

AggregateClause::AggregateClause(QueryBuilder *query, const QString &function, const QStringList &columns)
    : Clause(*new AggregateClausePrivate())
{
    Q_D(AggregateClause);
    d->query = query;
    d->function = function;
    setColumns(columns);
}

QString AggregateClause::function() const
{
    Q_D(const AggregateClause);
    return d->function;
}

QueryBuilder *AggregateClause::query() const
{
    Q_D(const AggregateClause);
    return d->query;
}

/**
 * @brief ColumnClause::ColumnClause
 * @param columns
 */
ColumnClause::ColumnClause(QueryBuilder *query, const QString &columns)
    : Clause(*new ColumnClausePrivate)
{
    Q_D(ColumnClause);
    d->query = query;
    setColumns(columns);
}

ColumnClause::ColumnClause(QueryBuilder *query, const QStringList &columns)
    : Clause(*new ColumnClausePrivate)
{
    Q_D(ColumnClause);
    d->query = query;
    setColumns(columns);
}

ColumnClause::ColumnClause(const ColumnClause &other)
{
    d_ptr = other.d_ptr;
}

QString ColumnClause::interept(QueryGrammar *grammar)
{
    return grammar->clauseColumns(this);
}

QueryBuilder *ColumnClause::query() const
{
    Q_D(const ColumnClause);
    return d->query;
}

/**
 * @brief JoinClause::JoinClause
 * @param query
 * @param type
 * @param table
 */
JoinClause::JoinClause(QueryBuilder *parent, const QString &type,
                       const QString &table)
    : Clause(*new JoinClausePrivate)
{
    Q_D(JoinClause);
    d->query.reset(new QueryBuilder(parent->connection(), parent->grammar()));
    d->type = type;
    d->table = table;
}

QString JoinClause::interept(QueryGrammar *grammar)
{
    return grammar->clauseJoin(this);
}

QString JoinClause::table() const
{
    Q_D(const JoinClause);
    return d->table;
}

QString JoinClause::joinType() const
{
    Q_D(const JoinClause);
    return d->type;
}

QueryBuilder &JoinClause::on(const QString &first, const QString &op,
                       const QString &second, const QString &boolean)
{
    Q_D(JoinClause);
    return d->query->whereColumn(first, op, second, boolean);
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
    return d->query->where(first, op, second, boolean);
}

QueryBuilder *JoinClause::query() const
{
    Q_D(const JoinClause);
    return d->query.get();
}

QList<JoinClause *> JoinClause::joins() const
{
    Q_D(const JoinClause);
    return d->joins;
}

/**
 * @brief WhereClause::WhereClause
 */
WhereClause::WhereClause(QueryBuilder *query, const QString &boolean)
    : Clause(*new WhereClausePrivate)
{
    Q_D(WhereClause);
    d->query.reset(query);
    d->boolean = boolean;
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

QString WhereClause::interept(QueryGrammar *grammar)
{
    return grammar->clauseWhere(this);
}

void WhereClause::setInvokableMethod(const QString &method)
{
    Q_D(WhereClause);
    d->method = method;

//    if(method != "where")
//        d->method = d->method.replace(0, 1, method[0].toUpper());
}

QString WhereClause::invokableMethod() const
{
    Q_D(const WhereClause);
    return d->method;
}

bool WhereClause::betweenOrNot() const
{
    Q_D(const WhereClause);
    return d->betweenOrNotBetween;
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
    return d->first;
}

QString WhereClause::second() const
{
    Q_D(const WhereClause);
    return d->first;
}

QueryBuilder *WhereClause::query() const
{
    Q_D(const WhereClause);
    return d->query.get();
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

QString HavingClause::interept(QueryGrammar *grammar)
{
    Q_D(const HavingClause);
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

QString GroupClause::interept(QueryGrammar *grammar)
{
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

QString OrderClause::interept(QueryGrammar *grammar)
{
    return grammar->clauseOrder(this);
}

QString OrderClause::direction() const
{
    Q_D(const OrderClause);
    return d->direction;
}

/**
 * @brief UnionClause::UnionClause
 * @param query
 * @param all
 */
UnionClause::UnionClause(QueryBuilder *query, bool all)
    : Clause(*new UnionClausePrivate)
{
    Q_D(UnionClause);
    d->query = query;
    d->all = all;
}

QString UnionClause::interept(QueryGrammar *grammar)
{
    return grammar->clauseUnion(this);
}

QueryBuilder *UnionClause::query() const
{
    Q_D(const UnionClause);
    return d->query;
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

QString LimitClause::interept(QueryGrammar *grammar)
{
    return grammar->clauseLimit(this);
}

int LimitClause::limit() const
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

QString OffsetClause::interept(QueryGrammar *grammar)
{
    return grammar->clauseOffset(this);
}

int OffsetClause::offset() const
{
    Q_D(const OffsetClause);
    return d->offset;
}
