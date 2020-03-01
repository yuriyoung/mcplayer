#include "MediaEngineProvider.h"

MediaEngineProvider::MediaEngineProvider(QObject *parent) : QObject(parent)
{

}

int MediaEngineProvider::hasSupport(const QByteArray &engineType, const QString &mimeType, const QStringList &codecs) const
{
    Q_UNUSED(engineType)
    Q_UNUSED(mimeType)
    Q_UNUSED(codecs)

    return -1;
}

int MediaEngineProvider::supportedFeatures(const MediaEngine *engine) const
{
    Q_UNUSED(engine)
    return -1;
}

QStringList MediaEngineProvider::supportedMimeTypes(const QByteArray &type, int flags) const
{
    Q_UNUSED(type)
    Q_UNUSED(flags)
    QStringList supportedTypes;

    return supportedTypes;
}
