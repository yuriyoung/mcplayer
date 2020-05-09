#ifndef COLUMN_H
#define COLUMN_H

#include <QMetaType>
#include <QVariant>
#include <QString>
#include <QDebug>

class ColumnPrivate;
class Column
{
public:
    explicit Column(const QString& name = QString(), int type = QMetaType::Char);
    Column(const QString& name, int typeId, const QString &table);
    Column(const Column &other);
    ~Column();

    Column &operator=(const Column& other);
    bool operator==(const Column& other) const;
    inline bool operator!=(const Column &other) const { return !operator==(other); }

    void setValue(const QVariant& value);
    inline QVariant value() const;

    void setName(const QString& name);
    QString name() const;

    void setTableName(const QString &tableName);
    QString tableName() const;

    void setType(int type);
    int type() const;
    QString typeName() const;

    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    void setAutoIncreament(bool autoVal);
    bool isAutoIncreament() const;

    void setAutoValue(bool autoVal);
    bool isAutoValue() const;

    void setRequired(bool required);
    bool isRequired() const;

    void setDefaultValue(const QVariant &value);
    QVariant defaultValue() const;

    void setLength(int fieldLength);
    int length() const;

    void setPrecision(int precision);
    int precision() const;

    void setTotal(int total);
    int total() const;

    void setPlaces(int places);
    int places() const;

//    void setAllowed(const QStringList &allowed);
//    QStringList allowed() const;

    void setNullable(bool nullable = true);
    bool nullable() const;

    void setNull();
    bool isNull() const;

    bool isValid() const;

private:
    QVariant val;
    ColumnPrivate *d;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug, const Column &);
#endif

#endif // COLUMN_H
