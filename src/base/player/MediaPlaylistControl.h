#ifndef MEDIAPLAYLISTCONTROL_H
#define MEDIAPLAYLISTCONTROL_H

#include "MediaControl.h"
#include "MediaPlaylist.h"

#include <QObject>

class MediaPlaylistProvider;
class MediaPlaylistControl : public MediaControl
{
    Q_OBJECT
public:
    virtual ~MediaPlaylistControl();

    virtual MediaPlaylistProvider *playlistProvider() const = 0;
    virtual bool setPlaylistProvider(MediaPlaylistProvider *provider) = 0;

    virtual int currentIndex() const = 0;
    virtual void setCurrentIndex(int index) = 0;

    virtual int nextIndex(int steps) const = 0;
    virtual int previousIndex(int steps) const = 0;

    virtual void next() = 0;
    virtual void previous() = 0;

    virtual MediaPlaylist::PlaybackMode playbackMode() const = 0;
    virtual void setPlaybackMode(MediaPlaylist::PlaybackMode mode) = 0;

signals:
    void playlistProviderChanged();
    void currentIndexChanged(int index);
    void currentMediaChanged(const Media &);
    void playbackModeChanged(MediaPlaylist::PlaybackMode mode);

protected:
    explicit MediaPlaylistControl(QObject *parent = nullptr);
};

#define MediaPlaylistControl_iid "org.mcplayer.mediaplaylistcontrol/1.0"
MEDIA_DECLARE_CONTROL(MediaPlaylistControl, MediaPlaylistControl_iid)

#endif // MEDIAPLAYLISTCONTROL_H
