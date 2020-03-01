#include "LocalMediaPlaylistProvider.h"

#include <QRandomGenerator>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcLocalMediaPlaylistProvider, "mcplayer.LocalMediaPlaylistProvider")

class LocalMediaPlaylistProviderPrivate
{
public:
    QList<Media> resources;
};

LocalMediaPlaylistProvider::LocalMediaPlaylistProvider(QObject *parent)
    : MediaPlaylistProvider(parent)
    , d_ptr(new LocalMediaPlaylistProviderPrivate)
{

}

LocalMediaPlaylistProvider::~LocalMediaPlaylistProvider()
{

}

bool LocalMediaPlaylistProvider::load(QIODevice *device, const char *format)
{
    Q_UNUSED(device)
    Q_UNUSED(format)
    qDebug(lcLocalMediaPlaylistProvider) << "load playlist";

    // TODO:

    return false;
}

bool LocalMediaPlaylistProvider::save(QIODevice *device, const char *format)
{
    Q_UNUSED(device)
    Q_UNUSED(format)
    qDebug(lcLocalMediaPlaylistProvider) << "save playlist";

    // TODO:

    return false;
}

bool LocalMediaPlaylistProvider::isReadOnly() const
{
    return false;
}

int LocalMediaPlaylistProvider::mediaCount() const
{
    Q_D(const LocalMediaPlaylistProvider);
    return d->resources.size();
}

Media LocalMediaPlaylistProvider::media(int index) const
{
    Q_D(const LocalMediaPlaylistProvider);
    return d->resources.value(index);
}

bool LocalMediaPlaylistProvider::addMedia(const Media &media)
{
    Q_D(LocalMediaPlaylistProvider);
    qInfo(lcLocalMediaPlaylistProvider) << "addMedia" << media.canonicalUrl();

    int pos = this->mediaCount();
    emit mediaAboutToInserted(pos, pos);
    d->resources.append(media);
    emit mediaInserted(pos, pos);

    return true;
}

bool LocalMediaPlaylistProvider::addMedia(const QList<Media> &items)
{
    Q_D(LocalMediaPlaylistProvider);
    qInfo(lcLocalMediaPlaylistProvider) << "addMedia list";

    if(items.isEmpty())
        return true;

    int start = mediaCount();
    int end = start + items.count() - 1;

    emit mediaAboutToInserted(start, end);
    d->resources.append(items);
    emit mediaInserted(start, end);

    return true;
}

bool LocalMediaPlaylistProvider::insertMedia(int index, const Media &media)
{
    Q_D(LocalMediaPlaylistProvider);
    qInfo(lcLocalMediaPlaylistProvider) << "insertMedia";

    emit mediaAboutToInserted(index, index);
    d->resources.insert(index, media);
    emit mediaInserted(index, index);

    return true;
}

bool LocalMediaPlaylistProvider::insertMedia(int index, const QList<Media> &items)
{
    Q_D(LocalMediaPlaylistProvider);
    qInfo(lcLocalMediaPlaylistProvider) << "insertMedia list";

    if (items.isEmpty())
        return true;

    const int last = index + items.count()-1;
    emit mediaAboutToInserted(index, last);
    for (int i = 0; i < items.count(); ++i)
    {
        d->resources.insert(index + i, items.at(i));
    }
    emit mediaInserted(index, last);

    return true;
}

bool LocalMediaPlaylistProvider::moveMedia(int from, int to)
{
    Q_D(LocalMediaPlaylistProvider);
    qInfo(lcLocalMediaPlaylistProvider) << "moveMedia";

    Q_ASSERT(from >= 0 && from < mediaCount());
    Q_ASSERT(to >= 0 && to < mediaCount());

    if (from == to)
        return false;

    const Media media = d->resources.at(from);

    return removeMedia(from, from) && insertMedia(to, media);
}

bool LocalMediaPlaylistProvider::removeMedia(int pos)
{
    Q_D(LocalMediaPlaylistProvider);
    qInfo(lcLocalMediaPlaylistProvider) << "removeMedia";

    emit mediaAboutToRemoved(pos, pos);
    d->resources.removeAt(pos);
    emit mediaRemoved(pos, pos);

    return true;
}

bool LocalMediaPlaylistProvider::removeMedia(int start, int end)
{
    Q_D(LocalMediaPlaylistProvider);
    qInfo(lcLocalMediaPlaylistProvider) << "removeMedia";

    Q_ASSERT(start >= 0);
    Q_ASSERT(start <= end);
    Q_ASSERT(end < mediaCount());

    emit mediaAboutToRemoved(start, end);
    auto begin = d->resources.begin() + start;
    auto last = d->resources.begin() + end + 1;
    d->resources.erase(begin, last);
    emit mediaRemoved(start, end);

    return true;
}

bool LocalMediaPlaylistProvider::clear()
{
    qInfo(lcLocalMediaPlaylistProvider) << "clear";

    int count = mediaCount();
    if (count > 0)
    {
        int lastPos = count - 1;
        removeMedia(0, lastPos);
    }

    return true;
}

void LocalMediaPlaylistProvider::shuffle()
{
    Q_D(LocalMediaPlaylistProvider);
    qInfo(lcLocalMediaPlaylistProvider) << "shuffle";

    int count = mediaCount();
    if (count > 0)
    {
        QList<Media> resources;

        while (mediaCount() > 0)
        {
            resources.append(d->resources.takeAt(QRandomGenerator::global()->bounded(d->resources.size())));
        }

        d->resources = resources;
        emit mediaChanged(0, mediaCount()-1);
    }
}

