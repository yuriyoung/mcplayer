#ifndef LOCALMEDIAPLAYLISTCONTROL_H
#define LOCALMEDIAPLAYLISTCONTROL_H

#include "MediaPlaylistControl.h"

class MediaPlaylistNavigator;
class LocalMediaPlaylistControl : public MediaPlaylistControl
{
    Q_OBJECT
public:
    explicit LocalMediaPlaylistControl(QObject *parent = nullptr);
    ~LocalMediaPlaylistControl();

    MediaPlaylistProvider *playlistProvider() const;
    bool setPlaylistProvider(MediaPlaylistProvider *provider);

    int currentIndex() const;
    void setCurrentIndex(int index);

    int nextIndex(int steps) const;
    int previousIndex(int steps) const;

    void next();
    void previous();

    MediaPlaylist::PlaybackMode playbackMode() const;
    void setPlaybackMode(MediaPlaylist::PlaybackMode mode);

signals:

public slots:

private:
    MediaPlaylistNavigator *m_navigator = nullptr;
};

#endif // LOCALMEDIAPLAYLISTCONTROL_H
