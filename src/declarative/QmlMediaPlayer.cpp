#include "QmlMediaPlayer.h"
#include "QmlMediaMetadata.h"
#include "QmlMediaPlaylist.h"

#include <QQmlInfo>
#include <QTime>
#include <QDebug>

QmlMediaPlayer::QmlMediaPlayer(QObject *parent)
    : QObject(parent)
{

}

QmlMediaPlayer::~QmlMediaPlayer()
{
    m_metadata.reset();
    delete m_player;
}

void QmlMediaPlayer::classBegin()
{
    qDebug() << "QmlMediaPlayer::classBegin()";
    m_player = new MediaPlayer(this);

    // media meta data
    m_metadata.reset(new QmlMediaMetadata(m_player));
    connect(m_player, SIGNAL(metadataChanged()), m_metadata.data(), SIGNAL(metadataChanged()));

    // signal to slot
    connect(m_player, &MediaPlayer::playbackStateChanged, this, &QmlMediaPlayer::updatePlaybackState);
    connect(m_player, &MediaPlayer::mediaStatusChanged, this, &QmlMediaPlayer::updateMediaStatus);
    connect(m_player, &MediaPlayer::mediaChanged, this, &QmlMediaPlayer::updateMedia);
    connect(m_player, &MediaPlayer::timeChanged, this, &QmlMediaPlayer::updateTime);
    connect(m_player, SIGNAL(error(MediaPlayer::Error)), this, SLOT(updateError(MediaPlayer::Error)));

    // singal to signal
    connect(m_player, &MediaPlayer::durationChanged, this, &QmlMediaPlayer::durationChanged);
    connect(m_player, &MediaPlayer::positionChanged, this, &QmlMediaPlayer::positionChanged);
    connect(m_player, &MediaPlayer::volumeChanged, this, &QmlMediaPlayer::volumeChanged);
    connect(m_player, &MediaPlayer::mutedChanged, this, &QmlMediaPlayer::mutedChanged);
    connect(m_player, &MediaPlayer::rateChanged, this, &QmlMediaPlayer::rateChanged);
    connect(m_player, &MediaPlayer::bufferStatusChanged, this, &QmlMediaPlayer::bufferStatusChanged);
    connect(m_player, &MediaPlayer::seekableChanged, this, &QmlMediaPlayer::seekableChanged);

    emit mediaObjectChanged();
}

void QmlMediaPlayer::componentComplete()
{
    qDebug() << "QmlMediaPlayer::componentComplete()";

    if (!qFuzzyCompare(m_volume, qreal(1.0)))
            m_player->setVolume(qRound(m_volume * 100));

    if (m_muted)
        m_player->setMuted(m_muted);

    if (!qFuzzyCompare(m_rate, qreal(1.0)))
        m_player->setRate(static_cast<float>(m_rate));

//    if (m_notifyInterval != m_player->notifyInterval())
//        m_player->setNotifyInterval(m_notifyInterval);

    if (!m_media.isNull() && (m_autoLoad || m_autoPlay))
    {
        m_player->setMedia(m_media);
        m_loaded = true;
        if (m_position > 0)
            m_player->setPosition(m_position);
    }

    m_componentCompleted = true;

    if (!m_media.isNull() && m_autoLoad)
    {
        emit durationChanged();
    }

    if (m_autoPlay)
    {
        m_media.isNull() ? m_player->stop() : m_player->play();
    }
}

QmlMediaMetadata *QmlMediaPlayer::metadata() const
{
    return m_metadata.data();
}

TimeTick QmlMediaPlayer::duration() const
{
    return !m_componentCompleted ? 0 : m_player->duration();
}

qreal QmlMediaPlayer::position() const
{
    return !m_componentCompleted ? 0 : m_player->position();
}

bool QmlMediaPlayer::seekable() const
{
    return !m_componentCompleted ? false : m_player->seekable();
}

qreal QmlMediaPlayer::bufferStatus() const
{
    return !m_componentCompleted ? 0 : qreal(m_player->bufferStatus()) / 100;
}

QmlMediaPlayer::MediaStatus QmlMediaPlayer::mediaStatus() const
{
    return MediaStatus(m_player->mediaStatus());
}

QUrl QmlMediaPlayer::source() const
{
    return m_source;
}

void QmlMediaPlayer::setSource(const QUrl &url)
{
    if (url == m_source && m_playlist == nullptr)
        return;

    if (m_playlist)
    {
        m_playlist = nullptr;
        emit playlistChanged();
    }

    m_source = url;
    m_media = m_source.isEmpty() ? Media() : m_source;
    m_loaded = false;
    if (m_componentCompleted && (m_autoLoad || m_media.isNull() || m_autoPlay))
    {
        if (m_error != MediaPlayer::EngineMissingError && m_error != MediaPlayer::NoError)
        {
            m_error = MediaPlayer::NoError;
            m_errorString = QString();
            emit errorChanged();
        }

        m_player->setMedia(m_media);
        m_loaded = true;
    }
    else
    {
        emit sourceChanged();
    }

    if (m_autoPlay)
        m_player->play();
}

QmlMediaPlaylist *QmlMediaPlayer::playlist() const
{        
    return m_playlist;
}

void QmlMediaPlayer::setPlaylist(QmlMediaPlaylist *playlist)
{
    if (playlist == m_playlist && m_source.isEmpty())
        return;

    if (!m_source.isEmpty())
    {
        m_source.clear();
        emit sourceChanged();
    }

    m_playlist = playlist;
    m_media = m_playlist ? Media(m_playlist->mediaPlaylist(), QUrl(), false) : Media();
    m_loaded = false;
    if (m_componentCompleted && (m_autoLoad || m_media.isNull() || m_autoPlay))
    {
        if (m_error != MediaPlayer::EngineMissingError && m_error != MediaPlayer::NoError) {
            m_error = MediaPlayer::NoError;
            m_errorString = QString();
            emit errorChanged();
        }

        if (!playlist)
            m_playlistMediaChanged = true;
        m_player->setMedia(m_media);
        m_loaded = true;
    }
    else
        emit playlistChanged();

    if (m_autoPlay)
        m_player->play();
}

QmlMediaPlayer::PlaybackState QmlMediaPlayer::playbackState() const
{
    return PlaybackState(m_playbackState);
}

void QmlMediaPlayer::setPlaybackState(MediaPlayer::PlaybackState playbackState)
{
    if(m_playbackState == playbackState)
        return;

    if(m_componentCompleted)
    {
        switch (playbackState)
        {
        case MediaPlayer::PlayingState:
            if(!m_loaded)
            {
                m_player->setMedia(m_media);
                m_player->setPosition(0);
                m_loaded = true;
            }
            m_player->play();
            break;
        case MediaPlayer::PausedState:
            if(!m_loaded)
            {
                m_player->setMedia(m_media);
                m_player->setPosition(0);
                m_loaded = true;
            }
            m_player->pause();
            break;
        case MediaPlayer::StoppedState:
            m_player->stop();
            break;
        }
    }
}

TimeTick QmlMediaPlayer::time() const
{
    return !m_componentCompleted ? TICK_INVALID : m_time;
}

void QmlMediaPlayer::setTime(TimeTick ms)
{
    if(m_time == ms)
        return;

    if (m_componentCompleted)
    {
        m_player->setTime(ms);
    }
    else
    {
        m_time = ms;
        emit timeChanged();
    }
}

qreal QmlMediaPlayer::rate() const
{
    return !m_componentCompleted ? m_rate : static_cast<double>(m_player->rate());
}

void QmlMediaPlayer::setRate(qreal rate)
{
    if(qFuzzyCompare(this->rate(), rate))
        return;

    if (m_componentCompleted)
    {
        m_player->setRate(static_cast<float>(rate));
    }
    else
    {
        m_rate = rate;
        emit rateChanged();
    }
}

qreal QmlMediaPlayer::volume() const
{
    return !m_componentCompleted ? m_volume : qreal(m_player->volume()) / 100;
}

void QmlMediaPlayer::setVolume(qreal volume)
{
    if (volume < 0 || volume > 1)
    {
        qmlWarning(this) << tr("volume should be between 0.0 and 1.0");
        return;
    }

    if(qFuzzyCompare(this->volume(), volume))
        return;

    if (m_componentCompleted)
    {
        m_player->setVolume(qRound(volume * 100));
    }
    else
    {
        m_volume = volume;
        emit volumeChanged();
    }
}

bool QmlMediaPlayer::isMuted() const
{
    return !m_componentCompleted ? m_muted : m_player->isMuted();
}

void QmlMediaPlayer::setMuted(bool muted)
{
    if (isMuted() == muted)
        return;

    if (m_componentCompleted)
    {
        m_player->setMuted(muted);
    }
    else
    {
        m_muted = muted;
        emit mutedChanged();
    }
}

int QmlMediaPlayer::loopCount() const
{
    return m_loopCount;
}

void QmlMediaPlayer::setLoopCount(int loopCount)
{
    if (loopCount == 0)
        loopCount = 1;
    else if (loopCount < -1)
        loopCount = -1;

    if (m_loopCount == loopCount)
        return;

    m_loopCount = loopCount;
    m_runningCount = loopCount - 1;
    emit loopCountChanged();
}

bool QmlMediaPlayer::autoPlay() const
{
    return m_autoPlay;
}

void QmlMediaPlayer::setAutoPlay(bool autoplay)
{
    if (m_autoPlay == autoplay)
        return;

    m_autoPlay = autoplay;
    emit autoPlayChanged();
}

bool QmlMediaPlayer::isAutoLoad() const
{
    return m_autoLoad;
}

void QmlMediaPlayer::setAutoLoad(bool autoLoad)
{
    if (m_autoLoad == autoLoad)
        return;

    m_autoLoad = autoLoad;
    emit autoLoadChanged();
}

bool QmlMediaPlayer::hasAudio() const
{
    return !m_componentCompleted ? false : m_player->isAudioAvailable();
}

bool QmlMediaPlayer::hasVideo() const
{
    return !m_componentCompleted ? false : m_player->isVideoAvailable();
}

QmlMediaPlayer::Error QmlMediaPlayer::error() const
{
    return Error(m_error);
}

QString QmlMediaPlayer::errorString() const
{
    return m_errorString;
}

void QmlMediaPlayer::play()
{
    if(!m_componentCompleted)
        return;

    setPlaybackState(MediaPlayer::PlayingState);
}

void QmlMediaPlayer::pause()
{
    if(!m_componentCompleted)
        return;

    setPlaybackState(MediaPlayer::PausedState);
}

void QmlMediaPlayer::togglePause()
{
    if(!m_componentCompleted)
        return;

    m_player->togglePause();
}

void QmlMediaPlayer::resume()
{
    if(!m_componentCompleted)
        return;

    m_player->resume();
}

void QmlMediaPlayer::stop()
{
    if(!m_componentCompleted)
        return;

    setPlaybackState(MediaPlayer::StoppedState);
}

void QmlMediaPlayer::seek(qreal position)
{
    if (m_position < 0)
        m_position = 0;

    if (qFuzzyCompare(this->position(), position))
        return;

    m_player->setPosition(position);
    if (m_componentCompleted)
    {
        m_player->setPosition(position);
    }
    else
    {
        m_position = position;
        emit positionChanged();
    }
}

void QmlMediaPlayer::previous()
{
    if(!m_componentCompleted)
        return;

    m_player->previous();
}

void QmlMediaPlayer::next()
{
    if(!m_componentCompleted)
        return;

    m_player->next();
}

////////////////////////////////////////////////////
/// \brief private slots hanlder
///////////////////////////////////////////////////

void QmlMediaPlayer::updatePlaybackState()
{
    updateMediaStatus();
}

void QmlMediaPlayer::updateMediaStatus()
{
    if (m_player->mediaStatus() == MediaPlayer::EndOfMedia && m_runningCount != 0)
    {
        m_runningCount = std::max(m_runningCount - 1, -2);
        m_player->play();
    }

    const MediaPlayer::MediaStatus oldMediaStatus = m_mediaStatus;
    const MediaPlayer::PlaybackState lastPlaybackState = m_playbackState;
    const MediaPlayer::PlaybackState currentPlaybackState = m_player->playbackState();

    m_playbackState = currentPlaybackState;
    m_mediaStatus = m_player->mediaStatus();

    if (m_mediaStatus != oldMediaStatus)
        emit mediaStatusChanged();

    if (lastPlaybackState != currentPlaybackState)
    {
        if (lastPlaybackState == MediaPlayer::StoppedState)
            m_runningCount = m_loopCount - 1;

        switch (currentPlaybackState)
        {
        case MediaPlayer::StoppedState:
            emit stopped();
            break;
        case MediaPlayer::PausedState:
            emit paused();
            break;
        case MediaPlayer::PlayingState:
            emit playing();
            break;
        }

        emit playbackStateChanged();
    }
}

void QmlMediaPlayer::updateMedia(const Media &media)
{
    if (!media.playlist() && !m_playlistMediaChanged)
    {
        emit sourceChanged();
    }
    else
    {
        m_playlistMediaChanged = false;
        emit playlistChanged();
    }
}

void QmlMediaPlayer::updateTime(qint64 time)
{
    m_time = time;
    emit timeChanged();
}

void QmlMediaPlayer::updateError(MediaPlayer::Error err)
{
    m_error = err;
    m_errorString = m_player->errorString();

    emit error(Error(err), m_errorString);
    emit errorChanged();
}
