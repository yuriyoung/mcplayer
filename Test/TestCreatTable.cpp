#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Database.h"
#include "Connection.h"
#include "Grammar.h"


#include <QUuid>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonObject>
#include <QSharedPointer>
#include <QDebug>
#include <cstdint>
TEST_CASE("Creat table") {
    SECTION("use default config create tabel users") {
        Connection *connection = Database::instance()->connection();

        REQUIRE(connection->connectionName() == "connection0");

        QSharedPointer<Grammar> grammar = connection->schemaGrammar();
        Blueprint blueprint("users", [](Blueprint *blueprint){
            blueprint->increments("id").comment("primary key");
            blueprint->string("name");
            blueprint->string("password");
            blueprint->string("gender").nullable();
            blueprint->integer("post_id");
            blueprint->timestamp("verified_at").useCurrent();
            blueprint->timestamps();
            blueprint->softDeletes();
            blueprint->foreign({"post_id"}).references("id").on("posts").onDelete("cascade").onUpdate("cascade");
        });
        blueprint.create();
        blueprint.build(connection, grammar.data());
    }
}
