#include "AsyncTask.h"

#include <QThreadPool>
#include <QElapsedTimer>
#include <QAtomicInt>
#include <QWaitCondition>
#include <QMutex>
#include <QDebug>

class AsyncTaskPrivate
{
public:
    enum State
    {
        IdleState   = 0x00,
        Started   = 0x01,
        Running   = 0x02,
        Finished  = 0x04,
        Canceled  = 0x08
    };

    AsyncTaskPrivate(AsyncTask *q_ptr) : q(q_ptr) {}
    void prepare()
    {
        elapsTimer.start();
    }

    QAtomicInt state = IdleState;

    QMutex mutex;
    QWaitCondition waitCondition;
    QElapsedTimer elapsTimer;
    AsyncTask *q = nullptr;
};

AsyncTask::AsyncTask()
    : QObject(nullptr), QRunnable(), d(new AsyncTaskPrivate(this))
{
    setAutoDelete(false); //! importand: do not auto delete by threadpool
    qDebug() << this->name() << "create";
}

AsyncTask::~AsyncTask()
{
    wait();
    qDebug() << id() << name() <<"free";
}

void AsyncTask::run()
{
    QMutexLocker lock(&d->mutex);

    if (isCanceled())
    {
        // report finished
        return;
    }

    d->state.testAndSetRelaxed(AsyncTaskPrivate::IdleState, AsyncTaskPrivate::Running);
    emit started();

    // do task as long time
    process();

    if(d->state.load() & AsyncTaskPrivate::Canceled)
    {
        emit canceled();
    }
    else
    {
        d->state.testAndSetRelaxed(AsyncTaskPrivate::Running, AsyncTaskPrivate::Finished);
        emit finished();
    }
}

void AsyncTask::interrupt()
{
    if (d->state.load() & AsyncTaskPrivate::Canceled)
    {
        qDebug() << this->name() <<  "trying to request interrupt failed." << QThread::currentThread();
        return;
    }

    qDebug() << this->name() <<  "request interrupt." << QThread::currentThread();

    d->state.testAndSetRelaxed(AsyncTaskPrivate::Running, AsyncTaskPrivate::Canceled);
}

bool AsyncTask::isCanceled() const
{
    return d->state.load() & AsyncTaskPrivate::Canceled;
}

bool AsyncTask::isRunning() const
{
    return d->state.load() & AsyncTaskPrivate::Running;
}

bool AsyncTask::wait(quint32 timeout)
{
    if(0 >= timeout)
    {
        QMutexLocker lock(&d->mutex);
        return true;
    }

    if(d->mutex.tryLock(static_cast<int>(timeout)))
        d->mutex.unlock();

//    QMutexLocker lock(&d->mutex);
//    while (isRunning())
//    {
//        d->waitCondition.wait(&d->mutex, timeout);
//    }

    return true;
}

qint64 AsyncTask::elapsed() const
{
    return d->elapsTimer.isValid() ? d->elapsTimer.elapsed() : 0;
}

QString AsyncTask::name() const
{
    return QString("AsyncTask");
}

TaskId AsyncTask::id() const
{
    return TaskId(this);
}

class AsyncTaskSchedulerPrivate
{
public:
    QThreadPool *threadPool = nullptr;
    QList<AsyncTask::Pointer> tasks;
};

static AsyncTaskScheduler *taskScheduler = nullptr;
/**
 * @brief AsyncTaskScheduler::AsyncTaskScheduler
 * @param parent
 */
AsyncTaskScheduler::AsyncTaskScheduler(QObject *parent)
    : QObject(parent), d(new AsyncTaskSchedulerPrivate)
{
    d->threadPool = new QThreadPool(this);
    qDebug() << "AsyncTaskScheduler::AsyncTaskScheduler()";

    taskScheduler = this;
}

AsyncTaskScheduler::~AsyncTaskScheduler()
{
    qDebug() << "AsyncTaskScheduler::~AsyncTaskScheduler()";
}

AsyncTaskScheduler *AsyncTaskScheduler::instance()
{
    if(!taskScheduler)
        taskScheduler = new AsyncTaskScheduler();
    return taskScheduler;
}

void AsyncTaskScheduler::start(AsyncTask::Pointer task, AsyncTaskScheduler::Priority priority)
{
    d->tasks.append(task);
    task->d->prepare();

    QWeakPointer<AsyncTask> weakPtr = task;
    connect(task.data(), &AsyncTask::finished, this, [this, weakPtr]()
    {
        d->tasks.removeOne(weakPtr);
        emit tasksChanged();
    });
    connect(task.data(), &AsyncTask::canceled, this, [this, weakPtr]()
    {
        d->tasks.removeOne(weakPtr);
        emit tasksChanged();
    });

    d->threadPool->start(task.data(), priority);
    emit tasksChanged();
}

void AsyncTaskScheduler::stop(AsyncTask::Pointer task)
{
    if(task)
    {
        task->interrupt();
        task->wait();
    }
}

void AsyncTaskScheduler::stop(TaskId id)
{
    foreach (auto task, d->tasks)
    {
        if(task->id() == id)
            this->stop(task);
    }
}

bool AsyncTaskScheduler::hasActiveTask() const
{
    return !d->tasks.isEmpty();
}

int AsyncTaskScheduler::activeTaskCount() const
{
    return d->tasks.count();
}
