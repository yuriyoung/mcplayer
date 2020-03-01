#ifndef VLCPLAYERCONTROL_H
#define VLCPLAYERCONTROL_H

#include "player/MediaPlayerControl.h"
#include "VLCEngine.h"

struct libvlc_media_player_t;
struct libvlc_instance_t;
struct libvlc_media_t;

class VLCPlayerControlPrivate;
class VLCPlayerControl : public MediaPlayerControl
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(VLCPlayerControl)
public:
    VLCPlayerControl(VLCEngine *engine, QObject *parent = nullptr);
    ~VLCPlayerControl();

    MediaPlayer::PlaybackState playbackState() const;
    MediaPlayer::MediaStatus mediaStatus() const;

    int bufferStatus() const;
    qint64 duration() const;

    virtual qint64 time() const;
    virtual void setTime(qint64 ms);

    double position() const;
    void setPosition(double position);

    int volume() const;
    void setVolume(int volume);

    bool isMuted() const;
    void setMuted(bool muted);

    bool isAudioAvailable() const;
    bool isVideoAvailable() const;

    bool isSeekable() const;

    float rate() const;
    void setRate(float rate);

    Media media() const;
    const QIODevice *mediaStream() const;
    void setMedia(const Media &media, QIODevice *stream = nullptr);

    void play();
    void pause();
    void togglePause();
    void resume();
    void stop();

signals:
    void opening();
    void buffering(float);
    void playing();
    void paused();
    void stopped();
    void forward();
    void backward();
    void end();
    void vout(int count);

private:
    QScopedPointer<VLCPlayerControlPrivate> d_ptr;
};

#endif // VLCPLAYERCONTROL_H
