#ifndef SQLITECONNECTION_H
#define SQLITECONNECTION_H

#include "Connection.h"

class SQLiteConnectionPrivate;
class SQLiteConnection : public Connection
{
    Q_DECLARE_PRIVATE(SQLiteConnection)
public:
    explicit SQLiteConnection(const QString &prefix = "");

protected:
    virtual Grammar *createScheamGrammar();
    virtual Grammar *createQueryGrammar();
};

#endif // SQLITECONNECTION_H
