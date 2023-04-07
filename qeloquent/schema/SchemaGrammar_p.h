#ifndef SCHEMAGRAMMAR_P_H
#define SCHEMAGRAMMAR_P_H

#include "SchemaGrammar.h"
#include "Grammar_p.h"
#include <QHash>

class SchemaGrammarPrivate : public GrammarPrivate
{
    Q_DECLARE_PUBLIC(SchemaGrammar)
public:
    SchemaGrammarPrivate(Grammar *q);
    static const QHash<int, QByteArray> &defaultTypeNames();

    QString compileCommand(const Command &command);
    QString columnType(const ColumnDefinition &column);

    QHash<int, QByteArray> typeNames;
};

#endif // SCHEMAGRAMMAR_P_H
