#ifndef SQLITEQUERYGRAMMAR_H
#define SQLITEQUERYGRAMMAR_H

#include "QueryGrammar.h"

class SQLiteQueryGrammar : public QueryGrammar
{
public:
    explicit SQLiteQueryGrammar(QObject *parent = nullptr);

    QString compileUpdate(QueryBuilder *builder, const QList<QVariantMap> &values) override;
};

#endif // SQLITEQUERYGRAMMAR_H
