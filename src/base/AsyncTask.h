#ifndef ASYNCTASK_H
#define ASYNCTASK_H

#include <QObject>
#include <QThread>
#include <QRunnable>
#include <QSharedPointer>

/*!
 * ConcurrentTask / AsyncTask
 *
 */

typedef void *TaskId;

class AsyncTaskScheduler;
class AsyncTaskPrivate;
class AsyncTask : public QObject, public QRunnable
{
    Q_OBJECT
    friend class AsyncTaskScheduler;
public:
    using Pointer = QSharedPointer<AsyncTask>;

    explicit AsyncTask();
    ~AsyncTask() override;

    bool isCanceled() const;
    bool isRunning() const;
    bool wait(quint32 timeout = 0);

    /*!
     * eturns the number of milliseconds since this run was last started.
     */
    qint64 elapsed() const;
    virtual QString name() const;
    TaskId id() const;

signals:
    void started();
    void canceled();
    void finished();

public slots:
    virtual void interrupt();

protected:
    virtual void process() = 0;
    void run() override final;

private:
    QScopedPointer<AsyncTaskPrivate> d;
};

typedef  QSharedPointer<AsyncTask> TaskPtr;

class AsyncTaskSchedulerPrivate;
class AsyncTaskScheduler : public QObject
{
    Q_OBJECT
public:
    enum Priority
    {
        IdlePriority,   // scheduled only when no other threads are running.
        LowestPriority, // scheduled less often than LowPriority.
        LowPriority,    // scheduled less often than NormalPriority.
        NormalPriority, // the default priority of the operating system.
        HighPriority,   // scheduled more often than NormalPriority.
        HighestPriority,     // scheduled more often than HighPriority.
        TimeCriticalPriority,// scheduled as often as possible.
        InheritPriority     // (default) use the same priority as the creating thread.
    };

    explicit AsyncTaskScheduler(QObject *parent = nullptr);
    ~AsyncTaskScheduler();

    static AsyncTaskScheduler *instance();

    void start(AsyncTask::Pointer task, AsyncTaskScheduler::Priority priority = AsyncTaskScheduler::InheritPriority);
    void stop(AsyncTask::Pointer task);
    void stop(TaskId id);
    bool hasActiveTask() const;
    int activeTaskCount() const;

signals:
    void tasksChanged();
    void taskCaneled();

private:
    QScopedPointer<AsyncTaskSchedulerPrivate> d;
};


#endif // ASYNCTASK_H
