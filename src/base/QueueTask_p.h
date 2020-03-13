#ifndef QUEUETASK_P_H
#define QUEUETASK_P_H

#include "QueueTask.h"
#include <QThread>
#include <QMutex>
#include <QTimer>

class QueueTaskWorker;
class QueueTaskPrivate
{
    Q_DECLARE_PUBLIC(QueueTask)
public:
    QueueTaskPrivate(QueueTask *q) : q_ptr(q) {}
    QString name;

protected:
    QueueTask *q_ptr;
};

class QueueTaskSchedulerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QueueTaskScheduler)
    friend class TaskQueueWorkerPrivate;
public:
    QueueTaskSchedulerPrivate(QueueTaskScheduler *q) : q_ptr(q) {}

    void wakeThread();

    QueueTaskList penddingTasks, completedTasks;
    QMutex penddingMutex, completedMutex;
    QThread *taskThread = nullptr;
    QueueTaskWorker *worker = nullptr;
    QTimer *exitTimer = nullptr;

public slots:
    void updateTask();

protected:
    QueueTaskScheduler *q_ptr;
};

class QueueTaskWorkerPrivate
{
    Q_DECLARE_PUBLIC(QueueTaskWorker)
public:
    explicit QueueTaskWorkerPrivate(QueueTaskWorker *q) : q_ptr(q) {}

    QueueTaskSchedulerPrivate *scheduler_p;
    bool isBusy;

protected:
    QueueTaskWorker *q_ptr = nullptr;
};

class QueueTaskWorker : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(d, QueueTaskWorker)
public:
    explicit QueueTaskWorker(QueueTaskSchedulerPrivate *scheduler_p);
    ~QueueTaskWorker();

signals:
    void taskProcessed();
    void finished();

public slots:
    void processTask();

protected:
    QScopedPointer<QueueTaskWorkerPrivate> d;
};

#endif // QUEUETASK_P_H
