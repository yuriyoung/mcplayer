#include "MySqlConnection.h"

MySqlConnection::MySqlConnection(const QString &prefix)
    : Connection(prefix)
{

}

Grammar *MySqlConnection::createScheamGrammar()
{
    // TODO: create SqliteQueryGrammar

    return nullptr;
}

Grammar *MySqlConnection::createQueryGrammar()
{
    // TODO: create a schema builder somewhere

    return nullptr;
}

bool MySqlConnection::reconnect()
{
    return false;
}
