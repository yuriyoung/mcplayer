#include "catch.hpp"
#include "Database.h"

// qt libray
#include <QList>
#include <cstdint>
TEST_CASE("insert record") {
    SECTION("use default config create tabel users") {
        QList<QVariantMap> records
            {
                QVariantMap
                {
                    {"name", "Mey"},
                    {"gender", 2},
                    {"password", "secret"},
                    {"post_id","123"}
                },
                QVariantMap
                {
                    {"name", "Yui"},
                    {"gender", 2},
                    {"password", "secret"},
                    {"post_id","123"}
                }
            };

        QVariantMap record
            {
                {"name", "Saly Tow"},
                {"gender", 1},
                {"password", "secret"},
                {"post_id","123"}
            };

        qint64 ok = -1;
        ok = Database::table().from("users","u").insert(records);
        REQUIRE(ok == 1);
        ok = Database::table().from("users","u").where("id", 5).update({{"name", "Test update1"}});
        REQUIRE(ok == 1);
        ok = Database::table().from("users","u").updateOrInsert(record, {{"name", "Saly Two"}});
        REQUIRE(ok == 1);
        ok = Database::table().from("users","u").where("id", 27).destroy();
        REQUIRE(ok == 0);
        ok = Database::table().from("users").destroy(2);
        REQUIRE(ok == 0);
        ok = Database::table().from("users","u").destroy(2);
        REQUIRE(ok == 0);
    }
}
