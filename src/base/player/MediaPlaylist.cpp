#include "MediaPlaylist.h"
#include "MediaPlayer.h"
#include "MediaEngine.h"
#include "LocalMediaPlaylistControl.h"
#include "MediaPlaylistProvider.h"

#include <QFile>
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(lcMediaPlaylist, "mcplayer.MediaPlaylist")

class MediaPlaylistPrivate
{
    Q_DECLARE_PUBLIC(MediaPlaylist)
public:

    void connectPlaylist();
    void disconnectPlaylist();
    void syncControls(MediaPlaylistControl *oldControl, MediaPlaylistControl *newControl,
                      int *removedStart, int *removedEnd,
                      int *insertedStart, int *insertedEnd);

    MediaObject *mediaObject = nullptr; // means media player engine object

    MediaPlaylistControl *defaultControl = nullptr;
    MediaPlaylistControl *control = nullptr;

    MediaPlaylist::Error error = MediaPlaylist::NoError;
    QString errorString;

    MediaPlaylist *q_ptr = nullptr;
};

void MediaPlaylistPrivate::connectPlaylist()
{
    Q_Q(MediaPlaylist);
    if(this->control)
    {
        MediaPlaylistProvider *playlist = this->control->playlistProvider();
        QObject::connect(playlist, &MediaPlaylistProvider::mediaChanged, q, &MediaPlaylist::mediaChanged);
        QObject::connect(playlist, &MediaPlaylistProvider::mediaAboutToInserted, q, &MediaPlaylist::mediaAboutToInserted);
        QObject::connect(playlist, &MediaPlaylistProvider::mediaInserted, q, &MediaPlaylist::mediaInserted);
        QObject::connect(playlist, &MediaPlaylistProvider::mediaAboutToRemoved, q, &MediaPlaylist::mediaAboutToRemoved);
        QObject::connect(playlist, &MediaPlaylistProvider::mediaRemoved, q, &MediaPlaylist::mediaRemoved);

        QObject::connect(control, &MediaPlaylistControl::currentIndexChanged, q, &MediaPlaylist::currentIndexChanged);
        QObject::connect(control, &MediaPlaylistControl::currentMediaChanged, q, &MediaPlaylist::currentMediaChanged);
        QObject::connect(control, &MediaPlaylistControl::playbackModeChanged, q, &MediaPlaylist::playbackModeChanged);
    }
}

void MediaPlaylistPrivate::disconnectPlaylist()
{
    Q_Q(MediaPlaylist);
    if(this->control)
    {
        MediaPlaylistProvider *playlist = this->control->playlistProvider();
        QObject::disconnect(playlist, &MediaPlaylistProvider::mediaChanged, q, &MediaPlaylist::mediaChanged);
        QObject::disconnect(playlist, &MediaPlaylistProvider::mediaAboutToInserted, q, &MediaPlaylist::mediaAboutToInserted);
        QObject::disconnect(playlist, &MediaPlaylistProvider::mediaInserted, q, &MediaPlaylist::mediaInserted);
        QObject::disconnect(playlist, &MediaPlaylistProvider::mediaAboutToRemoved, q, &MediaPlaylist::mediaAboutToRemoved);
        QObject::disconnect(playlist, &MediaPlaylistProvider::mediaRemoved, q, &MediaPlaylist::mediaRemoved);

        QObject::disconnect(control, &MediaPlaylistControl::currentIndexChanged, q, &MediaPlaylist::currentIndexChanged);
        QObject::disconnect(control, &MediaPlaylistControl::currentMediaChanged, q, &MediaPlaylist::currentMediaChanged);
        QObject::disconnect(control, &MediaPlaylistControl::playbackModeChanged, q, &MediaPlaylist::playbackModeChanged);
    }}

void MediaPlaylistPrivate::syncControls(MediaPlaylistControl *oldControl, MediaPlaylistControl *newControl, int *removedStart, int *removedEnd, int *insertedStart, int *insertedEnd)
{
    Q_ASSERT(oldControl != nullptr && newControl != nullptr);
    Q_ASSERT(removedStart != nullptr && removedEnd != nullptr && insertedStart != nullptr && insertedEnd != nullptr);

    MediaPlaylistProvider *oldPlaylist = oldControl->playlistProvider();
    MediaPlaylistProvider *newPlaylist = newControl->playlistProvider();

    Q_ASSERT(oldPlaylist != NULL && newPlaylist != NULL);

    *removedStart = -1;
    *removedEnd = -1;
    *insertedStart = -1;
    *insertedEnd = -1;

    if (newPlaylist->isReadOnly())
    {

        if (oldPlaylist->mediaCount() > 0)
        {
            *removedStart = 0;
            *removedEnd = oldPlaylist->mediaCount() - 1;
        }
        if (newPlaylist->mediaCount() > 0)
        {
            *insertedStart = 0;
            *insertedEnd = newPlaylist->mediaCount() - 1;
        }
    }
    else
    {
        const int oldPlaylistSize = oldPlaylist->mediaCount();

        newPlaylist->clear();
        for (int i = 0; i < oldPlaylistSize; ++i)
            newPlaylist->addMedia(oldPlaylist->media(i));
    }

    newControl->setPlaybackMode(oldControl->playbackMode());
    newControl->setCurrentIndex(oldControl->currentIndex());
}

/**
 * @brief MediaPlaylist::MediaPlaylist
 * @param parent
 */
MediaPlaylist::MediaPlaylist(QObject *parent) : QObject(parent)
{
    d_ptr->q_ptr = this;
    d_ptr->defaultControl = new LocalMediaPlaylistControl(this);

    setMediaObject(nullptr);
}

MediaPlaylist::~MediaPlaylist()
{

}

MediaObject *MediaPlaylist::mediaObject() const
{
    Q_D(const MediaPlaylist);
    return d->mediaObject;
}

/**
 * @brief binding player engine object and connection playlist provider‘s/control's signals-slots
 * @param object
 * @return
 */
bool MediaPlaylist::setMediaObject(MediaObject *object)
{
    Q_D(MediaPlaylist);
    if(object && object == d->mediaObject)
    {
        qDebug(lcMediaPlaylist) << "is same media engine object";
        return true;
    }

    MediaEngine *engine = object ? object->engine() : nullptr;
    MediaPlaylistControl *newControl = nullptr;

    if(engine)
        newControl = qobject_cast<MediaPlaylistControl *>(engine->requestControl(MediaPlaylistControl_iid));

    if(!newControl)
        newControl = d->defaultControl;

    if(d->control != newControl)
    {
        int removedStart = -1;
        int removedEnd = -1;
        int insertedStart = -1;
        int insertedEnd = -1;

        // release old control
        if(d->control)
        {
            d->disconnectPlaylist();

            // sync current playlist to new playlist
            d->syncControls(d->control, newControl,
                            &removedStart, &removedEnd,
                            &insertedStart, &insertedEnd);
            if(d->mediaObject)
                d->mediaObject->engine()->releaseControl(d->control);
        }

        // replace control
        d->control = newControl; // if dose not a new control default apply LocalMediaPlaylistControl
        d->connectPlaylist();

        if (removedStart != -1 && removedEnd != -1)
        {
            emit mediaAboutToRemoved(removedStart, removedEnd);
            emit mediaRemoved(removedStart, removedEnd);
        }

        if (insertedStart != -1 && insertedEnd != -1)
        {
            emit mediaAboutToInserted(insertedStart, insertedEnd);
            emit mediaInserted(insertedStart, insertedEnd);
        }
    }

    d->mediaObject = object;
    return true;
}

int MediaPlaylist::currentIndex() const
{
    Q_D(const MediaPlaylist);
    if(!d->control)
        return -1;

    return d->control->currentIndex();
}

Media MediaPlaylist::currentMedia() const
{
    Q_D(const MediaPlaylist);
    if(!d->control)
        return Media();

    return d->control->playlistProvider()->media(currentIndex());
}

Media MediaPlaylist::media(int index) const
{
    Q_D(const MediaPlaylist);
    if(!d->control)
        return Media();

    return d->control->playlistProvider()->media(index);
}

int MediaPlaylist::nextIndex(int steps) const
{
    Q_D(const MediaPlaylist);
    if(!d->control)
        return -1;

    return d->control->nextIndex(steps);
}

int MediaPlaylist::previousIndex(int steps) const
{
    Q_D(const MediaPlaylist);
    if(!d->control)
        return -1;

    return d->control->previousIndex(steps);
}

MediaPlaylist::PlaybackMode MediaPlaylist::playbackMode() const
{
    Q_D(const MediaPlaylist);
    if(!d->control)
        return MediaPlaylist::Sequential;

    return d->control->playbackMode();
}

void MediaPlaylist::setPlaybackMode(MediaPlaylist::PlaybackMode mode)
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return;

    return d->control->setPlaybackMode(mode);
}

bool MediaPlaylist::isEmpty() const
{
    return mediaCount() == 0;
}

int MediaPlaylist::mediaCount() const
{
    Q_D(const MediaPlaylist);
    if(!d->control)
        return 0;

    return d->control->playlistProvider()->mediaCount();
}

bool MediaPlaylist::isReadOnly() const
{
    Q_D(const MediaPlaylist);
    if(!d->control)
        return false;

    return d->control->playlistProvider()->isReadOnly();
}

bool MediaPlaylist::addMedia(const Media &media)
{
    Q_D(MediaPlaylist);
    if(!d->control)
    {
        return false;
    }

    return d->control->playlistProvider()->addMedia(media);
}

bool MediaPlaylist::addMedia(const QList<Media> &list)
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return false;

    return d->control->playlistProvider()->addMedia(list);
}

bool MediaPlaylist::insertMedia(int index, const Media &media)
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return false;

    MediaPlaylistProvider *provider = d->control->playlistProvider();
    int pos = qBound(0, index, provider->mediaCount());
    return provider->insertMedia(pos, media);
}

bool MediaPlaylist::insertMedia(int index, const QList<Media> &list)
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return false;

    MediaPlaylistProvider *provider = d->control->playlistProvider();
    int pos = qBound(0, index, provider->mediaCount());
    return provider->insertMedia(pos, list);
}

bool MediaPlaylist::removeMedia(int start, int end)
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return false;

    MediaPlaylistProvider *provider = d->control->playlistProvider();
    start = qMax(0, start);
    end = qMin(end, provider->mediaCount() - 1);
    if(start > end)
        return false;

    return provider->removeMedia(start, end);
}

bool MediaPlaylist::removeMedia(int start)
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return false;

    MediaPlaylistProvider *provider = d->control->playlistProvider();
    if(start < 0 || start > provider->mediaCount())
        return false;

    return provider->removeMedia(start);
}

bool MediaPlaylist::moveMedia(int from, int to)
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return false;

    MediaPlaylistProvider *provider = d->control->playlistProvider();
    return provider->moveMedia(qBound(0, from, provider->mediaCount()),
                               qBound(0, to, provider->mediaCount()));
}

bool MediaPlaylist::clear()
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return false;

    return d->control->playlistProvider()->clear();
}

MediaPlaylist::Error MediaPlaylist::error() const
{
    Q_D(const MediaPlaylist);
    return d->error;
}

QString MediaPlaylist::errorString() const
{
    Q_D(const MediaPlaylist);
    return d->errorString;
}

void MediaPlaylist::load(const QNetworkRequest &request, const char *format)
{
    Q_D(MediaPlaylist);

    d->error = NoError;
    d->errorString.clear();

    if (d->control->playlistProvider()->load(request, format))
        return;

    if (isReadOnly())
    {
        d->error = AccessDeniedError;
        d->errorString = tr("Could not add items to read only playlist.");
        emit loadFailed();
        return;
    }

    // TODO: create playlist plugin to request playlist media from network
    // ...


    d->error = FormatNotSupportedError;
    d->errorString = tr("Playlist format is not supported");
    emit loadFailed();
}

void MediaPlaylist::load(const QUrl &location, const char *format)
{
    load(QNetworkRequest(location), format);
}

void MediaPlaylist::load(QIODevice *device, const char *format)
{
    Q_D(MediaPlaylist);
    d->error = NoError;
    d->errorString.clear();

    if(d->control->playlistProvider()->load(device, format))
        return;

    if(isReadOnly())
    {
        d->error = AccessDeniedError;
        d->errorString = tr("Could not add items to read only playlist.");
        emit loadFailed();
        return;
    }

    // TODO: create playlist plugin to parse playlist media
    // ...

    d->error = FormatNotSupportedError;
    d->errorString = tr("Playlist format is not supported");
    emit loadFailed();
}

bool MediaPlaylist::save(const QUrl &location, const char *format)
{
    Q_D(MediaPlaylist);
    d->error = NoError;
    d->errorString.clear();

    if (d->control->playlistProvider()->save(location,format))
        return true;

    QFile file(location.toLocalFile());

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        d->error = AccessDeniedError;
        d->errorString = tr("The file could not be accessed.");
        return false;
    }

    return save(&file, format);
}

bool MediaPlaylist::save(QIODevice *device, const char *format)
{
    Q_D(MediaPlaylist);

    d->error = NoError;
    d->errorString.clear();

    if(d->control->playlistProvider()->save(device, format))
        return true;

    // TODO: create playlist plugin to save playlist
    // ...

    d->error = FormatNotSupportedError;
    d->errorString = tr("Playlist format is not supported.");

    return false;
}

void MediaPlaylist::next()
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return;

    d->control->next();
}

void MediaPlaylist::previous()
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return;

    d->control->previous();
}

void MediaPlaylist::shuffle()
{
    Q_D(MediaPlaylist);

    if(!d->control)
        return;

    d->control->playlistProvider()->shuffle();
}

void MediaPlaylist::setCurrentIndex(int index)
{
    Q_D(MediaPlaylist);
    if(!d->control)
        return;

    d->control->setCurrentIndex(index);
}
