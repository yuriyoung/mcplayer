#include "QueryGrammar.h"
#include "QueryGrammar_p.h"
#include "QueryBuilder.h"
#include "Clause.h"

#include <QMetaObject>
#include <QRegularExpression>
#include <QDebug>

QueryGrammarPrivate::QueryGrammarPrivate(Grammar *q)
    : GrammarPrivate(q)
{

}

QStringList QueryGrammarPrivate::compileClauses(QueryBuilder *builder) const
{
    QMap<int, QString> segments; //NOTE: auto sort by clause type
    QMap<int, QList<Clause *> > clauses = builder->clauses();
    QMap<int, QList<Clause *> >::const_iterator it = clauses.constBegin();
    while (it != clauses.constEnd())
    {
        segments[it.key()] = compileClauses(builder, Clause::ClauseType(it.key()));
        ++it;
    }

    // Concatenate an array of segments, removing empties.
    QStringList statements;
    foreach(auto &seg, segments)
    {
        if(!seg.isEmpty())
            statements << seg;
    }

    return statements;
}

QString QueryGrammarPrivate::compileClauses(QueryBuilder *builder, Clause::ClauseType type) const
{
    QStringList segments;
    QList<Clause*> conditions = builder->clauses(type);
    QueryGrammar *grammar = const_cast<QueryGrammar*>(q_func());
    foreach(auto cond, conditions)
    {
        segments << cond->interept(grammar);
    }

    segments.removeAll("");
    QString result;
    if(type == Clause::Where || type == Clause::Having)
        result = removeLeadingBoolean(segments);
    else
        result = segments.join(" ");

    switch (type)
    {
    case Clause::Where:     return "where " + result; // TODO: using 'on' while is join clause
    case Clause::Having:    return "having " + result;
    case Clause::Order:     return "order by " + result;
    case Clause::Union:
    {
        if(!builder->clauses(Clause::UnionOrder).isEmpty())
            result += " " + this->compileClauses(builder, Clause::UnionOrder);

        // TODO: compile union limit and union offset ?

        break;
    }
    default: break;
    }

    return result;
}

QString QueryGrammarPrivate::removeLeadingBoolean(const QStringList clauses) const
{
    QString text = clauses.join(" ").trimmed();
    QRegExp rx("and\\s+|or\\s+");
    rx.setMinimal(true);
    // remove the leading boolean
    int pos = rx.indexIn(text);
    if(pos == 0)
    {
        int len = rx.matchedLength();
        text = text.remove(pos, len);
    }

    return text;
}

QString QueryGrammarPrivate::dateWhere(const QString &type, WhereClause *where) const
{
    QString value = q_ptr->parameter(where->value());
    QString column = q_ptr->wrap(where->columns().first());
    return QString("%1(%2) %3 %4").arg(type).arg(column).arg(where->op()).arg(value);
}

/**
 * @brief QueryGrammar::QueryGrammar
 */
QueryGrammar::QueryGrammar(QObject *parent)
    : Grammar(*new QueryGrammarPrivate(this), parent)
{

}

QueryGrammar::~QueryGrammar()
{
    qDebug() << "QueryGrammar::~QueryGrammar()";
}

QStringList QueryGrammar::compile(void *data)
{
    qDebug() << "QueryGrammar::compile()";

    QStringList statements;
    QueryBuilder *builder = static_cast<QueryBuilder *>(data);
    if(!builder)
    {
        qWarning() << "parameter data can not coverte to QueryBuilder";
        return statements;
    }

    switch (builder->statementType())
    {
    case QueryBuilder::SelectStatement:
        statements << compileSelect(builder);
        break;
    case QueryBuilder::InsertStatement:
    {
        BindingsHash bindings = builder->bindings();
        statements << this->compileInsert(builder, bindings[QueryBuilder::InserBinding]);
        break;
    }
    case QueryBuilder::UpdateStatement:
    {
        BindingsHash bindings = builder->bindings();
        statements << this->compileUpdate(builder, bindings[QueryBuilder::UpdateBinding]);
        break;
    }
    case QueryBuilder::DeleteStatement:
        statements << this->compileDelete(builder);
        break;
    }

    return statements;
}

QString QueryGrammar::compileSelect(QueryBuilder *builder)
{
    Q_D(QueryGrammar);

    QStringList statements = d->compileClauses(builder);

    if(!builder->clauses(Clause::Union).isEmpty() && builder->isAggregated())
    {
        // compile union aggregate
        QString sql = d->compileClauses(builder, Clause::Aggregate);
        builder->removeClause(Clause::Aggregate);
        return sql + " from (" + statements.join(" ") + ") as " + wrapTable("temp_table");
    }

    // TODO: compile unions here ?


    return statements.join(" ");
}

/**
 * [
 *  QMap<QString, QVariant>,
 *  Object, ?
 *  ...
 * ]
 * @brief QueryGrammar::compileInsert
 * @param builder
 * @param values
 * @return
 */
QString QueryGrammar::compileInsert(QueryBuilder *builder, const Records &values)
{
    const QString table = this->wrapTable(builder->from());

    if(values.isEmpty())
        return QString("insert into %1 default values").arg(table);

    // 从二维数组中取第一个元素的key作为绑定的列
    // 因为默认批量插入的是相同类型的对象，具有相同的列以及列顺序相同
    const QString columns = this->columnize(values.first().keys());

    // We need to build a list of parameter place-holders of values that are bound
    // to the query. Each insert should have the exact same amount of parameter
    // bindings so we will loop through the record and parameterize them all.
    QStringList records;
    foreach(auto &val, values)
    {
        // wrap place-holders of a record values
        records << "(" + this->parameterize(val.values()) + ")";
    }
    const QString parameters = records.join(", ");

    return QString("insert into %1 (%2) values %3")
            .arg(table).arg(columns).arg(parameters);
}

QString QueryGrammar::compileUpdate(QueryBuilder *builder, const Records &values)
{
    Q_D(QueryGrammar);
    if(values.isEmpty())
        return "";

    const QString table = this->wrapTable(builder->from());
    QStringList wheres = d->compileClauses(builder);
    // TODO: clause joins for a update statement

    QStringList columns;
    QVariantMap val = values.first();
    QVariantMap::const_iterator it = val.constBegin();
    while (it != val.constEnd())
    {
        columns << wrap(it.key()) + " = " + parameter(it.value());
    }

    // update table {join} set columns wheres
    return QString("update %1 %2 set %3 %4")
            .arg(table).arg("")
            .arg(columns.join(", "))
            .arg(wheres.join(" "));
}

QString QueryGrammar::compileDelete(QueryBuilder *builder)
{
    Q_D(QueryGrammar);
    const QString table = wrapTable(builder->from());
    const QStringList wheres = d->compileClauses(builder);
    // TODO: joins conditions
    // ...

    return QString("delete from %1 %2").arg(table).arg(wheres.join(" "));
}

QString QueryGrammar::compileExists(QueryBuilder *builder)
{
    Q_D(QueryGrammar);
    QStringList conditions = d->compileClauses(builder);
    QString exists = this->wrap("exists");
    return QString("select exists(%1) as %2")
            .arg(conditions.join(" ")).arg(exists);
}

QString QueryGrammar::compileRandom(const QString &seed)
{
    return QString("random(%1)").arg(seed);
}

QString QueryGrammar::compileTruncate(const QString &table)
{
    return "truncate table " + wrapTable(table);
}

QString QueryGrammar::compileSavepoint(const QString &name)
{
    return "savepoint " + name;
}

QString QueryGrammar::compileRollBack(const QString &name)
{
    return "rollback to savepoint " + name;
}

QString QueryGrammar::clauseAggregate(AggregateClause *ac)  const
{
//    if(!ac->query()->isAggregated())
//        return "";

    QString columns = columnize(ac->columns());
    // 如果查询有一个“distinct”约束，并且没有要求所有列，
    // 那么我们需要在列名之前加上“distinct”，
    // 以便在对数据执行聚合操作时考虑到它。
    if(ac->query()->isDistincted() && columns != "*")
        columns = "distinct " + columns;

    return QString("select %1(%2) as aggregate")
            .arg(ac->function()).arg(columns);
}

// TODO: do bindings in column clause?
QString QueryGrammar::clauseColumns(ColumnClause *cc) const
{
    // 如果查询执行的是一个聚合select，我们让聚合编译函数处理select子句的构建，
    // 因为它需要更多的语法，最好由该函数来处理，以保持一切整洁。
    if(cc->query()->isAggregated())
        return "";

    QString select = cc->query()->isDistincted() ?
                "select distinct " : "select ";

    // TODO: handling column bindings here?

    QStringList columns = cc->columns();
    if(columns.isEmpty())
        columns = QStringList("*");

    return select + columnize(columns);
}

QString QueryGrammar::clauseFrom(FromClause *fc) const
{
    return "from " + this->wrapTable(fc->table());
}

QString QueryGrammar::clauseWhere(WhereClause *where)
{
    // invok where*();
    QString invokMethod = where->invokableMethod();
//    invokMethod = invokMethod.startsWith("where", Qt::CaseSensitive) ? invokMethod : "where" + invokMethod;
    QByteArray text = invokMethod.toLocal8Bit();
    const char *method = text.constData();

    QString retVal;
    QMetaObject::invokeMethod(this,
        method,
        Qt::DirectConnection,
        Q_RETURN_ARG(QString, retVal),
        Q_ARG(WhereClause*, where)
    );

    return where->boolean() + " " + retVal;
}

QString QueryGrammar::clauseHaving(HavingClause *having) const
{
    QString result;
    QString column = this->wrap(having->columns().first());
    QString parameter = this->parameter(having->value());
    result = QString("%1 %2 %3 %4")
            .arg(having->boolean())
            .arg(column).arg(having->op()).arg(parameter);
    return result;
}

QString QueryGrammar::clauseHavingBetween(HavingClause *having) const
{
    QString column = this->wrap(having->columns().first());
    QString between = having->betweenOrNot() ? "between" :  "not between";
    QList<QVariant> values = having->value().toList();
    if(values.size() < 2)
    {
        qWarning() << "Having between at least two values";
        return "";
    }

    QString min = this->parameter(values.first());
    QString max = this->parameter(values.last());
    return QString("%1 %2 %3 %4 and %5")
            .arg(having->boolean())
            .arg(column).arg(between).arg(min).arg(max);
}

QString QueryGrammar::clauseGroup(GroupClause *group) const
{
    QStringList cols = group->columns();
    if(cols.isEmpty())
        return QString();

    return "group by " + this->columnize(cols);
}

QString QueryGrammar::clauseJoin(JoinClause *join) const
{
    Q_D(const QueryGrammar);
    // TODO: many many things ==!!!
    const QString table = wrapTable(join->table());

    // TODO: nested joins

    QStringList wheres = d->compileClauses(join->query());

    // inner join (table {}) {wheres}
    return QString("%1 join %2 %3")
            .arg(join->joinType())
            .arg(table)
            .arg(wheres.join(" "));
}

QString QueryGrammar::clauseOrder(OrderClause *order) const
{
    if(order->columns().isEmpty())
        return QString();

    return QString("%1 %2")
            .arg(wrap(order->columns().first()))
            .arg(order->direction());
}

QString QueryGrammar::clauseUnion(UnionClause *uc) const
{
    QString conjunction = uc->all() ? " union all " : " union ";
    return conjunction + "(" + uc->query()->toSql() + ")";
}

QString QueryGrammar::clauseLimit(LimitClause *limit) const
{
    return "limit " + QString::number(limit->limit());
}

QString QueryGrammar::clauseOffset(OffsetClause *offset) const
{
    return "offset " + QString::number(offset->offset());
}

QString QueryGrammar::where(WhereClause *where) const
{
    return wrap(where->columns().first()) + " " + where->op() + " " + parameter(where->value());
}

QString QueryGrammar::whereIn(WhereClause *where) const
{
    QVariant values = where->value();
    if(values.isValid())
    {
        QString result = values.canConvert(QVariant::List) ? parameterize(values.toList()) : parameter(values);
        return wrap(where->columns().first()) + " in (" + result + ")";
    }

    return "0 = 1";
}

QString QueryGrammar::whereNotIn(WhereClause *where) const
{
    QVariant values = where->value();
    if(values.isValid())
    {
        QString result = values.canConvert(QVariant::List) ? parameterize(values.toList()) : parameter(values);
        return wrap(where->columns().first()) + " not in (" + result + ")";
    }

    return "1 = 1";
}

QString QueryGrammar::whereInRaw(WhereClause *where) const
{
    QVariant values = where->value();
    if(values.isValid())
    {
        QString results = values.toStringList().join(", ");
        return QString("%1 in (%2)").arg(wrap(where->columns().first())).arg(results);
    }

    return "0 = 1";
}

QString QueryGrammar::whereNotInRaw(WhereClause *where) const
{
    QVariant values = where->value();
    if(values.isValid())
    {
        QString results = values.toStringList().join(", ");
        return QString("%1 not in (%2)").arg(wrap(where->columns().first())).arg(results);
    }

    return "1 = 1";
}

QString QueryGrammar::whereNull(WhereClause *where) const
{
    return wrap(where->columns().first()) + " is null";
}

QString QueryGrammar::whereNotNull(WhereClause *where) const
{
    return wrap(where->columns().first()) + "  is not null";
}

QString QueryGrammar::whereBetween(WhereClause *where) const
{
    QString between = where->betweenOrNot() ? "between" : "not between";
    QString min = parameter(where->value().toList().first());
    QString max = parameter(where->value().toList().last());

    return QString("%1 %2 %3 and %4")
            .arg(wrap(where->columns().first()))
            .arg(between).arg(min).arg(max);
}

QString QueryGrammar::whereNested(WhereClause *where) const
{
    Q_UNUSED(where)
    // TODO:

    return "";
}

QString QueryGrammar::whereColumn(WhereClause *where) const
{
    return wrap(where->first()) + " " + where->op() + " " + wrap(where->second());
}

QString QueryGrammar::whereDate(WhereClause *where) const
{
    Q_D(const QueryGrammar);
    return d->dateWhere("date", where);
}

QString QueryGrammar::whereTime(WhereClause *where) const
{
    Q_D(const QueryGrammar);
    return d->dateWhere("time", where);
}

QString QueryGrammar::whereDay(WhereClause *where) const
{
    Q_D(const QueryGrammar);
    return d->dateWhere("day", where);
}

QString QueryGrammar::whereMonth(WhereClause *where) const
{
    Q_D(const QueryGrammar);
    return d->dateWhere("month", where);
}

QString QueryGrammar::whereYear(WhereClause *where) const
{
    Q_D(const QueryGrammar);
    return d->dateWhere("year", where);
}

QString QueryGrammar::whereExists(WhereClause *where) const
{
    Q_D(const QueryGrammar);
    return "exists (" + d->compileClauses(where->query()).join(" ") + ")";
}

QString QueryGrammar::whereNotExists(WhereClause *where) const
{
    Q_D(const QueryGrammar);
    return "not exists (" + d->compileClauses(where->query()).join(" ") + ")";
}

QString QueryGrammar::whereRowValues(WhereClause *where) const
{
    QString columns = columnize(where->columns());
    QString values = parameterize(where->value().toList());
    return QString("(%1) %2 (%3)").arg(columns).arg(where->op()).arg(values);
}

QString QueryGrammar::whereSub(WhereClause *where) const
{
    Q_D(const QueryGrammar);
    QVariant values = where->value();
    QString select = d->compileClauses(where->query()).join(" ");
    return wrap(where->columns().first()) + " " + where->op() + " ("+ select +")";
}

QString QueryGrammar::whereJsonBoolen(WhereClause *where) const
{
    Q_UNUSED(where)
    // TODO

    qWarning() << "This database engine does not support JSON operations.";
    return "";
}

QString QueryGrammar::whereJsonContains(WhereClause *where) const
{
    Q_UNUSED(where)
    // TODO
    qWarning() << "This database engine does not support JSON operations.";
    return "";
}

