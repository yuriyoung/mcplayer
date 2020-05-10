#include "VLCMetadataControl.h"
#include "VLCPlayerControl.h"
#include "Metadata.h"

#include <vlc/vlc.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcVLCMetadataControl, "mcplayer.VLCMetadataControl")


typedef QMap<libvlc_meta_t, QString> VLCMetaDataKeyLookup;
Q_GLOBAL_STATIC(VLCMetaDataKeyLookup, metadataKeys)

/**
libvlc_meta_Title,
libvlc_meta_Artist,
libvlc_meta_Genre,
libvlc_meta_Copyright,
libvlc_meta_Album,
libvlc_meta_TrackNumber,
libvlc_meta_Description,
libvlc_meta_Rating,
libvlc_meta_Date,
libvlc_meta_Setting,
libvlc_meta_URL,
libvlc_meta_Language,
libvlc_meta_NowPlaying,
libvlc_meta_Publisher,
libvlc_meta_EncodedBy,
libvlc_meta_ArtworkURL,
libvlc_meta_TrackID,
libvlc_meta_TrackTotal,
libvlc_meta_Director,
libvlc_meta_Season,
libvlc_meta_Episode,
libvlc_meta_ShowName,
libvlc_meta_Actors,
libvlc_meta_AlbumArtist,
libvlc_meta_DiscNumber,
libvlc_meta_DiscTotal
*/
static const QList<libvlc_meta_t> vlcMetaTypeKeys =
{
    libvlc_meta_Title,
    libvlc_meta_Artist,
    libvlc_meta_Genre,
    libvlc_meta_Copyright,
    libvlc_meta_Album,
    libvlc_meta_TrackNumber,
    libvlc_meta_Description,
    libvlc_meta_Rating,
    libvlc_meta_Date,
    libvlc_meta_Setting,
    libvlc_meta_URL,
    libvlc_meta_Language,
    libvlc_meta_NowPlaying,
    libvlc_meta_Publisher,
    libvlc_meta_EncodedBy,
    libvlc_meta_ArtworkURL,
    libvlc_meta_TrackID,
    libvlc_meta_TrackTotal,
    libvlc_meta_Director,
    libvlc_meta_Season,
    libvlc_meta_Episode,
    libvlc_meta_ShowName,
    libvlc_meta_Actors,
    libvlc_meta_AlbumArtist,
    libvlc_meta_DiscNumber,
    libvlc_meta_DiscTotal
};

static const VLCMetaDataKeyLookup *vlcMetaDataKeys()
{
    if(metadataKeys->isEmpty())
    {
        metadataKeys->insert(libvlc_meta_Title,       Metadata::Title);
        metadataKeys->insert(libvlc_meta_Artist,      Metadata::Author);
        metadataKeys->insert(libvlc_meta_Genre,       Metadata::Genre);
        metadataKeys->insert(libvlc_meta_Copyright,   Metadata::Copyright);
        metadataKeys->insert(libvlc_meta_Album,       Metadata::AlbumTitle);
        metadataKeys->insert(libvlc_meta_TrackNumber, Metadata::TrackNumber);
        metadataKeys->insert(libvlc_meta_Description, Metadata::Description);
        metadataKeys->insert(libvlc_meta_Rating,      Metadata::ParentalRating);
        metadataKeys->insert(libvlc_meta_Date,        Metadata::Date);
        metadataKeys->insert(libvlc_meta_Setting,     "Setting");
        metadataKeys->insert(libvlc_meta_URL,         Metadata::URL);
        metadataKeys->insert(libvlc_meta_Language,    Metadata::Language);
        metadataKeys->insert(libvlc_meta_NowPlaying,  Metadata::NowPlaying);
        metadataKeys->insert(libvlc_meta_Publisher,   Metadata::Publisher);
        metadataKeys->insert(libvlc_meta_EncodedBy,   Metadata::EncodedBy);
        metadataKeys->insert(libvlc_meta_ArtworkURL,  "ArtworkURL");
        metadataKeys->insert(libvlc_meta_TrackID,     Metadata::TrackId);
        metadataKeys->insert(libvlc_meta_TrackTotal,  Metadata::TrackCount);
        metadataKeys->insert(libvlc_meta_Director,    Metadata::Director);
        metadataKeys->insert(libvlc_meta_Season,      Metadata::Season);
        metadataKeys->insert(libvlc_meta_Episode,     Metadata::Episode);
        metadataKeys->insert(libvlc_meta_ShowName,    Metadata::ShowName);
        metadataKeys->insert(libvlc_meta_Actors,      Metadata::Actors);
        metadataKeys->insert(libvlc_meta_AlbumArtist, Metadata::AlbumArtist);
        metadataKeys->insert(libvlc_meta_DiscNumber,  Metadata::DiscNumber);
        metadataKeys->insert(libvlc_meta_DiscTotal,   Metadata::DiscTotal);
    }

    return metadataKeys;
}

class VLCMetadataControlPrivate
{
    Q_DECLARE_PUBLIC(VLCMetadataControl)
public:
    VLCMetadataControlPrivate(VLCMetadataControl *q) : q_ptr(q) { }

    QString meta(libvlc_media_t *mdeia, libvlc_meta_t meta_id);
    void updateMetaData();

    void attachMediaEvents(libvlc_media_t *media);
    void detachMediaEvents();
    static void processEvents(const libvlc_event_t *event, void *data);

    VLCMetadataControl *q_ptr = nullptr;
    VLCEngine *engine = nullptr;
    QVariantMap metadata;
    libvlc_media_t *vlcMedia = nullptr;
    libvlc_event_manager_t *vlcEvent = nullptr;
};

QString VLCMetadataControlPrivate::meta(libvlc_media_t *mdeia, libvlc_meta_t meta_id)
{
    if(!mdeia)
        QString();

    QString result;
    if(char * meta = libvlc_media_get_meta(mdeia, meta_id))
        result = meta;

    return result;
}

void VLCMetadataControlPrivate::updateMetaData()
{
    metadata.clear();
    foreach (auto id, vlcMetaTypeKeys)
    {
        const QString key = vlcMetaDataKeys()->value(id, QString::number(id));
        metadata.insert(key, meta(vlcMedia, id));
    }

    if(!metadata.value(Metadata::Title).toBool())
    {
        metadata[Metadata::Title] = libvlc_media_get_mrl(vlcMedia);
    }
}


void VLCMetadataControlPrivate::attachMediaEvents(libvlc_media_t *media)
{
    vlcEvent = libvlc_media_event_manager(media);
    libvlc_event_attach(vlcEvent, libvlc_MediaParsedChanged, processEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaMetaChanged, processEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaDurationChanged, processEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaStateChanged, processEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaFreed, processEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaSubItemAdded, processEvents, this);
}

void VLCMetadataControlPrivate::detachMediaEvents()
{
    if(vlcEvent)
    {
        libvlc_event_detach(vlcEvent, libvlc_MediaParsedChanged, processEvents, this);
        libvlc_event_detach(vlcEvent, libvlc_MediaMetaChanged, processEvents, this);
        libvlc_event_detach(vlcEvent, libvlc_MediaDurationChanged, processEvents, this);
        libvlc_event_detach(vlcEvent, libvlc_MediaStateChanged, processEvents, this);
        libvlc_event_detach(vlcEvent, libvlc_MediaFreed, processEvents, this);
        libvlc_event_detach(vlcEvent, libvlc_MediaSubItemAdded, processEvents, this);
    }
}

void VLCMetadataControlPrivate::processEvents(const libvlc_event_t *event, void *data)
{
    VLCMetadataControlPrivate *d = static_cast<VLCMetadataControlPrivate *>(data);
    Q_ASSERT(d);

    switch (event->type)
    {
    case libvlc_MediaParsedChanged:
        qDebug(lcVLCMetadataControl) << "libvlc_MediaParsedChanged";
        d->updateMetaData();
        emit d->q_func()->metadataChanged();
        break;
    case libvlc_MediaMetaChanged:
    {
        qDebug(lcVLCMetadataControl) << "libvlc_MediaMetaChanged";
        libvlc_meta_t id = event->u.media_meta_changed.meta_type;
        QString key = vlcMetaDataKeys()->value(id, QString::number(id));
        QVariant value = d->meta(d->vlcMedia, event->u.media_meta_changed.meta_type);
        if(value.toBool())
        {
            d->metadata.insert(key, value);
            emit d->q_func()->metadataChanged(key, value);
        }
    }
        break;
    case libvlc_MediaDurationChanged:
        qDebug(lcVLCMetadataControl) << "libvlc_MediaDurationChanged";
         emit d->engine->playerControl()->durationChanged(libvlc_media_get_duration(d->vlcMedia));
        break;
    case libvlc_MediaStateChanged:
        qDebug(lcVLCMetadataControl) << "libvlc_MediaStateChanged";
        break;
    case libvlc_MediaFreed:
        qDebug(lcVLCMetadataControl) << "libvlc_MediaFreed";
        d->detachMediaEvents();
        break;
    case libvlc_MediaSubItemAdded:
        qDebug(lcVLCMetadataControl) << "libvlc_MediaSubItemAdded";
        break;
    }
}


/**
 * @brief VLCMetadataControl::VLCMetadataControl
 * @param engine
 * @param parent
 */
VLCMetadataControl::VLCMetadataControl(VLCEngine *engine, QObject *parent)
    : MediaMetadataControl(parent), d_ptr(new VLCMetadataControlPrivate(this))
{
    Q_D(VLCMetadataControl);
    d->engine = engine;
}

VLCMetadataControl::~VLCMetadataControl()
{
    Q_D(VLCMetadataControl);
    qDebug(lcVLCMetadataControl) << "VLCMetadataControl::~VLCMetadataControl()";

    if(d->vlcMedia)
        libvlc_media_release(d->vlcMedia);

    if(d->vlcEvent)
    {
        d->detachMediaEvents();
    }
}

bool VLCMetadataControl::isMetadataAvailable() const
{
    Q_D(const VLCMetadataControl);
    return !d->metadata.isEmpty();
}

QStringList VLCMetadataControl::availableMetadata() const
{
    Q_D(const VLCMetadataControl);
    return d->metadata.keys();
}

bool VLCMetadataControl::isWritable() const
{
    return false;
}

QVariant VLCMetadataControl::metadata(const QString &key) const
{
    Q_D(const VLCMetadataControl);
    return d->metadata.value(key);
}

void VLCMetadataControl::setMetadata(const QString &key, const QVariant &value)
{
    Q_UNUSED(key)
    Q_UNUSED(value)
}

void VLCMetadataControl::parseMediaAsync(libvlc_media_t *media, int type)
{
    Q_D(VLCMetadataControl);
    if(!media)
    {
        qWarning(lcVLCMetadataControl) << "Invalid media";
        return;
    }

    qInfo(lcVLCMetadataControl) << "async pasre media metadata";

    //! importand: increment the reference count of a media object avoid release out side
    //! but do not forget call libvlc_media_release() to decrement reference count
    libvlc_media_release(d->vlcMedia);
    d->vlcMedia = media;
    libvlc_media_retain(media);
    d->attachMediaEvents(d->vlcMedia);

    libvlc_media_parse_flag_t flag = libvlc_media_parse_flag_t(type);
    libvlc_media_parse_with_options(d->vlcMedia, flag, -1); // -1 used default timeout
}
