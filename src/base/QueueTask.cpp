#include "QueueTask.h"
#include "QueueTask_p.h"

#include <QCoreApplication>
#include <QMutexLocker>

QueueTask::QueueTask(const QString &name)
    : d_ptr(new QueueTaskPrivate(this))
{
    d_ptr->name = name;
}

QueueTask::~QueueTask()
{

}

void QueueTask::progress(qint64 progress, qint64 total) // update progress in the same TashQueue thread
{
    Q_UNUSED(progress)
    Q_UNUSED(total)
}

QString QueueTask::name() const
{
    Q_D(const QueueTask);
    return d->name;
}

void QueueTaskSchedulerPrivate::wakeThread()
{
    if(!taskThread)
    {
        QueueTaskScheduler *q = qobject_cast<QueueTaskScheduler *>(q_func());
        taskThread = new QThread;
        worker = new QueueTaskWorker(this);
        worker->moveToThread(taskThread);

        QObject::connect(q, &QueueTaskScheduler::taskAdded, worker, &QueueTaskWorker::processTask);
        QObject::connect(worker, &QueueTaskWorker::taskProcessed, this, &QueueTaskSchedulerPrivate::updateTask);

        taskThread->start();
    }

    if(exitTimer)
        exitTimer->stop();
}

void QueueTaskSchedulerPrivate::updateTask()
{
    Q_Q(QueueTaskScheduler);
    do
    {
        // handle finished queue
        QueueTask::Pointer task;
        {
            QMutexLocker lock(&completedMutex);
            if(completedTasks.isEmpty()) break;
            task = completedTasks.front();
            completedTasks.pop_front();
        }
        task->finish();
        emit q->processed(task);
    } while (true);

    if(exitTimer)
    {
        QMutexLocker lock(&penddingMutex);
        if(penddingTasks.isEmpty())
            exitTimer->start();
    }
}

/**
 * @brief QueueTaskScheduler::QueueTaskScheduler
 * @param parent
 * @param timeout
 */
QueueTaskScheduler::QueueTaskScheduler(QObject *parent, qint32 timeout)
    : QObject(parent), d(new QueueTaskSchedulerPrivate(this))
{
    Q_D(QueueTaskScheduler);
    if(timeout > 0)
    {
        d->exitTimer = new QTimer(this);
        connect(d->exitTimer, SIGNAL(timeout()), this, SLOT(stop()));
        d->exitTimer->setSingleShot(true);
        d->exitTimer->setInterval(timeout);
    }
}

QueueTaskScheduler::~QueueTaskScheduler()
{
    Q_D(QueueTaskScheduler);
    stop();
    d->exitTimer->deleteLater();
}

TaskId QueueTaskScheduler::append(QueueTask *task)
{
    return this->append(QueueTask::Pointer(task));
}

TaskId QueueTaskScheduler::append(QueueTask::Pointer task)
{
    Q_D(QueueTaskScheduler);
    {
        QMutexLocker lock(&d->penddingMutex);
        d->penddingTasks.append(task);
    }

    // wake up thread if work thread is sleeping
    d->wakeThread();

    emit taskAdded();
    return task->id();
}

void QueueTaskScheduler::append(const QueueTaskList &tasks)
{
    Q_D(QueueTaskScheduler);
    {
        QMutexLocker lock(&d->penddingMutex);
        d->penddingTasks.append(tasks);
    }

    // wake up work thread
    d->wakeThread();

    emit taskAdded();
}

void QueueTaskScheduler::cancel(TaskId id)
{
    Q_D(QueueTaskScheduler);
    {
        QMutexLocker lock(&d->penddingMutex);
        for(auto task : d->penddingTasks)
        {
            if(task->id() == id)
            {
                d->penddingTasks.removeOne(task);
                return;
            }
        }
    }

    QMutexLocker lock(&d->completedMutex);
    for(auto task : d->completedTasks)
    {
        if(task->id() == id)
        {
            d->completedTasks.removeOne(task);
            return;
        }
    }
}

void QueueTaskScheduler::stop()
{
    Q_D(QueueTaskScheduler);
    if(d->taskThread)
    {
        d->taskThread->requestInterruption();
        d->taskThread->quit();
        d->taskThread->wait();
        delete d->worker;
        delete d->taskThread;
        d->worker = nullptr;
        d->taskThread = nullptr;
    }

    d->penddingTasks.clear();
    d->completedTasks.clear();
}


/**
 * @brief TaskQueueWorker::TaskQueueWorker
 * @param queue
 */
QueueTaskWorker::QueueTaskWorker(QueueTaskSchedulerPrivate *scheduler_p)
    : QObject(), d(new QueueTaskWorkerPrivate(this))
{
    d->scheduler_p = scheduler_p;
}

QueueTaskWorker::~QueueTaskWorker()
{

}

void QueueTaskWorker::processTask()
{
    Q_D(QueueTaskWorker);
    if(d->isBusy)
        return ;

    d->isBusy = true;

    bool hasLeft = false;
    do
    {
        QueueTask::Pointer task;
        {
            QMutexLocker lock(&d->scheduler_p->penddingMutex);
            hasLeft = !d->scheduler_p->penddingTasks.isEmpty();
            if(hasLeft)
                task = d->scheduler_p->penddingTasks.front();
        }

        if(task)
        {
            // take a long time to process task
            task->process();

            bool processDone = false;
            {
                QMutexLocker penddingLock(&d->scheduler_p->penddingMutex);
                hasLeft = !d->scheduler_p->completedTasks.isEmpty();
                if(hasLeft && d->scheduler_p->penddingTasks.front() == task)
                {
                    d->scheduler_p->penddingTasks.pop_front();
                    hasLeft = !d->scheduler_p->penddingTasks.isEmpty();

                    QMutexLocker finishedLock(&d->scheduler_p->completedMutex);
                    processDone = d->scheduler_p->completedTasks.isEmpty();
                    d->scheduler_p->completedTasks.push_back(task);
                }
            }

            if(processDone)
                emit taskProcessed();
        }

        QCoreApplication::processEvents();
        thread()->msleep(10);

    } while (hasLeft && !thread()->isInterruptionRequested());

    d->isBusy = false;
    emit finished();
}
