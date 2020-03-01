#ifndef RUNTIMEERROR_H
#define RUNTIMEERROR_H

#include "global.h"

#include <QException>
#include <QString>

class RuntimeErrorPrivate;
class RuntimeError : public QException
{
    D_PTR(RuntimeError)
public:
    RuntimeError(const QString &message = {}, int code = 0);
    ~RuntimeError();

    QString message() const;
    int code() const;
};

#endif // RUNTIMEERROR_H
