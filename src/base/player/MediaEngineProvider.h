#ifndef MEDIAENGINEPROVIDER_H
#define MEDIAENGINEPROVIDER_H

#include "MediaEngine.h"
#include <QObject>

#define MEDIAENGINE_MEDIAPLAYER "org.mcplayer.mediaplayer"

class MediaEngineProvider : public QObject
{
    Q_OBJECT
public:
    enum Features {
        LowLatencyPlayback = 0x01,
        StreamPlayback = 0x02,
        VideoSurface = 0x04
    };

    explicit MediaEngineProvider(QObject *parent = nullptr);

    virtual MediaEngine *requestEngine(const QByteArray &type) = 0;
    virtual void releaseEngine(MediaEngine *engine) = 0;

    virtual int hasSupport(const QByteArray &engineType, const QString &mimeType, const QStringList& codecs) const;
    virtual int supportedFeatures(const MediaEngine *engine) const;
    virtual QStringList supportedMimeTypes(const QByteArray &type, int flags = 0) const;
};

#endif // MEDIAENGINEPROVIDER_H
