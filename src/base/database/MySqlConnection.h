#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include "Connection.h"

class MySqlConnection : public Connection
{
public:
    explicit MySqlConnection(const QString &prefix = "");

    virtual Grammar *queryGrammar();
    virtual Grammar *schemaGrammar();

    virtual bool reconnect();
};

#endif // MYSQLCONNECTION_H
