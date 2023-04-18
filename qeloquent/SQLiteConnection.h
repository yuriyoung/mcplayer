#ifndef SQLITECONNECTION_H
#define SQLITECONNECTION_H
#include "QEloquentMacro.h"
#include "Connection.h"

class SQLiteConnectionPrivate;
class QELOQUENT_EXPORT SQLiteConnection : public Connection
{
    Q_DECLARE_PRIVATE(SQLiteConnection)
public:
    explicit SQLiteConnection(const QString &prefix = "");

protected:
    virtual Grammar *createScheamGrammar();
    virtual Grammar *createQueryGrammar();
};

#endif // SQLITECONNECTION_H
