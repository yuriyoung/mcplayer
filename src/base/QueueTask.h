#ifndef QUEUETASK_H
#define QUEUETASK_H

#include <QObject>
#include <QSharedPointer>

/*!
 *  QueueTask / SyncTask
 *
 */

typedef void *TaskId; // no interface, just id

class QueueTaskPrivate;
class QueueTask
{
    Q_DECLARE_PRIVATE(QueueTask)
public:
    using Pointer = QSharedPointer<QueueTask>;

    explicit QueueTask(const QString &name);
    virtual ~QueueTask();

    virtual void process() = 0; // is executed in a separate thread
    virtual void finish() = 0; // is executed in the same as TaskQueue thread
    virtual void progress(qint64 progress, qint64 total);

    TaskId id() const { return TaskId(this); }
    QString name() const;

protected:
    QScopedPointer<QueueTaskPrivate> d_ptr;
};

typedef QList<QueueTask::Pointer> QueueTaskList;

class QueueTaskSchedulerPrivate;
class QueueTaskScheduler : public QObject
{
Q_OBJECT
    Q_DISABLE_COPY(QueueTaskScheduler)
    Q_DECLARE_PRIVATE_D(d, QueueTaskScheduler)
public:
    explicit QueueTaskScheduler(QObject *parent = nullptr, qint32 timeout = 0);
    ~QueueTaskScheduler();

    TaskId append(QueueTask *task);
    TaskId append(QueueTask::Pointer task);
    void append(const QueueTaskList &tasks);
    void cancel(TaskId id);

signals:
    void taskAdded();
    void processed(QueueTask::Pointer task);

public slots:
    void stop();

private:
    QScopedPointer<QueueTaskSchedulerPrivate> d;
};

#endif // QUEUETASK_H
