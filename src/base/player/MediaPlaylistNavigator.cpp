#include "MediaPlaylistNavigator.h"
#include "LocalMediaPlaylistProvider.h"

#include <QRandomGenerator>
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(lcMediaPlaylistNavigator, "mcplayer.MediaPlaylistNavigator")

Q_GLOBAL_STATIC(LocalMediaPlaylistProvider, defaultMediaPlaylist)

class MediaPlaylistNavigatorPrivate
{
public:
    int nextItemPos(int steps = 1) const;
    int previousItemPos(int steps = 1) const;

    MediaPlaylistProvider *playlist = nullptr;
    int currentPos = -1;
    int lastValidPos = -1;
    MediaPlaylist::PlaybackMode playbackMode = MediaPlaylist::Sequential;
    Media currentItem;

    mutable int randomPositionsOffset = -1;
    mutable QList<int> randomModePositions;
};

int MediaPlaylistNavigatorPrivate::nextItemPos(int steps) const
{
    if (playlist->mediaCount() == 0)
        return -1;

    if (steps == 0)
        return currentPos;

    switch (playbackMode)
    {
    case MediaPlaylist::CurrentItemOnce:
        return -1;
    case MediaPlaylist::CurrentItemRepeat:
        return currentPos;
    case MediaPlaylist::Sequential:
    {
        int nextPos = currentPos + steps;
        return nextPos < playlist->mediaCount() ? nextPos : -1;
    }
    case MediaPlaylist::Loop:
        return (currentPos + steps) % playlist->mediaCount();
    case MediaPlaylist::Random:
    {
        if(randomPositionsOffset == -1)
        {
            randomModePositions.clear();
            randomModePositions.append(currentPos);
            randomPositionsOffset = 0;
        }

        while (randomModePositions.size() < randomPositionsOffset + steps + 1)
            randomModePositions.append(-1);
        int pos = randomModePositions[randomPositionsOffset + steps];
        if(pos < 0 || pos >= playlist->mediaCount())
        {
            pos = QRandomGenerator::global()->bounded(playlist->mediaCount());
            randomModePositions[randomPositionsOffset] = pos;
        }

        return pos;
    }
    case MediaPlaylist::Heartbeat:
        // TODO:
        break;
    }

    return -1;
}

int MediaPlaylistNavigatorPrivate::previousItemPos(int steps) const
{
    if (playlist->mediaCount() == 0)
        return -1;

    if (steps == 0)
        return currentPos;

    switch (playbackMode)
    {
    case MediaPlaylist::CurrentItemOnce:
        return -1;
    case MediaPlaylist::CurrentItemRepeat:
        return currentPos;
    case MediaPlaylist::Sequential:
    {
        int prevPos = currentPos == -1 ? playlist->mediaCount() - steps : currentPos - steps;

        return prevPos >= 0 ? prevPos : -1;
    }
    case MediaPlaylist::Loop:
    {
        int prevPos = currentPos - steps;
        while (prevPos < 0)
            prevPos += playlist->mediaCount();

        return prevPos;
    }
    case MediaPlaylist::Random:
    {
        if(randomPositionsOffset == -1)
        {
            randomModePositions.clear();
            randomModePositions.append(currentPos);
            randomPositionsOffset = 0;
        }

        while (randomPositionsOffset - steps < 0)
        {
            randomModePositions.prepend(-1);
            randomPositionsOffset++;
        }

        int pos = randomModePositions[randomPositionsOffset - steps];
        if(pos < 0 || pos >= playlist->mediaCount())
        {
            pos = QRandomGenerator::global()->bounded(playlist->mediaCount());
            randomModePositions[randomPositionsOffset - steps] = pos;
        }

        return pos;
    }
    case MediaPlaylist::Heartbeat:
        // TODO:
        break;
    }

    return -1;
}

/**
 * @brief MediaPlaylistNavigator::MediaPlaylistNavigator
 * @param playlist
 * @param parent
 */
MediaPlaylistNavigator::MediaPlaylistNavigator(QObject *parent)
    : MediaPlaylistNavigator(defaultMediaPlaylist(), parent)
{
    qInfo(lcMediaPlaylistNavigator) << "create MediaPlaylistNavigator";
}

MediaPlaylistNavigator::MediaPlaylistNavigator(MediaPlaylistProvider *playlist, QObject *parent)
    : QObject(parent), d_ptr(new MediaPlaylistNavigatorPrivate)
{
    setPlaylist(playlist);
}

MediaPlaylistNavigator::~MediaPlaylistNavigator()
{

}

MediaPlaylistProvider *MediaPlaylistNavigator::playlist() const
{
    Q_D(const MediaPlaylistNavigator);
    return d->playlist;
}

void MediaPlaylistNavigator::setPlaylist(MediaPlaylistProvider *pls)
{
    Q_D(MediaPlaylistNavigator);
    qDebug(lcMediaPlaylistNavigator) << "setPlaylist" << pls->mediaCount();

    if (d->playlist == pls)
        return;

    if (d->playlist)
    {
        d->playlist->disconnect(this);
    }

    // assign to shared readonly null playlist if no specify playlist provider
    d->playlist = pls ? pls : defaultMediaPlaylist();

    connect(d->playlist, &MediaPlaylistProvider::mediaInserted, this, &MediaPlaylistNavigator::mediaInserted);
    connect(d->playlist, &MediaPlaylistProvider::mediaRemoved, this, &MediaPlaylistNavigator::mediaRemoved);
    connect(d->playlist, &MediaPlaylistProvider::mediaChanged, this, &MediaPlaylistNavigator::mediaChanged);

    d->randomPositionsOffset = -1;
    d->randomModePositions.clear();

    if (d->currentPos != -1)
    {
        d->currentPos = -1;
        emit currentIndexChanged(-1);
    }

    if (!d->currentItem.isNull())
    {
        d->currentItem = Media();
        emit activated(d->currentItem); //stop playback
    }
}

MediaPlaylist::PlaybackMode MediaPlaylistNavigator::playbackMode() const
{
    Q_D(const MediaPlaylistNavigator);
    return d->playbackMode;
}

Media MediaPlaylistNavigator::currentItem() const
{
    Q_D(const MediaPlaylistNavigator);
    return itemAt(d->currentPos);
}

Media MediaPlaylistNavigator::nextItem(int steps) const
{
//    Q_D(const MediaPlaylistNavigator);
    return itemAt(nextIndex(steps));
}

Media MediaPlaylistNavigator::previousItem(int steps) const
{
//    Q_D(const MediaPlaylistNavigator);
    return itemAt(previousIndex(steps));
}

Media MediaPlaylistNavigator::itemAt(int position) const
{
    Q_D(const MediaPlaylistNavigator);
    return d->playlist->media(position);
}

int MediaPlaylistNavigator::currentIndex() const
{
    Q_D(const MediaPlaylistNavigator);
    return d->currentPos;
}

int MediaPlaylistNavigator::nextIndex(int steps) const
{
    Q_D(const MediaPlaylistNavigator);
    return d->nextItemPos(steps);
}

int MediaPlaylistNavigator::previousIndex(int steps) const
{
    Q_D(const MediaPlaylistNavigator);
    return d->previousItemPos(steps);
}

void MediaPlaylistNavigator::next()
{
    Q_D(MediaPlaylistNavigator);
    qInfo(lcMediaPlaylistNavigator) << "next";

    int nextPos = d->nextItemPos();

    if (playbackMode() == MediaPlaylist::Random)
        ++d->randomPositionsOffset;

    jump(nextPos);
}

void MediaPlaylistNavigator::previous()
{
    Q_D(MediaPlaylistNavigator);
    qInfo(lcMediaPlaylistNavigator) << "previous";

    int prevPos = d->previousItemPos();
    if (playbackMode() == MediaPlaylist::Random)
        --d->randomPositionsOffset;

    jump(prevPos);
}

void MediaPlaylistNavigator::jump(int position)
{
    Q_D(MediaPlaylistNavigator);
    if (position < -1 || position >= d->playlist->mediaCount())
        position = -1;

    if (position != -1)
        d->lastValidPos = position;

    if (playbackMode() == MediaPlaylist::Random)
    {
        if (d->randomModePositions[d->randomPositionsOffset] != position)
        {
            d->randomModePositions.clear();
            d->randomModePositions.append(position);
            d->randomPositionsOffset = 0;
        }
    }

    d->currentItem = position != -1 ? d->playlist->media(position) : Media();

    if (position != d->currentPos)
    {
        d->currentPos = position;
        emit currentIndexChanged(d->currentPos);
        emit surroundingItemsChanged();
    }

    qInfo(lcMediaPlaylistNavigator) << "jump media: "
                                    <<  position << " - "
                                    << d->currentItem.canonicalUrl();

    emit activated(d->currentItem);
}

void MediaPlaylistNavigator::setPlaybackMode(MediaPlaylist::PlaybackMode mode)
{
    Q_D(MediaPlaylistNavigator);
    if (d->playbackMode == mode)
        return;

    if (mode == MediaPlaylist::Random)
    {
        d->randomPositionsOffset = 0;
        d->randomModePositions.append(d->currentPos);
    }
    else if (d->playbackMode == MediaPlaylist::Random)
    {
        d->randomPositionsOffset = -1;
        d->randomModePositions.clear();
    }

    d->playbackMode = mode;

    emit playbackModeChanged(mode);
    emit surroundingItemsChanged();
}

void MediaPlaylistNavigator::mediaInserted(int start, int end)
{
    Q_D(MediaPlaylistNavigator);

    if (d->currentPos >= start)
    {
        d->currentPos = end - start + 1;
        jump(d->currentPos);
    }

    emit surroundingItemsChanged();
}

void MediaPlaylistNavigator::mediaRemoved(int start, int end)
{
    Q_D(MediaPlaylistNavigator);
    if (d->currentPos > end)
    {
        d->currentPos = d->currentPos - end - start + 1;
        jump(d->currentPos);
    }
    else if (d->currentPos >= start)
    {
        //current item was removed
        d->currentPos = qMin(start, d->playlist->mediaCount() - 1);
        jump(d->currentPos);
    }

    emit surroundingItemsChanged();
}

void MediaPlaylistNavigator::mediaChanged(int start, int end)
{
    Q_D(MediaPlaylistNavigator);
    if (d->currentPos >= start && d->currentPos <= end)
    {
        Media src = d->playlist->media(d->currentPos);
        if (src != d->currentItem)
        {
            d->currentItem = src;
            emit activated(src);
        }
    }

    emit surroundingItemsChanged();
}
