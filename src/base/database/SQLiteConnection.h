#ifndef SQLITECONNECTION_H
#define SQLITECONNECTION_H

#include "Connection.h"

class SQLiteConnectionPrivate;
class SQLiteConnection : public Connection
{
    Q_DECLARE_PRIVATE(SQLiteConnection)
public:
    explicit SQLiteConnection(const QString &prefix = "");

    virtual Grammar *queryGrammar();
    virtual Grammar *schemaGrammar();
};

#endif // SQLITECONNECTION_H
