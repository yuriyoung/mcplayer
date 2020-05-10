#ifndef QMLMEDIAPLAYER_H
#define QMLMEDIAPLAYER_H

#include "player/MediaPlayer.h"
#include "utils/TimeTick.h"

#include <QObject>
#include <QQmlParserStatus>
#include <QtQml>

class QmlMediaPlaylist;
class QmlMediaMetadata;

class QmlMediaPlayer : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_DISABLE_COPY(QmlMediaPlayer)
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)
    Q_PROPERTY(int loops READ loopCount WRITE setLoopCount NOTIFY loopCountChanged)
    Q_PROPERTY(bool autoPlay READ autoPlay WRITE setAutoPlay NOTIFY autoPlayChanged)
    Q_PROPERTY(bool autoLoad READ isAutoLoad WRITE setAutoLoad NOTIFY autoLoadChanged)
    Q_PROPERTY(MediaStatus mediaStatus READ mediaStatus NOTIFY mediaStatusChanged)
    Q_PROPERTY(TimeTick time READ time WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(TimeTick duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qreal position READ position NOTIFY positionChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool hasAudio READ hasAudio NOTIFY hasAudioChanged)
    Q_PROPERTY(bool hasVideo READ hasVideo NOTIFY hasVideoChanged)
    Q_PROPERTY(qreal bufferStatus READ bufferStatus NOTIFY bufferStatusChanged)
    Q_PROPERTY(bool seekable READ seekable NOTIFY seekableChanged)
    Q_PROPERTY(qreal rate READ rate WRITE setRate NOTIFY rateChanged)
    Q_PROPERTY(Error error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
    Q_PROPERTY(QmlMediaPlaylist *playlist READ playlist WRITE setPlaylist NOTIFY playlistChanged)
    Q_PROPERTY(QmlMediaMetadata *metadata READ metadata CONSTANT)
    Q_ENUMS(PlaybackState MediaStatus Error)
public:
    enum PlaybackState
    {
        PlayingState = MediaPlayer::PlayingState,
        PausedState =  MediaPlayer::PausedState,
        StoppedState = MediaPlayer::StoppedState
    };

    enum MediaStatus
    {
        UnknownStatus = MediaPlayer::UnknownStatus,
        NoMedia       = MediaPlayer::NoMedia,
        Loading       = MediaPlayer::LoadingMedia,
        Loaded        = MediaPlayer::LoadedMedia,
        Stalled       = MediaPlayer::StalledMedia,
        Buffering     = MediaPlayer::BufferingMedia,
        Buffered      = MediaPlayer::BufferedMedia,
        EndOfMedia    = MediaPlayer::EndOfMedia,
        InvalidMedia  = MediaPlayer::InvalidMedia
    };

    enum Error
    {
        NoError        = MediaPlayer::NoError,
        ResourceError  = MediaPlayer::ResourceError,
        FormatError    = MediaPlayer::FormatError,
        NetworkError   = MediaPlayer::NetworkError,
        AccessDenied   = MediaPlayer::AccessDeniedError,
        EngineMissing = MediaPlayer::EngineMissingError
    };

    explicit QmlMediaPlayer(QObject *parent = nullptr);
    ~QmlMediaPlayer();

    void classBegin();
    void componentComplete();

    QmlMediaMetadata *metadata() const;

    TimeTick duration() const;
    qreal position() const;
    bool seekable() const;

    qreal bufferStatus() const;
    MediaStatus mediaStatus() const;

    QUrl source() const;
    void setSource(const QUrl &url);

    // QmlPlaylist in QML:
    //! MediaPlayer {
    //!     playlist: Playlist {
    //!         MediaItem { source: "FILENAME1" }
    //!         MediaItem { source: "FILENAME2" }
    //!         MediaItem { source: "FILENAME3" }
    //!     }
    //! }
    QmlMediaPlaylist *playlist() const;
    void setPlaylist(QmlMediaPlaylist *playlist);

    PlaybackState playbackState() const;
    void setPlaybackState(MediaPlayer::PlaybackState playbackState);

    TimeTick time() const;
    void setTime(TimeTick ms);

    qreal rate() const;
    void setRate(qreal rate);

    qreal volume() const;
    void setVolume(qreal volume);

    bool isMuted() const;
    void setMuted(bool muted);

    int loopCount() const;
    void setLoopCount(int loopCount);

    bool autoPlay() const;
    void setAutoPlay(bool autoplay);

    bool isAutoLoad() const;
    void setAutoLoad(bool autoLoad);

    bool hasAudio() const;
    bool hasVideo() const;

    Error error() const;
    QString errorString() const;

signals:
    void sourceChanged();
    void timeChanged();
    void durationChanged();
    void positionChanged();
    void seekableChanged();
    void bufferStatusChanged();

    void playbackStateChanged();
    void mediaStatusChanged();
    void mediaObjectChanged();

    void rateChanged();
    void volumeChanged();
    void mutedChanged();
    void loopCountChanged();
    void playlistChanged();
    void autoPlayChanged();
    void autoLoadChanged();

    void hasAudioChanged();
    void hasVideoChanged();

    void paused();
    void stopped();
    void playing();

    void errorChanged();
    void error(QmlMediaPlayer::Error error, const QString &errorString);

public slots:
    void play();
    void pause();
    void togglePause();
    void resume();
    void stop();
    void seek(qreal position);
    void previous();
    void next();

private slots:
    void updatePlaybackState();
    void updateMediaStatus();
    void updateMedia(const Media &media);
    void updateTime(qint64 time);
    void updateError(MediaPlayer::Error err);

private:
    bool m_componentCompleted = false;
    bool m_autoPlay = false;
    bool m_autoLoad = true;
    bool m_loaded = false;
    int m_loopCount = 1;

    qreal m_position = 0.0;
    qreal m_rate = 1.0;
    TimeTick m_time = 0;
    bool m_muted = false;
    qreal m_volume = 1.0;

    int m_runningCount = 0;
    bool m_playlistMediaChanged = false;

    QmlMediaPlaylist *m_playlist = nullptr;
    MediaPlayer *m_player = nullptr;
    QUrl m_source;
    Media m_media;
    QString m_errorString;
    MediaPlayer::MediaStatus m_mediaStatus = MediaPlayer::NoMedia;
    MediaPlayer::PlaybackState m_playbackState = MediaPlayer::StoppedState;
    MediaPlayer::Error m_error = MediaPlayer::EngineMissingError;
    QScopedPointer<QmlMediaMetadata> m_metadata;
};

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE(QmlMediaPlayer))

#endif // QMLMEDIAPLAYER_H
