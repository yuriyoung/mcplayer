#include "SQLiteQueryGrammar.h"
#include "QueryBuilder.h"

SQLiteQueryGrammar::SQLiteQueryGrammar(QObject *parent)
    : QueryGrammar(parent)
{

}

QString SQLiteQueryGrammar::compileUpdate(QueryBuilder *builder, const QList< QVariantMap > &values)
{
//    Q_D(SQLiteQueryGrammar)
    if(builder->clauses(Clause::Limit).isEmpty() && builder->clauses(Clause::Join).isEmpty())
        return QueryGrammar::compileUpdate(builder, values);

    const QString table = this->wrapTable(builder->table());
    QString alias = builder->table().split(QRegExp("\\s+as\\s+")).last();
    QString select = this->compileSelect(&builder->select(alias + ".rowid"));

    QStringList columns;
    const QVariantMap record = values.first();
    QVariantMap::const_iterator it = record.constBegin();
    while (it != record.constEnd())
    {
        columns << wrap(it.key()) + " = " + parameter(it.value(), true);
        ++it;
    }

    return QString("update %1 set %2 where %3 in (%4)")
            .arg(table).arg(columns.join(", ")).arg(wrap("rowid")).arg(select);
}
