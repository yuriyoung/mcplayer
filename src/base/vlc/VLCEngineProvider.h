#ifndef VLCSERVICEPROVIDER_H
#define VLCSERVICEPROVIDER_H

#include "player/MediaEngineProvider.h"

#include <QObject>
#include <QMap>

struct libvlc_instance_t;

class VLCEngineProvider : public MediaEngineProvider
{
    Q_OBJECT
public:
    explicit VLCEngineProvider(QObject *parent = nullptr);
    ~VLCEngineProvider();

    virtual MediaEngine *requestEngine(const QByteArray &type);
    virtual void releaseEngine(MediaEngine *engine);

    struct EngineData
    {
        QByteArray type;
        libvlc_instance_t *instance = nullptr;
    };

private:
//    QMap<MediaEngine *, EngineData> m_engines;
    QMap<MediaEngine *, QByteArray> m_engines;
};

#endif // VLCSERVICEPROVIDER_H
