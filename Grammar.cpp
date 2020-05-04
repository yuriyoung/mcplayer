#include "Grammar.h"
#include "Grammar_p.h"

#include <QDate>
#include <QDateTime>
#include <QDebug>

GrammarPrivate::GrammarPrivate(Grammar *q)
    : q_ptr(q)
{

}

GrammarPrivate::~GrammarPrivate()
{

}

Grammar::Grammar(QObject *parent)
    : Grammar(*new GrammarPrivate(this), parent)
{

}

Grammar::Grammar(GrammarPrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{

}

Grammar::~Grammar()
{

}

QStringList Grammar::wrapArray(const QStringList &values) const
{
    QStringList list;
    foreach(auto val, values)
    {
        list.append(this->wrap(val));
    }

    return list;
}

QString Grammar::wrapTable(const QString &table) const
{
    Q_D(const Grammar);
    // TODO: compute the value if it is a Expression
    // ...

    return this->wrap(d->tablePrefix + table, true);
}

QString Grammar::wrap(const QString &value, bool prefixAlias) const
{
    // TODO: value is a SQL Expression
    // ...

    // 如果被包装的值有一个列别名，我们需要将其分离出来
    // 这样我们就可以把表达式的每个片段封装起来
    // 然后使用“as”连接器将它们连接在一起。
    if(value.indexOf(" as ", 0, Qt::CaseInsensitive) != -1)
    {
        return this->wrapAliasedValue(value, prefixAlias);
    }

    return this->wrapSegments(value.split("."));
}

QString Grammar::wrapAliasedValue(const QString &value, bool prefixAlias) const
{
    Q_D(const Grammar);
    QStringList segments = value.split(QRegExp("\\s+as\\s+", Qt::CaseInsensitive));
    Q_ASSERT(segments.size() >= 2);

    if(prefixAlias)
    {
        segments[1] = d->tablePrefix + segments[1];
    }

    return this->wrap(segments[0]) + " as " + this->wrapValue(segments[1]);
}

QString Grammar::wrapSegments(const QStringList &segments) const
{
    QStringList list;
    int size = segments.size();
    for(int i = 0; i < size; ++i)
    {
        QString seg = segments[i];
        list.append(i == 0 && size > 1 ? wrapTable(seg) : wrapValue(seg));
    }
    return list.join(".");
}

QString Grammar::wrapValue(const QString &value) const
{
    if("*" != value)
    {
        QString val = value;
        return "\"" +  val.replace("\"", "\"\"") + "\"";
    }

    return value;
}

QString Grammar::columnize(const QString &columns) const
{
    QStringList list = columns.trimmed().split(",", QString::SkipEmptyParts);
    return this->columnize(list);
}

QString Grammar::columnize(const QStringList &columns) const
{
    QStringList list;
    foreach(auto col, columns)
    {
        list.append(this->wrap(col));
    }

    return list.join(", ");
}

/**
 * @brief Create query parameter place-holders for an array.
 * @param values
 * @return
 */
QString Grammar::parameterize(const QVariantList &values, bool prepared) const
{
    QStringList list;
    foreach(auto &val, values)
    {
        list.append(this->parameter(val, prepared));
    }
    return list.join(", ");
}

/**
 * @brief get query parameter place-holder for a value.
 * @param value
 * @return
 */
QString Grammar::parameter(const QVariant &value, bool prepared) const
{
//    return value.isValid() ? fromValue(value) : "?";
    return prepared ? "?" : fromValue(value);
}

QString Grammar::preparize(const QStringList &columns) const
{
    QStringList list;
    foreach(auto &col, columns)
        list << this->wrap(col) + " = ?";

    return list.join(", ");
}

QString Grammar::quoteString(const QStringList &values) const
{
    QStringList list;
    foreach(auto val, values)
    {
        list.append(quoteString(val));
    }

    return list.join(", ");
}

QString Grammar::quoteString(const QString &value) const
{
    return QString("'%1'").arg(value);
}

QString Grammar::dateFormat() const
{
    return QLatin1String("yyyy-MM-dd");
}

QString Grammar::datetimeFormat() const
{
    return QLatin1String("yyyy-MM-dd HH:mm:ss");
}

void Grammar::setTablePrefix(const QString &prefix)
{
    Q_D(Grammar);
    if(prefix == d->tablePrefix)
        return;
    d->tablePrefix = prefix;
}

QString Grammar::tablePrefix() const
{
    Q_D(const Grammar);
    return d->tablePrefix;
}

QString Grammar::fromValue(const QVariant &value) const
{
    const QLatin1String nullValue("NULL");
    if(value.isNull() || !value.isValid())
        return nullValue;

    QString result;
    switch (value.type())
    {
    case QVariant::Int:
    case QVariant::UInt:
        result = value.toString();
        break;
    case QVariant::Bool:
        result = QString::number(value.toBool());
        break;
    case QVariant::Date:
        result = value.toDate().isValid()
                ? quoteString(value.toDate().toString(dateFormat()))
                : nullValue;
        break;
    case QVariant::Time:
        result = value.toTime().isValid()
                ? quoteString(value.toTime().toString("HH:mm:ss"))
                : nullValue;
        break;
    case QVariant::DateTime:
        result = value.toDateTime().isValid()
                ? quoteString(value.toDateTime().toString(datetimeFormat()))
                : nullValue;
        break;
    case QVariant::Char:
    case QVariant::String:
    {
        QString r = value.toString();

        // TODO: trim white space from end ?

        // escape the "'" character
        r.replace(QLatin1Char('\''), QLatin1String("''"));
        result = quoteString(r);
        break;
    }
    case QVariant::StringList:
        result = quoteString(value.toStringList().join(" "));
        break;
    case QVariant::ByteArray:
    {
        QByteArray ba = value.toByteArray();
        QString res;
        static const char hexChars[] = "0123456789abcdef";
        for(int i = 0; i < ba.size(); ++i)
        {
            uchar c = static_cast<uchar>(ba[i]);
            res += QLatin1Char(hexChars[c >> 4]);
            res += QLatin1Char(hexChars[c & 0x0f]);
        }
        result = quoteString(res);
        break;
    }
    default: result = quoteString(value.toString()); break;
    }

    return result;
}
