#include "VLCEngine.h"
#include "VLCPlayerControl.h"
#include "VLCMetadataControl.h"

#include <vlc/vlc.h>
#include <QCoreApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcVLCEngine, "mcplayer.VLCEngine")

class VLCEnginePrivate
{
    Q_DECLARE_PUBLIC(VLCEngine)
public:
    VLCEnginePrivate(VLCEngine *q)
        : q_ptr(q)
    {
        createVLCInstance();
    }

    libvlc_instance_t* createVLCInstance()
    {
        QVector<const char*> opts;

        opts.push_back( "--ignore-config" );
        opts.push_back( "--no-video" ); // just audio for current version
//        opts.push_back( "--aout=direcsound" );
//        opts.push_back( "--input-fast-seek" );
        opts.push_back( "--force-dolby-surround=1" );
//        opts.push_back( "-H" ); // print vlc help information
//        opts.push_back( "--advanced" ); // print vlc advanced help information
        opts.push_back( "--codec='avcodec'" );
//        opts.push_back( "" );
//        opts.push_back( "--video-filter=adjust" );
//        opts.push_back( "--no-video-title-show" );
//        opts.push_back( "--ffmpeg-hw" );
//        opts.push_back( "--avcodec-hw=any" );

        /*!
         * place the libvlc "plugins" folder same dir with the application
         * MacOS: McPlayer.app/Contents/MacOS/plugins
         * or
         * MacOS: McPlayer.app/Contents/Frameworks/vlc/plugins ?
         *
         */
        QByteArray pluginPath = qApp->applicationDirPath().append("/plugins").toUtf8();
        qputenv("VLC_PLUGIN_PATH", pluginPath);

        vlcIinstance = libvlc_new(opts.size(), opts.data());
        return vlcIinstance;
    }

    VLCEngine *q_ptr = nullptr;
    libvlc_instance_t *vlcIinstance = nullptr;
    VLCPlayerControl *playerControl = nullptr;
    VLCMetadataControl *metadataControl = nullptr;
};

VLCEngine::VLCEngine(QObject *parent)
    : MediaEngine(parent)
    , d_ptr(new VLCEnginePrivate(this))
{

}

VLCEngine::~VLCEngine()
{
    Q_D(VLCEngine);
    libvlc_release(d->vlcIinstance);
}

MediaControl *VLCEngine::requestControl(const char *name)
{
    Q_D(VLCEngine);
    if (qstrcmp(name, MediaPlayerControl_iid) == 0)
    {
        if(!d->playerControl)
        {
            d->playerControl = new VLCPlayerControl(this);
//            libvlc_retain(d->vlcIinstance);
        }

        return d->playerControl;
    }
    else if(qstrcmp(name, MediaMetadataControl_iid) == 0)
    {
        if(!d->metadataControl)
        {
            d->metadataControl = new VLCMetadataControl(this);
//            libvlc_retain(d->vlcIinstance);
        }

        return d->metadataControl;
    }

    return nullptr;
}

void VLCEngine::releaseControl(MediaControl *control)
{
    Q_D(VLCEngine);
    if (!control)
     {
         qWarning(lcVLCEngine) << "Attempted release of null control";
         return;
     }
    else if (control == d->playerControl)
    {
        qInfo(lcVLCEngine) << "release playerControl";
        d->playerControl = nullptr;
//        libvlc_release(d->vlcIinstance);
    }
    else if(control == d->metadataControl)
    {
        qInfo(lcVLCEngine) << "release metadataControl";
        d->metadataControl = nullptr;
    }

    qInfo(lcVLCEngine) << "release controls done";
    delete control;
}

libvlc_instance_t *VLCEngine::vlcInstance() const
{
    Q_D(const VLCEngine);
    return d->vlcIinstance;
}

VLCPlayerControl *VLCEngine::playerControl() const
{
    Q_D(const VLCEngine);
    return d->playerControl;
}

VLCMetadataControl *VLCEngine::metadataControl() const
{
    Q_D(const VLCEngine);
    return d->metadataControl;
}
