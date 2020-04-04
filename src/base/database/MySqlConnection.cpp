#include "MySqlConnection.h"

MySqlConnection::MySqlConnection(const QString &prefix)
    : Connection(prefix)
{

}

Grammar *MySqlConnection::queryGrammar()
{
    // TODO: create SqliteQueryGrammar

    return nullptr;
}

Grammar *MySqlConnection::schemaGrammar()
{
    // do not create here
    // TODO: create a schema builder somewhere

    return nullptr;
}

bool MySqlConnection::reconnect()
{
    return false;
}
