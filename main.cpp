#include "schema/Blueprint.h"
#include "schema/SQLiteSchemaGrammar.h"
#include "Connection.h"
#include "SQLiteConnection.h"
#include "ConnectionProvider.h"
#include "Database.h"
#include "query/QueryBuilder.h"
#include "query/QueryGrammar.h"
#include "eloquent/User.h"

#include <QCoreApplication>
#include <QMetaMethod>
#include <QJsonObject>
#include <QDate>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

/*
    Database db;
    db.setTablePrefix("tst_");
    SchemaBuilder builder(&db);
    builder.create("posts", [](Blueprint *blueprint)
    {
        blueprint->unsignedInteger("id", true);
//        blueprint->primary({"id"});
        blueprint->string("title");
        blueprint->text("content");
        blueprint->integer("user_id", false, true);
        blueprint->tinyInteger("published");
        blueprint->foreign({"user_id"}, "", "id", "users");
    });
*/

//    SchemaGrammar grammar;
//    ColumnDefinition column
//    {
//        {"type", "double"},
//        {"name", "name"},
//        {"useCurrent", true},
//        {"length", 8},
//        {"total", 8},
//        {"places", 8},
//    };
//    qDebug() << grammar.getType(column);

/*
    int methodIndex = grammar.metaObject()->indexOfMethod("getType");
    if(methodIndex < 0)
    {
        QByteArray normalizedSignature = QMetaObject::normalizedSignature("getType(ColumnDefinition)");
        methodIndex = grammar.metaObject()->indexOfMethod(normalizedSignature);
        qDebug() << "index normalized Signature";
    }
    QMetaMethod metaMethod = grammar.metaObject()->method(methodIndex);

    qDebug() << "Access: " << metaMethod.access();
    qDebug() << "Valid: " << metaMethod.isValid();
    qDebug() << "Index: " << metaMethod.methodIndex();
    qDebug() << "Signature: " << metaMethod.methodSignature();
    qDebug() << "Type: " << metaMethod.methodType();
    qDebug() << "Name: " << metaMethod.name();
    qDebug() << "Parameter names: " << metaMethod.parameterNames();
    qDebug() << "Parameter types: " << metaMethod.parameterTypes();
    qDebug() << "Return type: " << metaMethod.returnType();
    qDebug() << "Type name: " << metaMethod.typeName();
*/
/*

    SQLiteSchemaGrammar grammar;
    Blueprint blueprint("users", [](Blueprint *blueprint){
        blueprint->increments("id").comment("primary key");
        blueprint->string("name").unique();
        blueprint->string("password");
        blueprint->string("gender").nullable();
        blueprint->integer("post_id");
        blueprint->timestamp("verified_at").useCurrent();
        blueprint->timestamps();
        blueprint->softDeletes();

//        blueprint->foreign({"post_id"}).references("id").on("posts").onDelete("cascade").onUpdate("cascade");
    });

    blueprint.create();
    blueprint.build(nullptr, &grammar);
*/


/*
    {
        DatabaseManager::schema()->dropIfExists("users");

        DatabaseManager::schema("users", [](SchemaBuilder *table){
            // ...
        });

        DatabaseManager::schema()->create("users", [](Blueprint *table)
        {
            table->increments("id").comment("primary key");
            table->string("name").unique();
            table->string("password");
            table->string("gender").nullable();
            table->integer("post_id");
            table->timestamp("verified_at").useCurrent();
            table->timestamps();
            table->softDeletes();

            table->foreign({"post_id"}).references("id").on("posts").onDelete("cascade").onUpdate("cascade");
        });
    }
*/

//    QDateTime now = QDateTime::currentDateTime();
//    qDebug() << now.toString("yyyy-MM-dd HH:mm:ss");
    //Create Table
    Connection *connection = Database::instance()->connection();
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

    {
//        QString sql = Database::table("users").select(/*"name"*/)
//                .from("users")//.distinct()
//                .where("gender", "1")
//                .where("name", "=", "abc")
//                .orWhere("name", "=", "aa")
//                .orderBy("name")
////                .join("posts", "users.post_id", "=", "posts.id")
//                .join("posts", "post_id", "=", "id")
//                .limit(10)
//                .toSql();

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
        ok = Database::table().from("users","u").where("id", 5).update({{"name", "Test update1"}});
        ok = Database::table().from("users","u").updateOrInsert(record, {{"name", "Saly Two"}});
        ok = Database::table().from("users","u").where("id", 27).destroy();
        ok = Database::table().from("users").destroy(2);
        ok = Database::table().from("users","u").destroy(2);
        qDebug() << ok;
    }

//    QMap<int, QSharedPointer<A>> map;
//    map.insert(0, QSharedPointer<A>(new A(1)));
//    map.insert(1, QSharedPointer<A>(new A(2)));
//    map.insert(2, QSharedPointer<A>(new A(3)));
//    map.insert(0, QSharedPointer<A>(new A(4)));
//    map.remove(2);

//    User user;
//    user.dump();

    return app.exec();
}
