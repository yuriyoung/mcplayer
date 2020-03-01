#include "MediaDiscoverer.h"

#include <QMutexLocker>
#include <QQueue>
#include <QTimer>
#include <QDir>
#include <QDirIterator>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcMediaDiscoverer, "mcplayer.MediaDiscoverer")

class MediaDiscovererPrivate
{
    Q_DECLARE_PUBLIC(MediaDiscoverer)
public:
    MediaDiscovererPrivate(MediaDiscoverer *q);
    void enqueue(const QString &entryPoint, int type);

    MediaDiscoverer *q_ptr = nullptr;
};

MediaDiscovererPrivate::MediaDiscovererPrivate(MediaDiscoverer *q)
    : q_ptr(q)
{

}

void MediaDiscovererPrivate::enqueue(const QString &entryPoint, int type)
{
    Q_UNUSED(entryPoint)
    Q_UNUSED(type)

    // TODO: the discovering task must be stop before remove the same entry
    // TODO: the ban task must be stop before unban the same entry

    // scan entries with a sub thread
}

/**
 * @brief MediaDiscoverer::MediaDiscoverer
 * @param parent
 */
MediaDiscoverer::MediaDiscoverer(QObject *parent)
    : QObject(parent), d(new MediaDiscovererPrivate(this))
{

}

MediaDiscoverer::~MediaDiscoverer()
{
    this->stop();
}

void MediaDiscoverer::add(const QString &entryPoint)
{
    d->enqueue(entryPoint, 0);
}

void MediaDiscoverer::remove(const QString &entryPoint)
{
    d->enqueue(entryPoint, 1);
}

void MediaDiscoverer::reload()
{
    d->enqueue("", -1);
}

void MediaDiscoverer::reload(const QString &entryPoint)
{
    d->enqueue(entryPoint, 2);
}

void MediaDiscoverer::ban(const QString &entryPoint)
{
    d->enqueue(entryPoint, 3);
}

void MediaDiscoverer::unban(const QString &entryPoint)
{
    d->enqueue(entryPoint, 4);
}

bool MediaDiscoverer::discover(const QString &path)
{
    this->add(path);
    return false;
}

void MediaDiscoverer::start()
{

}

void MediaDiscoverer::stop()
{

}

