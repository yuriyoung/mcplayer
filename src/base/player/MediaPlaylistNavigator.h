#ifndef MEDIAPLAYLISTNAVIGATOR_H
#define MEDIAPLAYLISTNAVIGATOR_H

#include "MediaPlaylistProvider.h"
#include <QObject>

class MediaPlaylistNavigatorPrivate;
class MediaPlaylistNavigator : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MediaPlaylistNavigator)
    Q_DECLARE_PRIVATE(MediaPlaylistNavigator)
public:
    explicit MediaPlaylistNavigator(QObject *parent = nullptr);
    explicit MediaPlaylistNavigator(MediaPlaylistProvider *playlist, QObject *parent = nullptr);
    virtual ~MediaPlaylistNavigator();

    MediaPlaylistProvider *playlist() const;
    void setPlaylist(MediaPlaylistProvider *pls);

    MediaPlaylist::PlaybackMode playbackMode() const;

    Media currentItem() const;
    Media nextItem(int steps = 1) const;
    Media previousItem(int steps = 1) const;
    Media itemAt(int position) const;

    int currentIndex() const;
    int nextIndex(int steps = 1) const;
    int previousIndex(int steps = 1) const;

signals:
    void activated(const Media &media);
    void currentIndexChanged(int);
    void playbackModeChanged(MediaPlaylist::PlaybackMode mode);
    void surroundingItemsChanged();

public slots:
    void next();
    void previous();
    void jump(int position);
    void setPlaybackMode(MediaPlaylist::PlaybackMode mode);

private slots:
    void mediaInserted(int start, int end);
    void mediaRemoved(int start, int end);
    void mediaChanged(int start, int end);

private:
    QScopedPointer<MediaPlaylistNavigatorPrivate> d_ptr;
};

#endif // MEDIAPLAYLISTNAVIGATOR_H
