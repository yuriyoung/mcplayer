#include "QmlMediaMetadata.h"

QmlMediaMetadata::QmlMediaMetadata(MediaObject *player, QObject *parent)
    : QObject(parent), m_mediaObject(player)
{

}

QVariant QmlMediaMetadata::metadata(const QString &key) const
{
    return m_mediaObject->metadata(key);
}

void QmlMediaMetadata::setMetadata(const QString &key, const QVariant &value)
{
    m_mediaObject->setMetadata(key, value);
}
