#ifndef QUERYGRAMMAR_P_H
#define QUERYGRAMMAR_P_H

#include "Grammar_p.h"
#include "QueryGrammar.h"

class QueryGrammarPrivate : public GrammarPrivate
{
    Q_DECLARE_PUBLIC(QueryGrammar)
public:
    QueryGrammarPrivate(Grammar *q);

     static const QMap<int, QByteArray> &defaultSelectClauaseNames();

    QStringList compileClauses(QueryBuilder *builder) const;
    QString compileClauses(QueryBuilder *builder, Clause::ClauseType type) const;

    // remove first "and" or "or"
    QString removeLeadingBoolean(const QString clause) const;
    QString dateWhere(const QString &type, WhereClause *where) const;
};

#endif // QUERYGRAMMAR_P_H
