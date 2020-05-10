#include "VLCPlayerControl.h"
#include "VLCMetadataControl.h"

#include <vlc/vlc.h>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcVLCPlayerControl, "mcplayer.VLCPlayerControl")

class VLCPlayerControlPrivate
{
    Q_DECLARE_PUBLIC(VLCPlayerControl)
public:
    VLCPlayerControlPrivate(VLCPlayerControl *q) : q_ptr(q) {}

    void setMediaStatus(MediaPlayer::MediaStatus status);

    QString debugError() const;
    libvlc_media_t *toVlcMedia(const Media &media, int *flag);

    void attachEvents();
    void dettachEvents();
    static void processEvents(const libvlc_event_t *event, void *data);

    int bufferLevel = 0;
    Media currentMedia;
    MediaPlayer::MediaStatus mediaStatus = MediaPlayer::NoMedia;

    libvlc_media_t *currentVLCMedia = nullptr;
    libvlc_event_manager_t *vlcEvent = nullptr;
    libvlc_media_player_t *vlcPlayer = nullptr;
    libvlc_media_stats_t *stats = new libvlc_media_stats_t;

    VLCEngine *engine = nullptr;
    VLCPlayerControl *q_ptr = nullptr;
};

void VLCPlayerControlPrivate::setMediaStatus(MediaPlayer::MediaStatus status)
{
    Q_Q(VLCPlayerControl);
    if(mediaStatus == status)
        return;

    mediaStatus = status;
    emit q->mediaStatusChanged(mediaStatus);
}

QString VLCPlayerControlPrivate::debugError() const
{
    QString error;
    if (libvlc_errmsg())
    {
        error = QString::fromUtf8(libvlc_errmsg());
        libvlc_clearerr();
        qCritical(lcVLCPlayerControl) << libvlc_media_player_get_state(vlcPlayer) << error;
    }

    return error;
}

libvlc_media_t *VLCPlayerControlPrivate::toVlcMedia(const Media &media, int *flag)
{
    libvlc_media_t *vlc_media = nullptr;
    if(!media.isNull())
    {
        const QUrl &url = media.canonicalUrl();
        if(url.isLocalFile())
        {
            *flag = libvlc_media_parse_local;
            QString path = QDir::toNativeSeparators(media.canonicalUrl().toLocalFile());
            vlc_media = libvlc_media_new_path(engine->vlcInstance(), path.toUtf8().data());
        }
        else
        {
            *flag = libvlc_media_parse_network;
            QString urlString = media.canonicalUrl().toString();
            vlc_media = libvlc_media_new_location(engine->vlcInstance(), urlString.toUtf8().data());
        }
    }
    else
    {
        *flag = -1;
        vlc_media = libvlc_media_new_as_node(engine->vlcInstance(), "");
    }

    return vlc_media;
}

void VLCPlayerControlPrivate::attachEvents()
{
    qInfo(lcVLCPlayerControl) << "attach vlc events";

    for (int e = libvlc_MediaPlayerMediaChanged; e <= libvlc_MediaPlayerChapterChanged ; ++e)
        libvlc_event_attach(vlcEvent, static_cast<libvlc_event_e>(e), processEvents, this);
}

void VLCPlayerControlPrivate::dettachEvents()
{
    qInfo(lcVLCPlayerControl) << "dettach vlc events";

    for (int e = libvlc_MediaPlayerMediaChanged; e <= libvlc_MediaPlayerChapterChanged ; ++e)
        libvlc_event_detach(vlcEvent, static_cast<libvlc_event_e>(e), processEvents, this);
}

void VLCPlayerControlPrivate::processEvents(const libvlc_event_t *event, void *data)
{
    VLCPlayerControlPrivate *d = static_cast<VLCPlayerControlPrivate *>(data);
    Q_ASSERT(d);

    VLCPlayerControl *player = static_cast<VLCPlayerControl *>(d->q_func());
    switch (event->type)
    {
    case libvlc_MediaPlayerMediaChanged:
        emit player->mediaChanged(player->d_func()->currentMedia);
        qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerMediaChanged: "
                                    << player->d_func()->currentMedia.canonicalUrl();
        break;
    case libvlc_MediaPlayerNothingSpecial:
            player->d_func()->setMediaStatus(MediaPlayer::NoMedia);
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerNothingSpecial: ";
            break;
        case libvlc_MediaPlayerOpening:
            player->d_func()->setMediaStatus(MediaPlayer::LoadingMedia);
            emit player->opening();
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerOpening: ";
            break;
        case libvlc_MediaPlayerBuffering:
            player->d_func()->setMediaStatus(MediaPlayer::BufferingMedia);
            player->d_func()->bufferLevel = qRound(event->u.media_player_buffering.new_cache);
            emit player->buffering(event->u.media_player_buffering.new_cache);
            emit player->bufferStatusChanged(qRound(event->u.media_player_buffering.new_cache));
            break;
        case libvlc_MediaPlayerPlaying:
            emit player->stateChanged(MediaPlayer::PlayingState);
            emit player->playing();
//            player->d_func()->setMediaStatus(MediaPlayer::NowPlayingMedia);
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerPlaying: ";
            break;
        case libvlc_MediaPlayerPaused:
            player->stateChanged(MediaPlayer::PausedState);
            emit player->paused();
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerPaused: ";
            break;
        case libvlc_MediaPlayerStopped:
            player->stateChanged(MediaPlayer::StoppedState);
            emit player->stopped();
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerStopped: ";
            break;
        case libvlc_MediaPlayerForward:
            emit player->forward();
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerForward: ";
            break;
        case libvlc_MediaPlayerBackward:
            emit player->backward();
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerBackward: ";
            break;
        case libvlc_MediaPlayerEndReached:
            player->d_func()->setMediaStatus(MediaPlayer::EndOfMedia);
            emit player->end();
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerEndReached: ";
            break;
        case libvlc_MediaPlayerEncounteredError:
            emit player->error(MediaPlayer::ResourceError, player->d_func()->debugError());
            player->d_func()->setMediaStatus(MediaPlayer::InvalidMedia);
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerEncounteredError: ";
            player->d_func()->debugError();
            break;
        case libvlc_MediaPlayerTimeChanged:
            emit player->timeChanged(event->u.media_player_time_changed.new_time);
    //        qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerTimeChanged: " << event->u.media_player_time_changed.new_time;
//            libvlc_media_get_stats(d->vlcMedia, d->stats);
    //        qDebug() << player->d_func()->stats->i_read_bytes
    //            << player->d_func()->stats->f_input_bitrate
    //            << player->d_func()->stats->i_demux_read_bytes
    //            << player->d_func()->stats->f_demux_bitrate
    //            << player->d_func()->stats->i_demux_corrupted
    //            << player->d_func()->stats->i_demux_discontinuity
    //            << player->d_func()->stats->i_decoded_audio
    //            << player->d_func()->stats->i_played_abuffers
    //            << player->d_func()->stats->i_lost_abuffers;
            break;
        case libvlc_MediaPlayerPositionChanged:
            emit player->positionChanged(static_cast<double>(event->u.media_player_position_changed.new_position));
    //        qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerPositionChanged: "
    //                                    << event->u.media_player_position_changed.new_position;
            break;
        case libvlc_MediaPlayerSeekableChanged:
            emit player->seekableChanged(event->u.media_player_seekable_changed.new_seekable);
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerSeekableChanged: "
                                        << event->u.media_player_seekable_changed.new_seekable;
            break;
        case libvlc_MediaPlayerPausableChanged:
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerPausableChanged: "
                                        << event->u.media_player_pausable_changed.new_pausable;
            break;
        case libvlc_MediaPlayerTitleChanged:
            emit player->titleChanged(event->u.media_player_title_changed.new_title);
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerTitleChanged: "
                                        << event->u.media_player_title_changed.new_title;
            break;
        case libvlc_MediaPlayerSnapshotTaken:
            emit player->snapshotTaken(event->u.media_player_snapshot_taken.psz_filename);
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerSnapshotTaken: ";
            break;
        case libvlc_MediaPlayerLengthChanged:
            emit player->durationChanged(event->u.media_player_length_changed.new_length);
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerLengthChanged: "
                                        << event->u.media_player_length_changed.new_length;
            break;
        case libvlc_MediaPlayerVout:
            emit player->voutChanged(event->u.media_player_vout.new_count);
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerVout: "
                                        << event->u.media_player_vout.new_count;
            break;
        case libvlc_MediaPlayerMuted:
            emit player->mutedChanged(true);
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerMuted: ";
            break;
        case libvlc_MediaPlayerUnmuted:
            emit player->mutedChanged(false);
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerUnmuted: ";
            break;
        case libvlc_MediaPlayerAudioVolume:
    //        emit player->volumeChanged(qRound(event->u.media_player_audio_volume.volume * 100));
    //        qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerAudioVolume: "
    //                                    << event->u.media_player_audio_volume.volume;
            break;
        case libvlc_MediaPlayerAudioDevice:
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerAudioDevice: "
                                        << event->u.media_player_audio_device.device;
            break;
        case libvlc_MediaPlayerChapterChanged:
            qInfo(lcVLCPlayerControl()) << "libvlc_MediaPlayerChapterChanged: ";
            break;
        default:
            break;
    }
}

/**
 * @brief VLCPlayerControl::VLCPlayerControl
 * @param engine
 * @param parent
 */
VLCPlayerControl::VLCPlayerControl(VLCEngine *engine, QObject *parent)
    : MediaPlayerControl(parent), d_ptr(new VLCPlayerControlPrivate(this))
{
    Q_D(VLCPlayerControl);
    d->engine = engine;

    d->vlcPlayer = libvlc_media_player_new(d->engine->vlcInstance());
    libvlc_media_player_set_role(d->vlcPlayer, libvlc_role_Music);
//    libvlc_audio_output_set(d->vlcPlayer, "directsound");

    d->vlcEvent = libvlc_media_player_event_manager(d->vlcPlayer);
    d->attachEvents();
}

VLCPlayerControl::~VLCPlayerControl()
{
    Q_D(VLCPlayerControl);
    if(playbackState() != MediaPlayer::StoppedState)
        this->stop();

    libvlc_media_release(d->currentVLCMedia);
    d->dettachEvents();
    qDebug(lcVLCPlayerControl) << "VLCPlayerControl::~VLCPlayerControl()";
}

MediaPlayer::PlaybackState VLCPlayerControl::playbackState() const
{
    Q_D(const VLCPlayerControl);

    libvlc_state_t state = libvlc_media_player_get_state(d->vlcPlayer);
    switch (state)
    {
    case libvlc_Playing:
        return MediaPlayer::PlayingState;
    case libvlc_Paused:
        return MediaPlayer::PausedState;
    case libvlc_Stopped:
        return MediaPlayer::StoppedState;
    default:break;
    }

    return MediaPlayer::PlaybackState(state);
}

MediaPlayer::MediaStatus VLCPlayerControl::mediaStatus() const
{
    Q_D(const VLCPlayerControl);
    return d->mediaStatus;
}

int VLCPlayerControl::bufferStatus() const
{
    Q_D(const VLCPlayerControl);
    return d->bufferLevel;
}

qint64 VLCPlayerControl::duration() const
{
    Q_D(const VLCPlayerControl);
    return libvlc_media_player_get_length(d->vlcPlayer);
}

qint64 VLCPlayerControl::time() const
{
    Q_D(const VLCPlayerControl);
    return libvlc_media_player_get_time(d->vlcPlayer);
}

void VLCPlayerControl::setTime(qint64 ms)
{
    Q_D(VLCPlayerControl);
    libvlc_media_player_set_time(d->vlcPlayer, ms);
}

double VLCPlayerControl::position() const
{
    Q_D(const VLCPlayerControl);
    float p = libvlc_media_player_get_position(d->vlcPlayer);
    return p < 0 ? 0 : static_cast<double>(p);
}

void VLCPlayerControl::setPosition(double position)
{
    Q_D(VLCPlayerControl);
    libvlc_media_player_set_position(d->vlcPlayer, static_cast<float>(position));
}

int VLCPlayerControl::volume() const
{
    Q_D(const VLCPlayerControl);
    int val = libvlc_audio_get_volume(d->vlcPlayer);
    return  val < 0 ? 0 : val;
}

void VLCPlayerControl::setVolume(int volume)
{
    Q_D(VLCPlayerControl);
    const int newVolume = qBound(0, volume, 100);
    libvlc_audio_set_volume(d->vlcPlayer, newVolume);
}

bool VLCPlayerControl::isMuted() const
{
    Q_D(const VLCPlayerControl);
    return libvlc_audio_get_mute(d->vlcPlayer) == 0;
}

void VLCPlayerControl::setMuted(bool muted)
{
    Q_D(VLCPlayerControl);
    libvlc_audio_set_mute(d->vlcPlayer, muted);
}

bool VLCPlayerControl::isAudioAvailable() const
{
    Q_D(const VLCPlayerControl);
    return libvlc_audio_output_list_get(d->engine->vlcInstance()) != nullptr;
}

bool VLCPlayerControl::isVideoAvailable() const
{
    return false;
}

bool VLCPlayerControl::isSeekable() const
{
    Q_D(const VLCPlayerControl);
    return libvlc_media_player_is_seekable(d->vlcPlayer) != 0;
}

float VLCPlayerControl::rate() const
{
    Q_D(const VLCPlayerControl);
    return libvlc_media_player_get_rate(d->vlcPlayer);
}

void VLCPlayerControl::setRate(float rate)
{
    Q_D(VLCPlayerControl);
    int ret = libvlc_media_player_set_rate(d->vlcPlayer, static_cast<float>(rate));
    if(ret == 0)
    {
        emit rateChanged(rate);
    }
}

Media VLCPlayerControl::media() const
{
    Q_D(const VLCPlayerControl);
    return d->currentMedia;
}

const QIODevice *VLCPlayerControl::mediaStream() const
{
    // TODO: support QIODevice streams
    return nullptr;
}

void VLCPlayerControl::setMedia(const Media &media, QIODevice *stream)
{
    Q_UNUSED(stream)
    Q_D(VLCPlayerControl);
    d->currentMedia = media;
    int type;

    //! important
    libvlc_media_release(d->currentVLCMedia);
    d->currentVLCMedia = d->toVlcMedia(media, &type);

    if(!d->currentVLCMedia)
    {
        emit mediaStatusChanged(MediaPlayer::NoMedia);
        return;
    }

    libvlc_media_player_set_media(d->vlcPlayer, d->currentVLCMedia);

    // parse & read meta data
    d->engine->metadataControl()->parseMediaAsync(d->currentVLCMedia, type);
}

void VLCPlayerControl::play()
{
    Q_D(VLCPlayerControl);
    if(libvlc_media_player_play(d->vlcPlayer) != 0)
    {
        d->debugError();
    }
}

void VLCPlayerControl::pause()
{
    Q_D(VLCPlayerControl);
    libvlc_media_player_set_pause(d->vlcPlayer, true);
}

void VLCPlayerControl::togglePause()
{
    Q_D(VLCPlayerControl);
    libvlc_media_player_pause(d->vlcPlayer);
}

void VLCPlayerControl::resume()
{
    Q_D(VLCPlayerControl);
    libvlc_state_t state = libvlc_media_player_get_state(d->vlcPlayer);
    if(libvlc_Paused == state)
        libvlc_media_player_set_pause(d->vlcPlayer, false);
}

void VLCPlayerControl::stop()
{
    Q_D(VLCPlayerControl);
    libvlc_media_player_stop(d->vlcPlayer);
}
