#include "MediaPlaylistProvider.h"

#include <QRandomGenerator>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcMediaPlaylistProvider, "mcplayer.MediaPlaylistProvider")

/**
 * @brief MediaPlaylistProvider::MediaPlaylistProvider
 * @param parent
 */
MediaPlaylistProvider::MediaPlaylistProvider(QObject *parent)
    : QObject(parent)
{

}

MediaPlaylistProvider::~MediaPlaylistProvider()
{

}

bool MediaPlaylistProvider::load(const QNetworkRequest &request, const char *format)
{
    Q_UNUSED(request)
    Q_UNUSED(format)

    return false;
}

bool MediaPlaylistProvider::load(QIODevice *device, const char *format)
{
    Q_UNUSED(device)
    Q_UNUSED(format)

    return false;
}

bool MediaPlaylistProvider::save(const QUrl &location, const char *format)
{
    Q_UNUSED(location)
    Q_UNUSED(format)

    return false;
}

bool MediaPlaylistProvider::save(QIODevice *device, const char *format)
{
    Q_UNUSED(device)
    Q_UNUSED(format)

    return false;
}

bool MediaPlaylistProvider::isReadOnly() const
{
    return true;
}

bool MediaPlaylistProvider::addMedia(const Media &media)
{
    Q_UNUSED(media)
    return false;
}

bool MediaPlaylistProvider::addMedia(const QList<Media> &mediaList)
{
    for (const Media &media : mediaList)
    {
        if (!addMedia(media))
            return false;
    }
    return true;
}

bool MediaPlaylistProvider::insertMedia(int index, const Media &media)
{
    Q_UNUSED(index)
    Q_UNUSED(media)
    return false;
}

bool MediaPlaylistProvider::insertMedia(int index, const QList<Media> &mediaList)
{
    for (int i=0; i < mediaList.count(); ++i)
    {
        if (!insertMedia(index + i, mediaList.at(i)))
            return false;
    }

    return true;
}

bool MediaPlaylistProvider::moveMedia(int from, int to)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    return false;
}

bool MediaPlaylistProvider::removeMedia(int pos)
{
    Q_UNUSED(pos)
    return false;
}

bool MediaPlaylistProvider::removeMedia(int start, int end)
{
    for (int pos = start; pos <= end; ++pos)
    {
        if (!removeMedia(pos))
            return false;
    }

    return false;
}

bool MediaPlaylistProvider::clear()
{
    return removeMedia(0, mediaCount()-1);
}

void MediaPlaylistProvider::shuffle()
{

}
