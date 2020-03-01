#ifndef DATABASEERROR_H
#define DATABASEERROR_H

#include "RuntimeError.h"

class DatabaseError : public RuntimeError
{
public:
    DatabaseError(int errorCode, const QString &errorString, const QString &message = {});

    QString errorString() const;

private:
    QString errorText = "Unknown database error";
};

class ConnectionDatabaseError : public DatabaseError
{
public:
    explicit ConnectionDatabaseError(const QString &message = {});
};

class StatementDatabaseError : public DatabaseError
{
public:
    explicit StatementDatabaseError(const QString &message = {});
};

class TransactionDatabaseError : public DatabaseError
{
public:
    explicit TransactionDatabaseError(const QString &message = {});
};

#endif // DATABASEERROR_H
