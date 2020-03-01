#include "RuntimeError.h"

class RuntimeErrorPrivate
{
public:
    RuntimeErrorPrivate(const QString &message, int code)
        : message(message), code(code) {}

    QString message;
    int code;
};

RuntimeError::RuntimeError(const QString &message, int code)
    : QException(), d(new RuntimeErrorPrivate(message, code))
{

}

RuntimeError::~RuntimeError()
{

}

QString RuntimeError::message() const
{
    return d->message;
}

int RuntimeError::code() const
{
    return d->code;
}
