#ifndef MODEL_H
#define MODEL_H

#include "HasRelationship.h"
#include "relations/Relation.h"

#include <QObject>
#include <QJsonObject>
#include <QVariantMap>
#include <QMetaProperty>
#include <QExplicitlySharedDataPointer>

/**
 * the Model data member auto mapping from/to database table
 *
 * usage:
 * class User : public Model
 * {
 *      Q_OBJECT
 *      // generate getter and setter right-click with mouse and select 'Refactor' in Qt Creator
 *      // the model will create a name column in the users table
 *      Q_PROPERTY(int id READ id WRITE setId)
 *      Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
 * public:
 *      // you can rewrite the default table
 *      QString table() const { return "users"; }
 *
 *      // also you can rewrite the default primary key
 *      QString primaryKey() const { return "uuid"; }
 *
 *      // the User model data
 *      int id() const
 *      {
 *          return m_id;
 *      }
 *
 *      QString name() const
 *      {
 *          return m_name;
 *      }
 *
 *      // other data
 *
 *      // relationship
 *      // for get Post results to call Relation::results() it returns a Collection<Post>
 *      Relation posts() const
 *      {
 *          // Post derive Model.
 *          // just create a relation query. for get the data from database
 *          // you have to call Relation::get();
 *          return hasMany<Post>();
 *      }
 * signals:
 *      void nameChanged(int id);
 *
 * public slots:
 *      void setId(int id)
 *      {
 *          if(id == m_id)
 *              return;
 *
 *          m_id = id;
 *      }
 *
 *      void setName(const QString &name)
 *      {
 *          if(name == m_name)
 *              return;
 *
 *          m_name = name;
 *      }
 *
 * private:
 *      int m_id;
 *      QString m_name;
 * }
 */

class EloquentBuilder;
class Connection;
class ModelPrivate;
class Model : public QObject//, public HasRelationship
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Model)
public:
    Q_INVOKABLE explicit Model(QObject *parent = nullptr);

    // Relation convert to Model
    // eg: User user = Post::find(1).user();
    Model(const Relation &relation, QObject *parent = nullptr);

    virtual ~Model();

    static void all(); // return a Collection<Model>
    static void destroy(const QList<int> &ids);
    static void with();
    static void query();
    static void on();

    // TODO: convert singular name to plural name
    //! default return class name (lower)
    virtual QString table() const;

    //! the model's primary key name. default is "id"
    virtual QString primaryKey() const;

    //! the model's primary key value
    QVariant primaryValue() const;

    //! the default foreign key name for the model. default is <classname>_id
    QString foreignKey() const;

    virtual bool save();
    virtual bool update(const QVariantMap &attributes);
    virtual bool del();

    virtual Model *fill(const QVariantMap &attributes);
    virtual Model *foreFill(const QVariantMap &attributes);

    virtual QStringList fillable() const;
    void setFillable(const QStringList &fillable);
    bool isFillable(const QString &key) const;

    virtual QVariantMap attributes() const;
    virtual QVariant attribute(const QString &key) const;
    virtual void setAttribute(const QString &key, const QVariant &value);
    QJsonObject attributesToJson() const;

    // TODO: ModelProperty instead of QMetaProperty
    // metaobject properties mapping to model attributes
    // Model:key - MetaProperty => table: column - Field
    /**
     * ["title"] => {
     *		"index": 1,
     *		"name": "title",
     *		"type": QVariant(QString),
     *		"typeName": "QString",
     *		"value": QVariant(...),
     *		"defaultValue": "",
     *		"table": "",
     *		"auto": false,
     *		"signal": "nameChanged",
     *		"slot": "setTitle",
     *		"method": "setTitle",
     *		"auto": false,
     *		"readable": true,
     *		"writable": true,
     *		"resetable": false,
     *		"valid": true,
     *		"revision": 0,
     *		"constant": false,
     *		"stored": true,
     *		"class": "Model",
     *		"superClass": "QObject",
     * },
     * ["other"] => {...}
     *
     */
    QHash<QString, QMetaProperty> metaProperty() const;

    /**
     * {
     * 	"id": {"index":0, "name":"id", "type": 1, "typeName":"int", ...}
     * 	"title": {"index":1, "name":"title", "type": 2, "typeName":"QString", ...}
     * 	"create_at": {"index":2, "name":"create_at", "type": 3, "typeName":"QDateTime", ...}
     *	...
     * }
     */
    QJsonObject metaPropertyToJson() const;

    // TODO: static metaobject including pk fk table ...

    Connection *connection() const;
    void setConnection(Connection *connection);
    EloquentBuilder newQuery() const;

    void dump();

private:
    // only one copy always while Model convert to eg:User Objects
    QExplicitlySharedDataPointer<ModelPrivate> d_ptr;
};

#endif // MODEL_H
