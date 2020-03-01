#include "MediaLibrary.h"
#include "MediaDiscoverer.h"
#include "utils/Lazy.h"

#include <QSharedPointer>

static const char* const supportedMediaExtensions[] = {
    "3g2", "3gp", "a52", "aac", "ac3", "adx", "aif", "aifc",
    "aiff", "alac", "amr", "amv", "aob", "ape", "asf",
    "avi", "divx", "dts", "dv",
    "flac", "flv", "gxf", "iso", "it", "itml",
    "m1v", "m2t", "m2ts", "m2v", "m4a", "m4b",
    "m4p", "m4v", "mid", "mka", "mkv", "mlp", "mod", "mov",
    "mp1", "mp2", "mp3", "mp4", "mpc", "mpeg", "mpeg1", "mpeg2",
    "mpeg4", "mpg", "mts", "mxf", "nsv", "nuv", "oga", "ogg",
    "ogm", "ogv", "ogx", "oma", "opus", "ps", "qtl",
    "rec", "rm", "rmi", "rmj", "rmvb", "s3m", "spx",
    "tod", "trp", "ts", "tta", "vob", "voc", "vqf",
    "vro", "w64", "wav", "webm", "wma", "wmv", "wmx",
    "wpl", "wv", "wvx", "xa", "xm"
};

static const char* const supportedPlaylistExtensions[] = {
    "asx", "b4s", "conf", /*"cue",*/ "ifo", "m3u", "m3u8", "pls", "ram", "sdp",
    "vlc", "wax", "xspf"
};

class MediaLibraryPrivate
{
public:
    MediaLibraryPrivate(MediaLibrary *q);

    MediaLibrary *q_ptr = nullptr;
    Util::Lazy<MediaDiscoverer> discoverer;
};

MediaLibraryPrivate::MediaLibraryPrivate(MediaLibrary *q)
    : q_ptr(q)
    , discoverer(LAZY_CREATE(MediaDiscoverer, q))
{

}

MediaLibrary::MediaLibrary(QObject *parent)
    : QObject(parent)
    , d(new MediaLibraryPrivate(this))
{
    // TODO: connetion discoverer signals and slots
    // ...
}

MediaLibrary::~MediaLibrary()
{

}

void MediaLibrary::addEntryPoint(const QString &entryPoint)
{
    d->discoverer->discover(entryPoint);
}

void MediaLibrary::removeEntryPoint(const QString &entryPoint)
{
    d->discoverer->remove(entryPoint);
}

void MediaLibrary::banFolder(const QString &entryPoint)
{
    d->discoverer->ban(entryPoint);
}

void MediaLibrary::unbanFolder(const QString &entryPoint)
{
    d->discoverer->unban(entryPoint);
}

void MediaLibrary::addDevice(const QString &uuid, const QString &path, bool removable)
{
    Q_UNUSED(uuid)
    Q_UNUSED(path)
    Q_UNUSED(removable)
}

void MediaLibrary::removeDevice(const QString &uuid, const QString &path)
{
    Q_UNUSED(uuid)
    Q_UNUSED(path)
}

void MediaLibrary::discover(const QString &entryPoint)
{
    Q_UNUSED(entryPoint)
    d->discoverer->discover(entryPoint);
}

void MediaLibrary::reload()
{
    d->discoverer->reload();
}

void MediaLibrary::reload(const QString &entryPoint)
{
    d->discoverer->reload(entryPoint);
}

void MediaLibrary::clean()
{

}

bool MediaLibrary::supportedMediaExtension(const QString &ext)
{
    return std::binary_search(std::begin(supportedMediaExtensions),
                              std::end(supportedMediaExtensions),
                              ext.toLatin1().data(),
                              [](const char *l, const char *r){ return _stricmp(l, r) < 0;});
}

bool MediaLibrary::supportedPlaylistExtension(const QString &ext)
{
    return std::binary_search(std::begin(supportedPlaylistExtensions),
                              std::end(supportedPlaylistExtensions),
                              ext.toLatin1().data(),
                              [](const char *l, const char *r){ return _stricmp(l, r) < 0;});
}
