#include "MediaPlayer.h"
#include "MediaObject_p.h"
#include "MediaEngine.h"
#include "MediaPlaylist.h"
#include "MediaPlayerControl.h"

#include "vlc/VLCEngineProvider.h"

#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QLoggingCategory>

#define MAX_NESTED_PLAYLIST 5

Q_LOGGING_CATEGORY(lcMediaPlayer, "mcplayer.MediaPlayer")
Q_GLOBAL_STATIC(VLCEngineProvider, provider)

static void registerMediaPlayerMetaTypes()
{
    qRegisterMetaType<MediaPlayer::PlaybackState>("MediaPlayer::PlaybackState");
    qRegisterMetaType<MediaPlayer::MediaStatus>("MediaPlayer::MediaStatus");
    qRegisterMetaType<MediaPlayer::Error>("MediaPlayer::Error");
}
Q_CONSTRUCTOR_FUNCTION(registerMediaPlayerMetaTypes)


class MediaPlayerPrivate : public MediaObjectPrivate
{
    Q_DECLARE_PUBLIC(MediaPlayer)
public:
    MediaPlaylist *parentPlaylist(MediaPlaylist *pls);
    bool isInPlaylist(const QUrl &url);
    void setMedia(const Media &media, QIODevice *stream = nullptr);
    void setPlaylist(MediaPlaylist *pls);
    void applyPlaylistMedia();
    void bindPlaylist();
    void unbindPlaylist();
    void loadPlaylist();
    void initControls();

    // private slots
    void updatePlaybackState(MediaPlayer::PlaybackState state);
    void updateMediaStatus(MediaPlayer::MediaStatus ms);
    void updateCurrentMedia(const Media &media);
    void updateMedia(const Media &media);
    void updatePlaylist();
    void destroyPlaylist();
    void updateError(int error, const QString &errorString);

public:
    Media qrcMedia;
    Media rootMedia;
    Media pendingPlaylist;

    MediaPlayer::MediaStatus mediaStatus = MediaPlayer::UnknownStatus;
    MediaPlayer::PlaybackState playbackState = MediaPlayer::StoppedState;
    MediaPlayer::Error error = MediaPlayer::NoError;
    QString errorString;

    int ignoreNextStatusChange = -1;
    int nestedPlaylists = 0;
    bool hasStreamPlaybackFeature = false;

    MediaPlaylist *playlist = nullptr;
    MediaPlayerControl *control = nullptr;
};


MediaPlaylist *MediaPlayerPrivate::parentPlaylist(MediaPlaylist *pls)
{
    // find a parent playlist for a media in the active chain of playlists
    // just only the subtree of active ones
    for (MediaPlaylist *current = rootMedia.playlist();
         current && current != pls;
         current = current->currentMedia().playlist())
    {
        if (current->currentMedia().playlist() == pls)
            return current;
    }

    return nullptr;
}

bool MediaPlayerPrivate::isInPlaylist(const QUrl &url)
{
    //  Check whether a url is already in the chain of playlists
    for (MediaPlaylist *current = rootMedia.playlist();
         current && current != playlist;
         current = current->currentMedia().playlist())
    {
        if (current->currentMedia().canonicalUrl() == url)
            return true;
    }

    return false;
}

void MediaPlayerPrivate::setMedia(const Media &media, QIODevice *stream)
{
    if(!control)
        return;

    // playback for the qt resource system file
    if(!media.isNull() && !stream && media.canonicalUrl().scheme() == QLatin1String("qrc"))
    {
        QScopedPointer<QFile> file;
        qInfo(lcMediaPlayer) << "set qrc (Qt resource system) media" << media.canonicalUrl();
        qrcMedia = media;
        file.reset(new QFile(QLatin1Char(':') + media.canonicalUrl().path()));
        if(!file->open(QFile::ReadOnly))
        {
            QMetaObject::invokeMethod(this, "updateError", Qt::QueuedConnection,
                                      Q_ARG(int, MediaPlayer::ResourceError),
                                      Q_ARG(QString, MediaPlayer::tr("Attempting to play invalid media resource")));
            QMetaObject::invokeMethod(this, "updateMediaStatus", Qt::QueuedConnection,
                                      Q_ARG(MediaPlayer::MediaStatus, MediaPlayer::InvalidMedia));
            file.reset();
            ignoreNextStatusChange = MediaPlayer::NoMedia;
            control->setMedia(Media(), nullptr);
        }
        else if(hasStreamPlaybackFeature)
        {
            control->setMedia(media, file.data());
        }
        else
        {
            // create temp file
            QTemporaryFile *tempFile = new QTemporaryFile;
            //  some file it doesn't have an extension
            const QString suffix = QFileInfo(*file).suffix();
            if(!suffix.isEmpty())
                tempFile->setFileTemplate(tempFile->fileTemplate() + QLatin1Char('.') + suffix);

            // copy qrc file data into temporary file
            tempFile->open();
            char buffer[4096];
            while (true)
            {
                qint64 len = file->read(buffer, sizeof (buffer));
                if(len < 1) break;
                tempFile->write(buffer, len);
            }
            tempFile->close();
            file.reset(tempFile);
            control->setMedia(Media(QUrl::fromLocalFile(file->fileName())), nullptr);
        }

    }
    else
    {
        qrcMedia = Media();
        control->setMedia(media, stream);
    }
}

void MediaPlayerPrivate::setPlaylist(MediaPlaylist *pls)
{
    this->unbindPlaylist();
    this->playlist = pls;
    this->applyPlaylistMedia();
}

void MediaPlayerPrivate::applyPlaylistMedia()
{
    Q_Q(MediaPlayer);
    if(this->playlist)
    {
        this->bindPlaylist();

        // nested playlist
        if(playlist->currentMedia().playlist())
        {
            if(this->nestedPlaylists < MAX_NESTED_PLAYLIST)
            {
                emit q->currentMediaChanged(playlist->currentMedia());
                playlist->currentMedia().playlist()->setCurrentIndex(0);
                ++nestedPlaylists;
                this->setPlaylist(playlist->currentMedia().playlist());
            }
            else
            {
                this->playlist->next();
            }
        }
        else
        {
            bool same = (q->currentMedia() == playlist->currentMedia());
            setMedia(this->playlist->currentMedia(), nullptr);
            if(same)
            {
                emit q->currentMediaChanged(q->currentMedia());
            }
        }
    }
    else
    {
        setMedia(Media(), nullptr);
    }
}

void MediaPlayerPrivate::bindPlaylist()
{
    Q_Q(MediaPlayer);
    if(playlist)
    {
        q->bind(playlist);
        connect(playlist, &MediaPlaylist::currentMediaChanged,
                            this, &MediaPlayerPrivate::updateMedia);
        connect(playlist, &MediaPlaylist::destroyed,
                            this, &MediaPlayerPrivate::destroyPlaylist);
    }
}

void MediaPlayerPrivate::unbindPlaylist()
{
    Q_Q(MediaPlayer);
    if(playlist)
    {
        QObject::disconnect(playlist, &MediaPlaylist::currentMediaChanged,
                            this, &MediaPlayerPrivate::updateMedia);
        QObject::disconnect(playlist, &MediaPlaylist::destroyed,
                            this, &MediaPlayerPrivate::destroyPlaylist);
        q->unbind(playlist);
    }
}

void MediaPlayerPrivate::loadPlaylist()
{
    Q_Q(MediaPlayer);
    Q_ASSERT(pendingPlaylist.isNull());

    if(nestedPlaylists < MAX_NESTED_PLAYLIST
            && !q->currentMedia().canonicalUrl().isEmpty()
            && !isInPlaylist(q->currentMedia().canonicalUrl()))
    {
        // loading remote url media
        pendingPlaylist = Media(new MediaPlaylist, q->currentMedia().canonicalUrl(), true);
        QObject::connect(pendingPlaylist.playlist(), SIGNAL(loaded()), q, SLOT(_q_handlePlaylistLoaded()));
        QObject::connect(pendingPlaylist.playlist(), SIGNAL(loadFailed()), q, SLOT(_q_handlePlaylistLoadFailed()));
        pendingPlaylist.playlist()->load(pendingPlaylist.canonicalRequest());
    }
    else if(playlist)
    {
        playlist->next();
    }
}

void MediaPlayerPrivate::initControls()
{
    Q_Q(MediaPlayer);
    if(engine)
    {
        control = qobject_cast<MediaPlayerControl*>(engine->requestControl(MediaPlayerControl_iid));
        if (control)
        {
            qInfo(lcMediaPlayer) << "request media player control completed.";
            // connect control signals to player slots
            q->connect(control, &MediaPlayerControl::stateChanged, this, &MediaPlayerPrivate::updatePlaybackState);
            q->connect(control, &MediaPlayerControl::mediaStatusChanged, this, &MediaPlayerPrivate::updateMediaStatus);
            q->connect(control, &MediaPlayerControl::mediaChanged, this, &MediaPlayerPrivate::updateCurrentMedia);
            q->connect(control, &MediaPlayerControl::error, this, &MediaPlayerPrivate::updateError);

            // connect control signals to player signals
            q->connect(control, &MediaPlayerControl::timeChanged, q, &MediaPlayer::timeChanged);
            q->connect(control, &MediaPlayerControl::durationChanged, q, &MediaPlayer::durationChanged);
            q->connect(control, &MediaPlayerControl::positionChanged, q, &MediaPlayer::positionChanged);
            q->connect(control, &MediaPlayerControl::volumeChanged, q, &MediaPlayer::volumeChanged);
            q->connect(control, &MediaPlayerControl::mutedChanged, q, &MediaPlayer::mutedChanged);
            q->connect(control, &MediaPlayerControl::seekableChanged, q, &MediaPlayer::seekableChanged);
            q->connect(control, &MediaPlayerControl::rateChanged, q, &MediaPlayer::rateChanged);
            q->connect(control, &MediaPlayerControl::bufferStatusChanged, q, &MediaPlayer::bufferStatusChanged);

            q->connect(control, &MediaPlayerControl::opening, q, &MediaPlayer::opening);
            q->connect(control, &MediaPlayerControl::buffering, q, &MediaPlayer::buffering);
            q->connect(control, &MediaPlayerControl::playing, q, &MediaPlayer::playing);
            q->connect(control, &MediaPlayerControl::paused, q, &MediaPlayer::paused);
            q->connect(control, &MediaPlayerControl::stopped, q, &MediaPlayer::stopped);
            q->connect(control, &MediaPlayerControl::forward, q, &MediaPlayer::forward);
            q->connect(control, &MediaPlayerControl::backward, q, &MediaPlayer::backward);
            q->connect(control, &MediaPlayerControl::end, q, &MediaPlayer::end);
            q->connect(control, &MediaPlayerControl::voutChanged, q, &MediaPlayer::voutChanged);

            playbackState = control->playbackState();
            mediaStatus = control->mediaStatus();

            if (mediaStatus == MediaPlayer::StalledMedia || mediaStatus == MediaPlayer::BufferingMedia)
                q->watchProperty("bufferStatus");

            // see positionChanged do this
//            if(state == MediaPlayer::PlayingState)
//                q->watchProperty("position");

            // TODO: is suppotrt some features(eg:stream device playback) that provided engine?
            // ...

            // TODO: connetion audio output signals and set audio format
            // ...
        }
        else
        {
            qCritical(lcMediaPlayer) << "request media player control failed.";
        }
    }
}

void MediaPlayerPrivate::updatePlaybackState(MediaPlayer::PlaybackState state)
{
    Q_Q(MediaPlayer);
    qDebug(lcMediaPlayer) << "state changed"
                          << playbackState
                          << "to"<< state;
//    sender()->dumpObjectInfo();

    if (playlist && playlist->currentIndex() != -1
            && state != playbackState
            && state == MediaPlayer::StoppedState)
    {
        if(control->mediaStatus() == MediaPlayer::EndOfMedia
                || control->mediaStatus() == MediaPlayer::InvalidMedia)
        {
            Q_ASSERT(playbackState != MediaPlayer::StoppedState);
            qDebug() << "============================" << playbackState << state;
            playlist->next();
            return;
        }
        else if(control->mediaStatus() == MediaPlayer::LoadingMedia)
        {
            return;
        }
    }

    if(state != playbackState)
    {
        playbackState = state;
        state == MediaPlayer::PlayingState ? q->watchProperty("position") : q->unwatchProperty("position");
        emit q->playbackStateChanged(state);
    }
}

void MediaPlayerPrivate::updateMediaStatus(MediaPlayer::MediaStatus ms)
{
    Q_Q(MediaPlayer);
    if (int(ms) == this->ignoreNextStatusChange)
    {
        this->ignoreNextStatusChange = -1;
        return;
    }

    if (ms != this->mediaStatus)
    {
        this->mediaStatus = ms;

        switch (ms)
        {
        case MediaPlayer::StalledMedia:
        case MediaPlayer::BufferingMedia:
            q->watchProperty("bufferStatus");
            break;
        default:
            q->unwatchProperty("bufferStatus");
            break;
        }

        emit q->mediaStatusChanged(ms);
    }
}

void MediaPlayerPrivate::updateCurrentMedia(const Media &media)
{
    Q_Q(MediaPlayer);
    qInfo(lcMediaPlayer) << "set current media:" << media.canonicalUrl();

    emit q->currentMediaChanged(qrcMedia.isNull() ? media : qrcMedia);
}

void MediaPlayerPrivate::updateMedia(const Media &media)
{
    Q_Q(MediaPlayer);
    if(!control)
        return;

    qInfo(lcMediaPlayer) << "update media:" << media.canonicalUrl();

    Q_ASSERT(playlist);
    if(media.isNull() && playlist != rootMedia.playlist())
    {
        MediaPlaylist *pls = parentPlaylist(playlist);
        Q_ASSERT(pls);

        unbindPlaylist();
        playlist = pls;
        bindPlaylist();

        Q_ASSERT(!pendingPlaylist.playlist());

        --nestedPlaylists;
        Q_ASSERT(nestedPlaylists >= 0);

        playlist->next();

        return;
    }

    if(media.playlist())
    {
        if(nestedPlaylists < MAX_NESTED_PLAYLIST)
        {
            ++nestedPlaylists;
            Q_ASSERT(!pendingPlaylist.playlist());

            // disconnect current playlist
            unbindPlaylist();

            // new playlist signals are connected
            playlist = media.playlist();
            emit q->currentMediaChanged(media);

            // handling pending playlist
           this->updatePlaylist();
        }
        else if(playlist)
        {
            playlist->next();
        }

        return;
    }

    const MediaPlayer::PlaybackState currentState = playbackState;
    setMedia(media, nullptr);

    if(!media.isNull())
    {
        switch (currentState)
        {
        case MediaPlayer::PlayingState:
            control->play();
            break;
        case MediaPlayer::PausedState:
            control->pause();
            break;
        default:
            break;
        }
    }
}

void MediaPlayerPrivate::updatePlaylist()
{
    Q_Q(MediaPlayer);
    qInfo(lcMediaPlayer) << "playlist loaded:";

    if (pendingPlaylist.playlist())
    {
        Q_ASSERT(!q->currentMedia().playlist());
        // if there is an active playlist
        if (playlist)
        {
            Q_ASSERT(playlist->currentIndex() >= 0);

            unbindPlaylist();
            playlist->insertMedia(playlist->currentIndex() + 1, pendingPlaylist);
            playlist->removeMedia(playlist->currentIndex());
            nestedPlaylists++;
        }
        else
        {
            Q_ASSERT(!rootMedia.playlist());
            rootMedia = pendingPlaylist;
            emit q->mediaChanged(rootMedia);
        }

        playlist = pendingPlaylist.playlist();
        emit q->currentMediaChanged(pendingPlaylist);
    }

    pendingPlaylist = Media();

    playlist->next();
    applyPlaylistMedia();

    switch (playbackState)
    {
    case MediaPlayer::PausedState:
        control->pause();
        break;
    case MediaPlayer::PlayingState:
        control->play();
        break;
    case MediaPlayer::StoppedState:
        break;
    }
}

void MediaPlayerPrivate::destroyPlaylist()
{
    qInfo(lcMediaPlayer) << "destroy playlist";

    playlist = nullptr;
    setMedia(Media(), nullptr);
}

void MediaPlayerPrivate::updateError(int error, const QString &errorString)
{
    Q_Q(MediaPlayer);
    qWarning(lcMediaPlayer) << "error:" << errorString;

    if(error == int(MediaPlayer::MediaIsPlaylist))
    {
        this->loadPlaylist();
    }
    else
    {
        this->error = MediaPlayer::Error(error);
        this->errorString = errorString;
        emit q->error(this->error);

        if(playlist)
            playlist->next();
    }
}

/**
 * @brief MediaPlayer::MediaPlayer
 * @param parent
 */
MediaPlayer::MediaPlayer(QObject *parent)
    : MediaObject(*new MediaPlayerPrivate, provider->requestEngine(MEDIAENGINE_MEDIAPLAYER), parent)
{
    Q_D(MediaPlayer);
    if(!d->engine)
    {
        d->error = MediaPlayer::EngineMissingError;
        qCritical(lcMediaPlayer) << "media player engine missing.";
    }
    else
    {
        d->initControls();
    }
}

MediaPlayer::~MediaPlayer()
{
    Q_D(MediaPlayer);
    d->unbindPlaylist();
    disconnect();

    if (d->engine)
    {
        if (d->control)
            d->engine->releaseControl(d->control);
        if (d->metadataControl)
            d->engine->releaseControl(d->metadataControl);

        provider->releaseEngine(d->engine);
    }
}

int MediaPlayer::hasSupport(const QString &mimeType, const QStringList &codecs)
{
    return provider->hasSupport(QByteArray(MEDIAENGINE_MEDIAPLAYER), mimeType, codecs);
}

QStringList MediaPlayer::supportedMimeTypes(Flags flags)
{
    return provider->supportedMimeTypes(QByteArray(MEDIAENGINE_MEDIAPLAYER), flags);
}

qint64 MediaPlayer::duration() const
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return 0;

    return d->control->duration();
}

bool MediaPlayer::seekable() const
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return false;

    return d->control->isSeekable();
}

const QIODevice *MediaPlayer::mediaStream() const
{
    Q_D(const MediaPlayer);

    // When playing a resource file, we might have passed a QFile to the backend. Hide it from
    // the user.
    if (d->control && d->qrcMedia.isNull())
        return d->control->mediaStream();

    return nullptr;
}

int MediaPlayer::bufferStatus() const
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return 0;

    return d->control->bufferStatus();
}

MediaPlayer::PlaybackState MediaPlayer::playbackState() const
{
    Q_D(const MediaPlayer);
    if(d->control && d->mediaStatus == MediaPlayer::EndOfMedia
            && d->playbackState != d->control->playbackState())
        return d->control->playbackState();

    return d->playbackState;
}

MediaPlayer::MediaStatus MediaPlayer::mediaStatus() const
{
    Q_D(const MediaPlayer);
    return d->mediaStatus;
}

Media MediaPlayer::currentMedia() const
{
    Q_D(const MediaPlayer);
    if (!d->qrcMedia.isNull())
        return d->qrcMedia;

    if (d->control)
        return d->control->media();

    return Media();
}

Media MediaPlayer::media() const
{
    Q_D(const MediaPlayer);
    return d->rootMedia;
}

void MediaPlayer::setMedia(const Media &media, QIODevice *stream)
{
    Q_D(MediaPlayer);

   Media oldMedia = d->rootMedia;
   d->unbindPlaylist();
   d->playlist = nullptr;
   d->nestedPlaylists = 0;
   d->rootMedia = media;

   if (oldMedia != media)
       emit mediaChanged(d->rootMedia);

   MediaPlaylist *pls = media.playlist();
   if (pls)
   {
       qInfo(lcMediaPlayer) << "set playlist" << media.canonicalUrl();
       // reset playlist to the 1st item
       // Note: must be set to the -1 item when delay to create engine
       pls->setCurrentIndex(0);
       d->setPlaylist(pls);
   }
   else
   {
       qInfo(lcMediaPlayer) << "set media" << media.canonicalUrl();
       d->setMedia(media, stream);
   }
}

MediaPlaylist *MediaPlayer::playlist() const
{
    Q_D(const MediaPlayer);
    return d->rootMedia.playlist();
}

void MediaPlayer::setPlaylist(MediaPlaylist *playlist)
{
    Media m(playlist, QUrl(), false);
    setMedia(m);
}

qint64 MediaPlayer::time() const
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return 0.0;

    return d->control->time();
}

void MediaPlayer::setTime(qint64 ms)
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return;

    return d->control->setTime(ms);
}

float MediaPlayer::rate() const
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return 0.0;

    return d->control->rate();
}

void MediaPlayer::setRate(float rate)
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return;

    return d->control->setRate(rate);
}

double MediaPlayer::position() const
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return 0;

    return d->control->position();
}

void MediaPlayer::setPosition(double pos)
{
    Q_D(MediaPlayer);
    if(!d->control)
        return;

    d->control->setPosition(pos);
}

int MediaPlayer::volume() const
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return 0;

    return d->control->volume();
}

void MediaPlayer::setVolume(int volume)
{
    Q_D(MediaPlayer);
    if(!d->control)
        return;

    int clamped = qBound(0, volume, 100);
    if (clamped == this->volume())
        return;

    d->control->setVolume(clamped);
}

bool MediaPlayer::isMuted() const
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return false;

    return d->control->isMuted();
}

void MediaPlayer::setMuted(bool muted)
{
    Q_D(MediaPlayer);
    if(!d->control || muted == this->isMuted())
        return;

    d->control->setMuted(muted);
}

bool MediaPlayer::isAudioAvailable() const
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return false;

    return d->control->isAudioAvailable();
}

bool MediaPlayer::isVideoAvailable() const
{
    Q_D(const MediaPlayer);
    if(!d->control)
        return false;

    return d->control->isVideoAvailable();
}

MediaPlayer::Error MediaPlayer::error() const
{
    Q_D(const MediaPlayer);
    return d->error;
}

QString MediaPlayer::errorString() const
{
    Q_D(const MediaPlayer);
    return d->errorString;
}

void MediaPlayer::play()
{
    Q_D(MediaPlayer);
    if(!d->control)
    {
        qDebug(lcMediaPlayer) << "The MediaPlayer object does not have a valid engine";
        QMetaObject::invokeMethod(d, "updateError", Qt::QueuedConnection,
                                  Q_ARG(int, MediaPlayer::EngineMissingError),
                                  Q_ARG(QString, tr("The MediaPlayer object does not have a valid engine")));
        return;
    }

    if(d->rootMedia.playlist() && !d->rootMedia.playlist()->isEmpty())
    {
        if(d->playbackState != MediaPlayer::PlayingState)
            d->updatePlaybackState(MediaPlayer::PlayingState);

        if(d->rootMedia.playlist()->currentIndex() == -1)
        {
            if(d->playlist != d->rootMedia.playlist())
                d->setPlaylist(d->rootMedia.playlist());

            Q_ASSERT(d->playlist == d->rootMedia.playlist());

            emit currentMediaChanged(d->rootMedia);
            d->playlist->setCurrentIndex(0);
        }
    }

    d->error = MediaPlayer::NoError;
    d->errorString = QString();

    d->control->play();
}

void MediaPlayer::pause()
{
    Q_D(MediaPlayer);
    if(!d->control)
        return;

    d->control->pause();
}

void MediaPlayer::togglePause()
{
    Q_D(MediaPlayer);
    if(!d->control)
        return;

    d->control->togglePause();
}

void MediaPlayer::resume()
{
    Q_D(MediaPlayer);
    if(!d->control)
        return;

    d->control->resume();
}

void MediaPlayer::stop()
{
    Q_D(MediaPlayer);
    if(d->control)
        d->control->stop();

    if(d->playlist && d->playbackState != MediaPlayer::StoppedState)
    {
        d->playbackState = MediaPlayer::StoppedState;
        unwatchProperty("position");
        emit playbackStateChanged(MediaPlayer::StoppedState);
    }
}

void MediaPlayer::previous()
{
    Q_D(MediaPlayer);
    if(!d->playlist)
        return;

    d->playlist->previous();
}

void MediaPlayer::next()
{
    Q_D(MediaPlayer);
    if(!d->playlist)
        return;

    d->playlist->next();
}
