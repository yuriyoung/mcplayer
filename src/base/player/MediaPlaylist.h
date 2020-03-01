#ifndef MEDIAPLAYLIST_H
#define MEDIAPLAYLIST_H

#include "MediaBindable.h"
#include "Media.h"

#include <QObject>
#include <QIODevice>

class MediaPlaylistPrivate;
class MediaPlaylist : public QObject, public MediaBindable
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MediaPlaylist)
    Q_INTERFACES(MediaBindable)
    Q_PROPERTY(MediaPlaylist::PlaybackMode playbackMode READ playbackMode WRITE setPlaybackMode NOTIFY playbackModeChanged)
    Q_PROPERTY(Media currentMedia READ currentMedia NOTIFY currentMediaChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_ENUMS(PlaybackMode Error)

public:
    enum PlaybackMode
    {
        CurrentItemOnce,
        CurrentItemRepeat,
        Sequential,
        Loop,
        Random,
        Heartbeat
    };

    enum Error
    {
        NoError,
        FormatError,
        FormatNotSupportedError,
        NetworkError,
        AccessDeniedError
    };
    explicit MediaPlaylist(QObject *parent = nullptr);
    virtual ~MediaPlaylist() override;

    MediaObject *mediaObject() const override;

    int currentIndex() const;
    Media currentMedia() const;
    Media media(int index) const;

    int nextIndex(int steps = 1) const;
    int previousIndex(int steps = 1) const;

    PlaybackMode playbackMode() const;
    void setPlaybackMode(PlaybackMode mode);

    bool isEmpty() const;
    int mediaCount() const;
    bool isReadOnly() const;

    // playlist manager
    bool addMedia(const Media &media);
    bool addMedia(const QList<Media> &list);
    bool insertMedia(int index, const Media &media);
    bool insertMedia(int index, const QList<Media> &list);
    bool removeMedia(int start, int end);
    bool removeMedia(int start);
    bool moveMedia(int from, int to);
    bool clear();

    MediaPlaylist::Error error() const;
    QString errorString() const;

    // TODO: read/write plsylist
    void load(const QNetworkRequest &request, const char *format = nullptr);
    void load(const QUrl &location, const char *format = nullptr);
    void load(QIODevice *device, const char *format = nullptr);
    bool save(const QUrl &location, const char *format = nullptr);
    bool save(QIODevice * device, const char *format);

signals:
    void currentIndexChanged(int index);
    void playbackModeChanged(MediaPlaylist::PlaybackMode mode);
    void currentMediaChanged(const Media &media);
    void mediaChanged(int start, int end);

    void mediaAboutToInserted(int start, int end);
    void mediaInserted(int start, int end);

    void mediaAboutToRemoved(int start, int end);
    void mediaRemoved(int start, int end);

    void loadSucceed();
    void loadFailed();

public slots:
    void next();
    void previous();
    void shuffle();
    void setCurrentIndex(int index);

protected:
    bool setMediaObject(MediaObject *object) override;

private:
    QScopedPointer<MediaPlaylistPrivate> d_ptr;
};

Q_DECLARE_METATYPE(MediaPlaylist::PlaybackMode)
Q_DECLARE_METATYPE(MediaPlaylist::Error)

#endif // MEDIAPLAYLIST_H
