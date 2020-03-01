#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>

/**
 * @brief The Database class provider access sqlite database
 *
 * single thread write queue task
 * multiple thread read queue task
 *
 *
 */
class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);

signals:

public slots:
};

#endif // DATABASE_H
