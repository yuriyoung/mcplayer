#include "Media.h"
#include "MediaPlaylist.h"

#include <QPointer>

static void registerMediaMetaTypes()
{
    qRegisterMetaType<Media>();
}
Q_CONSTRUCTOR_FUNCTION(registerMediaMetaTypes)


class MediaPrivate : public QSharedData
{
public:
    MediaPrivate() : isPlaylistOwned(false) {}

    MediaPrivate(const MediaPrivate &other)
        : playlist(other.playlist),
          isPlaylistOwned(false)
    {
    }

    MediaPrivate(MediaPlaylist *pls, const QUrl &url, bool isOwn)
        : playlist(pls),
          isPlaylistOwned(isOwn)
    {
        resources << MediaResource(url);
    }

    ~MediaPrivate()
    {
        if (isPlaylistOwned && !playlist.isNull())
            playlist.data()->deleteLater();
    }

    bool operator ==(const MediaPrivate &other) const
    {
        return playlist == other.playlist;
    }

    MediaPrivate& operator=(const MediaPrivate &other)
    {
        playlist = other.playlist;
        isPlaylistOwned = other.isPlaylistOwned;
        return *this;
    }

    MediaResourceList resources;
    QPointer<MediaPlaylist> playlist;
    bool isPlaylistOwned;
};

/**
 * @brief Media::Media
 */
Media::Media()
{
    // just a null media
}

Media::Media(const QUrl &url)
    : d(new MediaPrivate)
{
    d->resources << MediaResource(url);
}

Media::Media(const MediaResource &resource)
    : d(new MediaPrivate)
{
    d->resources << resource;
}

Media::Media(const QNetworkRequest &request)
    : d(new MediaPrivate)
{
     d->resources << MediaResource(request);
}

Media::Media(const MediaResourceList &resources)
    : d(new MediaPrivate)
{
    d->resources = resources;
}

Media::Media(MediaPlaylist *playlist, const QUrl &contentUrl, bool takeOwnership)
    : d(new MediaPrivate(playlist, contentUrl, takeOwnership))
{

}

Media::Media(const Media &other)
    : d(other.d)
{

}

Media::~Media()
{

}

Media &Media::operator=(const Media &other)
{
    d = other.d;
    return *this;
}

bool Media::operator==(const Media &other) const
{
    return (d.constData() == nullptr && other.d.constData() == nullptr) ||
            (d.constData() != nullptr && other.d.constData() != nullptr &&
            *d.constData() == *other.d.constData());
}

bool Media::operator!=(const Media &other) const
{
    return !(*this == other);
}

bool Media::isNull() const
{
    return d.constData() == nullptr;
}

QUrl Media::canonicalUrl() const
{
    return canonicalResource().url();
}

QNetworkRequest Media::canonicalRequest() const
{
    return canonicalResource().request();
}

MediaResource Media::canonicalResource() const
{
    return isNull() ?  MediaResource() : d->resources.value(0);
}

MediaResourceList Media::resources() const
{
    return isNull() ? MediaResourceList() : d->resources;
}

MediaPlaylist *Media::playlist() const
{
    return isNull() ? nullptr : d->playlist.data();
}
