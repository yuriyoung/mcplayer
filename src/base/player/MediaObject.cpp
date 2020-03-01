#include "MediaObject.h"
#include "MediaObject_p.h"
#include "MediaEngine.h"
#include "MediaBindable.h"
#include "MediaMetadataControl.h"

#include <QMetaObject>
#include <QMetaProperty>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcMediaObject, "mcplayer.MediaObject")

MediaObject::MediaObject(QObject *parent, MediaEngine *engine)
    : MediaObject(*new MediaObjectPrivate, engine, parent)
{

}

MediaObject::MediaObject(MediaObjectPrivate &dd, MediaEngine *engine, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    Q_D(MediaObject);
    d->q_ptr = this;
    d->engine = engine;

    d->notifyTimer = new QTimer(this);
    d->notifyTimer->setInterval(1000);
    connect(d->notifyTimer, &QTimer::timeout, this, &MediaObject::notify);

    if(d->engine)
    {
        d->metadataControl = d->engine->requestControl<MediaMetadataControl *>();
        if(d->metadataControl)
        {
            connect(d->metadataControl, SIGNAL(metadataChanged()),
                             this, SIGNAL(metadataChanged()));
            connect(d->metadataControl, SIGNAL(metadataChanged(QString, QVariant)),
                             this, SIGNAL(metadataChanged(QString, QVariant)));
            connect(d->metadataControl, SIGNAL(metadataAvailableChanged(bool)),
                             this, SIGNAL(metadataAvailableChanged(bool)));
            connect(d->metadataControl, SIGNAL(writableChanged(bool)),
                             this, SIGNAL(writableChanged(bool)));
        }
    }
}

MediaObject::~MediaObject()
{
    Q_D(const MediaObject);
    d->notifyTimer->deleteLater();
//    d->engine->releaseControl(d->metadataControl);
}

bool MediaObject::isAvailable() const
{
    Q_D(const MediaObject);
    if (d->engine == nullptr)
        return false;

    return true;
}

MediaEngine *MediaObject::engine() const
{
    Q_D(const MediaObject);
    return d->engine;
}

void MediaObject::watchProperty(const QByteArray &name)
{
    Q_D(MediaObject);
    const QMetaObject *mo = metaObject();
    int index = mo->indexOfProperty(name.constData());
    if(index != -1 && mo->property(index).hasNotifySignal())
    {
        d->notifyProperties.insert(index);
        if(!d->notifyTimer->isActive())
            d->notifyTimer->start();
    }
}

void MediaObject::unwatchProperty(const QByteArray &name)
{
    Q_D(MediaObject);
    int index = metaObject()->indexOfProperty(name.constData());
    if (index != -1)
    {
        d->notifyProperties.remove(index);
        if (d->notifyProperties.isEmpty())
            d->notifyTimer->stop();
    }
}

bool MediaObject::bind(QObject *bindable)
{
    qDebug(lcMediaObject) << "bind bindable object";
    MediaBindable *mb = qobject_cast<MediaBindable *>(bindable);
    if(!mb)
    {
        qDebug(lcMediaObject) << "bind failed: is not a bindable object";
        return false;
    }

    MediaObject *mo = mb->mediaObject();
    if(this == mo)
    {
        qDebug(lcMediaObject) << "object has been binded." << mo->objectName();
        return true;
    }

    if(mo)
        mo->unbind(bindable);

    return mb->setMediaObject(this);
}

void MediaObject::unbind(QObject *bindable)
{
    qDebug(lcMediaObject) << "unbind bindable object";

    MediaBindable *mb = qobject_cast<MediaBindable *>(bindable);
    if(mb && mb->mediaObject() == this)
        mb->setMediaObject(nullptr);
    else
        qWarning("MediaObject: Trying to undind not connected bindable object");
}

int MediaObject::notifyInterval() const
{
    Q_D(const MediaObject);
    return d->notifyTimer->interval();
}

void MediaObject::setNotifyInterval(int milliSeconds)
{
    Q_D(MediaObject);
    if(d->notifyTimer->interval() != milliSeconds)
    {
        d->notifyTimer->setInterval(milliSeconds);
        emit notifyIntervalChanged(milliSeconds);
    }
}

bool MediaObject::isMetadataAvailable() const
{
    Q_D(const MediaObject);
    return d->metadataControl ? d->metadataControl->isMetadataAvailable() : false;
}

QVariant MediaObject::metadata(const QString &key) const
{
    Q_D(const MediaObject);
    return d->metadataControl ? d->metadataControl->metadata(key) : QVariant();
}

void MediaObject::setMetadata(const QString &key, const QVariant &value)
{
    Q_D(MediaObject);
    if(d->metadataControl)
        d->metadataControl->setMetadata(key, value);
}

QStringList MediaObject::availableMetadata() const
{
    Q_D(const MediaObject);
    return d->metadataControl ? d->metadataControl->availableMetadata() : QStringList();
}

void MediaObject::notify()
{
    Q_D(MediaObject);
    const QMetaObject* mo = metaObject();
    QSet<int> properties = d->notifyProperties;
    for (int index : qAsConst(properties))
    {
        QMetaProperty mp = mo->property(index);
        // invoking propertyChanged signal
        mp.notifySignal().invoke(this, QGenericArgument(QMetaType::typeName(mp.userType()), mp.read(this).data()));
    }
}
