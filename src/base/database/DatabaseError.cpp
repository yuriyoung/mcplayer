#include "DatabaseError.h"

DatabaseError::DatabaseError(int errorCode, const QString &errorString, const QString &message)
    : RuntimeError(message, errorCode), errorText(errorString)
{

}

QString DatabaseError::errorString() const
{
    return this->errorText;
}

ConnectionDatabaseError::ConnectionDatabaseError(const QString &message)
    : DatabaseError(1, QLatin1String("Database connection error"), message)
{

}

StatementDatabaseError::StatementDatabaseError(const QString &message)
    : DatabaseError(2, QLatin1String("Database statement error"), message)
{

}

TransactionDatabaseError::TransactionDatabaseError(const QString &message)
    : DatabaseError(3, QLatin1String("Database transaction error"), message)
{

}
