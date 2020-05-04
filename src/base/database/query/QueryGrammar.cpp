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
    QMap<int, QString> segments; //NOTE: auto ascending order by clause type
    QList<int> keys = builder->clauses().keys();
    foreach(int key, keys)
    {
        segments[key] = compileClauses(builder, Clause::ClauseType(key));
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
    QList<Clause*> clauses = builder->clauses(type);
    QueryGrammar *grammar = const_cast<QueryGrammar*>(q_func());
    for(int i = 0; i < clauses.size(); ++i)
    {
        segments << clauses[i]->interpret(grammar, i == 0);
    }

    QString result = segments.join(" ").trimmed();
    if(Clause::Union == type)
    {
        if(!builder->clauses(Clause::UnionOrder).isEmpty())
            result += " " + this->compileClauses(builder, Clause::UnionOrder);

        // TODO: compile union limit and union offset ?
    }

    return result;
}

/**
 * @brief just remove first and / or string.
 *  this function absoleted. for details see \fn *Clause::leading().
 * @param clause
 * @return
 */
QString QueryGrammarPrivate::removeLeadingBoolean(const QString clause) const
{
    QString text = clause;
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

QStringList QueryGrammar::compile(void *data, int type)
{
    qDebug() << "QueryGrammar::compile()";

//    QSharedPointer<QueryBuilder> query(static_cast<QueryBuilder *>(data));
    QueryBuilder *query = static_cast<QueryBuilder *>(data);
    if(!query)
    {
        qWarning() << "Could not compile for null data, can not convert to QueryBuilder.";
        return QStringList();
    }

    QStringList statements;
    switch (type)
    {
    case QueryBuilder::SelectStatement:
        statements << this->compileSelect(query);
        break;
    case QueryBuilder::InsertStatement:
    {
        QList<QVariantMap> bindings = query->bindings(QueryBuilder::InsertBinding);
        statements << this->compileInsert(query, bindings);
        break;
    }
    case QueryBuilder::UpdateStatement:
    {
        QList<QVariantMap> bindings = query->bindings(QueryBuilder::UpdateBinding);
        statements << this->compileUpdate(query, bindings);
        break;
    }
    case QueryBuilder::DeleteStatement:
        statements << this->compileDelete(query);
        break;
    case QueryBuilder::ExistsStatement:
        statements << this->compileExists(query);
        break;
    case QueryBuilder::RandomStatement:
        statements << this->compileRandom("");
        break;
    case QueryBuilder::TruncateStatement:
        statements << this->compileSavepoint("");
        break;
    case QueryBuilder::RollBackStatement:
        statements << this->compileRollBack("");
        break;
    }

    qDebug() << statements.join("; ");

    return statements;
}

QString QueryGrammar::compileSelect(QueryBuilder *builder) const
{
    Q_D(const QueryGrammar);

    if(builder->clauses(Clause::Column).isEmpty())
        builder->addClause(Clause::Column, new ColumnClause("*", builder));

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
    const QString table = this->wrapTable(builder->table());

    if(values.isEmpty())
        return QString("insert into %1 default values").arg(table);

    // 从二维数组中取第一个元素的key作为绑定的列
    // 因为默认批量插入的是相同类型的对象，具有相同的列以及列顺序
    const QString columns = this->columnize(values.first().keys());

    // We need to build a list of parameter place-holders of values that are bound
    // to the query. Each insert should have the exact same amount of parameter
    // bindings so we will loop through the record and parameterize them all.
    QStringList records;
    foreach(auto &val, values)
    {
        // wrap place-holders of a record values
        records << "(" + this->parameterize(val.values(), true) + ")";
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

    const QString table = this->wrapTable(builder->table());
    builder->removeClause(Clause::Column);
    builder->removeClause(Clause::From);
    QStringList wheres = d->compileClauses(builder);

    QStringList columns;
    const QVariantMap record = values.first();
    QVariantMap::const_iterator it = record.constBegin();
    while (it != record.constEnd())
    {
        columns << wrap(it.key()) + " = " + parameter(it.value(), true);
        ++it;
    }

    // TODO: join clause
    // update table {join} set columns wheres
    return QString("update %1 set %3 %4")
            .arg(table)
            .arg(columns.join(", "))
            .arg(wheres.join(" "));
}

QString QueryGrammar::compileDelete(QueryBuilder *builder)
{
    Q_D(QueryGrammar);
    builder->removeClause(Clause::From);
    const QString table = wrapTable(builder->table());
    const QStringList clauses = d->compileClauses(builder);
    // TODO: alias and joins alause
    // ...

    // delete {alias} from table {joins} where
    return QString("delete from %1 %2").arg(table).arg(clauses.join(" "));
}

QString QueryGrammar::compileExists(QueryBuilder *builder)
{
    QString select = this->compileSelect(builder);
    QString exists = this->wrap("exists");
    return QString("select exists(%1) as %2")
            .arg(select).arg(exists);
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
QString QueryGrammar::clauseColumn(ColumnClause *cc) const
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
    // invoke where*();
    QString invokeMethod = where->whereMethod();
    QByteArray text = invokeMethod.toLocal8Bit();
    const char *method = text.constData();

    QString retVal;
    QMetaObject::invokeMethod(
        this,
        method,
        Qt::DirectConnection,
        Q_RETURN_ARG(QString, retVal),
        Q_ARG(WhereClause*, where)
    );

    //TODO: is the join query where?(see clauseJoin(...))

    return where->leading() ? "where " + retVal : where->boolean() + " " + retVal;
}

QString QueryGrammar::clauseHaving(HavingClause *having) const
{
    QString column = this->wrap(having->columns().first());
    QString parameter = this->parameter(having->value());
    QString leading = having->leading() ? "having" : having->boolean();

    return QString("%1 %2 %3 %4")
            .arg(leading)
            .arg(column)
            .arg(having->op())
            .arg(parameter);
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

    const QString table = wrapTable(join->table());

    // TODO: nested joins

    QStringList wheres = d->compileClauses(join->subQuery().data());
    wheres.replaceInStrings("where ", "on ");

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

    QString leading = order->leading() ? "order by " : "";

    return QString("%1%2 %3")
            .arg(leading)
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
    return "limit " + QString::number(limit->value());
}

QString QueryGrammar::clauseOffset(OffsetClause *offset) const
{
    return "offset " + QString::number(offset->value());
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
        QString result = values.canConvert(QVariant::List)
                ? parameterize(values.toList())
                : parameter(values);
        return wrap(where->columns().first()) + " in (" + result + ")";
    }

    return "0 = 1";
}

QString QueryGrammar::whereNotIn(WhereClause *where) const
{
    QVariant values = where->value();
    if(values.isValid())
    {
        QString result = values.canConvert(QVariant::List)
                ? parameterize(values.toList())
                : parameter(values);
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
    QString between = where->type() == WhereClause::Between ? "between" : "not between";
    QString min = parameter(where->value().toList().first());
    QString max = parameter(where->value().toList().last());

    return QString("%1 %2 %3 and %4")
            .arg(wrap(where->columns().first()))
            .arg(between).arg(min).arg(max);
}

QString QueryGrammar::whereNested(WhereClause *where) const
{
    Q_D(const QueryGrammar);
    QString nested = d->compileClauses(where->subQuery().get(), Clause::Where);
    return "(" + nested + ")";
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
    QString sql = this->compileSelect(where->subQuery().get());
    return "exists (" + sql + ")";
}

QString QueryGrammar::whereNotExists(WhereClause *where) const
{
    QString sql = this->compileSelect(where->subQuery().get());
    return "not exists (" + sql + ")";
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
    QString select = d->compileClauses(where->subQuery().get()).join(" ");
    return wrap(where->columns().first()) + " " + where->op() + " ("+ select +")";
}

QString QueryGrammar::whereJsonBoolen(WhereClause *where) const
{
    Q_UNUSED(where)
    // TODO: implemention in sub-class for some database engine
    qWarning() << "This database engine does not support JSON operations.";
    return "";
}

QString QueryGrammar::whereJsonContains(WhereClause *where) const
{
    Q_UNUSED(where)
    // TODO: implemention in sub-class for some database engine
    qWarning() << "This database engine does not support JSON operations.";
    return "";
}

void QueryGrammar::removeClause(QueryBuilder *builder, Clause::ClauseType type)
{
    builder->removeClause(type);
}

