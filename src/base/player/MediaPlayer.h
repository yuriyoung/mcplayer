#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include "MediaObject.h"
#include "MediaPlaylist.h"
#include "Media.h"

#include <QObject>

class MediaPlayerPrivate;
class MediaPlayer : public MediaObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MediaPlayer)
    Q_DISABLE_COPY(MediaPlayer)
    Q_PROPERTY(Media media READ media WRITE setMedia NOTIFY mediaChanged)
    Q_PROPERTY(Media currentMedia READ currentMedia NOTIFY currentMediaChanged)
    Q_PROPERTY(MediaPlaylist* playlist READ playlist WRITE setPlaylist)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qreal position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool seekable READ seekable NOTIFY seekableChanged)
    Q_PROPERTY(qreal rate READ rate WRITE setRate NOTIFY rateChanged)
    Q_PROPERTY(PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)
    Q_PROPERTY(MediaStatus mediaStatus READ mediaStatus NOTIFY mediaStatusChanged)
    Q_PROPERTY(QString error READ errorString)
    Q_ENUMS(PlaybackState MediaStatus Error)
public:
    enum PlaybackState
    {
        StoppedState,
        PlayingState,
        PausedState
    };

    // TODO: add a NowPlayingMedia status ?
    enum MediaStatus
    {
        UnknownStatus,
        NoMedia,
//        NowPlayingMedia,
        LoadingMedia,
        LoadedMedia,
        StalledMedia,
        BufferingMedia,
        BufferedMedia,
        EndOfMedia,
        InvalidMedia
    };

    enum Error
    {
        NoError,
        ResourceError,
        FormatError,
        NetworkError,
        AccessDeniedError,
        EngineMissingError,
        MediaIsPlaylist
    };

    enum Flag
    {
        LowLatency = 0x01,
        StreamPlayback = 0x02,
        VideoSurface = 0x04
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    explicit MediaPlayer(QObject *parent = nullptr);
    ~MediaPlayer();

    static int hasSupport(const QString &mimeType, const QStringList& codecs = QStringList());
    static QStringList supportedMimeTypes(Flags flags = Flags());

    // impl a class VideoWidget (QWidget) for widget
//    void setVideoOutput(VideoWidget *);

     // impl a class GraphicsVideoItem (QGraphicsObject) for graph sceen view
//    void setVideoOutput(GraphicsVideoItem *);

     // impl a class VideoSurface (QObject) for qml
//    void setVideoOutput(VideoSurface *surface);

    qint64 duration() const;
    bool seekable() const;
    int bufferStatus() const;
    const QIODevice *mediaStream() const;

    PlaybackState playbackState() const;
    MediaStatus mediaStatus() const;

    Media currentMedia() const;
    Media media() const;
    void setMedia(const Media &media, QIODevice *stream = nullptr);

    MediaPlaylist *playlist() const;
    void setPlaylist(MediaPlaylist *playlist);

    qint64 time() const;
    void setTime(qint64 ms);

    float rate() const;
    void setRate(float rate);

    double position() const;
    void setPosition(double pos);

    int volume() const;
    void setVolume(int volume);

    bool isMuted() const;
    void setMuted(bool muted);

    bool isAudioAvailable() const;
    bool isVideoAvailable() const;

    MediaPlayer::Error error() const;
    QString errorString() const;

signals:
    void playbackStateChanged(MediaPlayer::PlaybackState newState);
    void mediaStatusChanged(MediaPlayer::MediaStatus mediaStatus);
    void mediaChanged(const Media &media);
    void currentMediaChanged(const Media &media);
    void timeChanged(qint64 time);
    void positionChanged(double position);
    void seekableChanged(bool seekable);
    void durationChanged(qint64 length);
    void rateChanged(qreal rate);
    void volumeChanged(int volume);
    void mutedChanged(bool muted);
    void bufferStatusChanged(int percentFilled);
    void voutChanged(int count);

    void opening();
    void buffering(float buffer);
    void playing();
    void paused();
    void stopped();
    void forward();
    void backward();
    void end();
    void error(MediaPlayer::Error error);

public slots:
    void play();
    void pause();
    void togglePause();
    void resume();
    void stop();
    void previous();
    void next();
};

Q_DECLARE_METATYPE(MediaPlayer::PlaybackState)
Q_DECLARE_METATYPE(MediaPlayer::MediaStatus)
Q_DECLARE_METATYPE(MediaPlayer::Error)

#endif // MEDIAPLAYER_H
