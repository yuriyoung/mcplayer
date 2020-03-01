#include "VLCEngineProvider.h"
#include "VLCEngine.h"

#include <QMapIterator>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcVLCEngineProvider, "mcplayer.VLCEngineProvider")

VLCEngineProvider::VLCEngineProvider(QObject *parent)
    : MediaEngineProvider(parent)
{

}

VLCEngineProvider::~VLCEngineProvider()
{
//    qDeleteAll(m_engines.begin(), m_engines.end());
    m_engines.clear();
}

MediaEngine *VLCEngineProvider::requestEngine(const QByteArray &type)
{
    QMapIterator<MediaEngine *, QByteArray> it(m_engines);
    while (it.hasNext())
    {
        it.next();
        if(it.value() == type)
            return it.key();
    }

    VLCEngine *engine = new VLCEngine;
    m_engines.insert(engine, type.isEmpty() ? QByteArray(MEDIAENGINE_MEDIAPLAYER) : type);
    return engine;
}

void VLCEngineProvider::releaseEngine(MediaEngine *engine)
{
    if(engine)
    {
        m_engines.remove(engine);
        delete engine;
    }
}
