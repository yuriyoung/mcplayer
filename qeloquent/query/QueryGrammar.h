#ifndef QUERYGRAMMAR_H
#define QUERYGRAMMAR_H

#include "Grammar.h"
#include "Clause.h"

#include <QVariant>

class QueryBuilder;
class QueryGrammarPrivate;
class QueryGrammar : public Grammar
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QueryGrammar)
public:
    using Records = QList< QVariantMap >;

    QueryGrammar(QObject *parent = nullptr);
    ~QueryGrammar() override;

    virtual QStringList compile(void *builder, int type) override;

    virtual QString compileSelect(QueryBuilder *builder) const;
    virtual QString compileInsert(QueryBuilder *builder, const Records &values);
    virtual QString compileUpdate(QueryBuilder *builder, const Records &values);
    virtual QString compileDelete(QueryBuilder *builder);
    virtual QString compileExists(QueryBuilder *builder);
    virtual QString compileRandom(const QString &seed);
    virtual QString compileTruncate(const QString &table);
    virtual QString compileSavepoint(const QString &name);
    virtual QString compileRollBack(const QString &name);

    /**
     * compile clause components (interpret by clause)
     */
    virtual QString clauseAggregate(AggregateClause *ac) const;
    virtual QString clauseColumn(ColumnClause *cc) const;
    virtual QString clauseFrom(FromClause *fc) const;
    virtual QString clauseWhere(WhereClause *where);
    virtual QString clauseHaving(HavingClause *having) const;
    virtual QString clauseHavingBetween(HavingClause *having) const;
    virtual QString clauseGroup(GroupClause *group) const;
    virtual QString clauseJoin(JoinClause *join) const;
    virtual QString clauseOrder(OrderClause *order) const;
    virtual QString clauseUnion(UnionClause *uc) const;
    virtual QString clauseLimit(LimitClause *limit) const;
    virtual QString clauseOffset(OffsetClause *offset) const;


// invokable where clauses
protected:
    Q_INVOKABLE virtual QString where(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereIn(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereNotIn(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereInRaw(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereNotInRaw(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereNull(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereNotNull(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereBetween(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereNested(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereColumn(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereDate(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereTime(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereDay(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereMonth(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereYear(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereExists(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereNotExists(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereRowValues(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereSub(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereJsonBoolen(WhereClause *where) const;
    Q_INVOKABLE virtual QString whereJsonContains(WhereClause *where) const;

    // other invokble clauses in sub-class
    // ...

protected:
    void removeClause(QueryBuilder *builder, Clause::ClauseType type);
};

#endif // QUERYGRAMMAR_H
