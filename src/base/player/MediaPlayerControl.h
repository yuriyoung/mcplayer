#ifndef MEDIAPLAYERCONTROL_H
#define MEDIAPLAYERCONTROL_H

#include "MediaControl.h"
#include "MediaPlayer.h"
#include "Media.h"

#include <QIODevice>

class MediaPlayerControl : public MediaControl
{
    Q_OBJECT
public:
    virtual MediaPlayer::PlaybackState playbackState() const = 0;
    virtual MediaPlayer::MediaStatus mediaStatus() const = 0;

    virtual qint64 duration() const = 0;

    virtual qint64 time() const = 0;
    virtual void setTime(qint64 ms) = 0;

    virtual double position() const = 0;
    virtual void setPosition(double position) = 0;

    virtual int volume() const = 0;
    virtual void setVolume(int volume) = 0;

    virtual bool isMuted() const = 0;
    virtual void setMuted(bool muted) = 0;

    virtual int bufferStatus() const = 0;

    virtual bool isAudioAvailable() const = 0;
    virtual bool isVideoAvailable() const = 0;

    virtual bool isSeekable() const = 0;

    virtual float rate() const = 0;
    virtual void setRate(float rate) = 0;

    virtual Media media() const = 0;
    virtual const QIODevice *mediaStream() const = 0;
    virtual void setMedia(const Media &media, QIODevice *stream = nullptr) = 0;

    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void togglePause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;

signals:
    void mediaChanged(const Media& media);
    void timeChanged(qint64 time);
    void durationChanged(qint64 duration);
    void positionChanged(double position);
    void stateChanged(MediaPlayer::PlaybackState newState);
    void mediaStatusChanged(MediaPlayer::MediaStatus status);
    void volumeChanged(int volume);
    void mutedChanged(bool mute);
    void audioAvailableChanged(bool audioAvailable);
    void videoAvailableChanged(bool videoAvailable);
    void bufferStatusChanged(int percentFilled);
    void seekableChanged(bool seekable);
    void rateChanged(float rate);
    void titleChanged(int title);
    void snapshotTaken(const QString &fileName);
    void voutChanged(int count);
    void opening();
    void buffering(float);
    void playing();
    void paused();
    void stopped();
    void forward();
    void backward();
    void end();
    void error(int error, const QString &errorString);

protected:
    explicit MediaPlayerControl(QObject *parent = nullptr);
};

#define MediaPlayerControl_iid "org.mcplayer.mediaplayercontrol/1.0"
MEDIA_DECLARE_CONTROL(MediaPlayerControl, MediaPlayerControl_iid)

#endif // MEDIAPLAYERCONTROL_H
