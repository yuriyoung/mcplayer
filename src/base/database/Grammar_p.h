#ifndef GRAMMAR_P_H
#define GRAMMAR_P_H

#include <QObject>

class Grammar;
class GrammarPrivate
{
    Q_DECLARE_PUBLIC(Grammar)
public:
    GrammarPrivate(Grammar *q) : q_ptr(q) { }
    virtual ~GrammarPrivate() {}

    Grammar *q_ptr = nullptr;
    QString tablePrefix = "";
};

#endif // GRAMMAR_P_H
