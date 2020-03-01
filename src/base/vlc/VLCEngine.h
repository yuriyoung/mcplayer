#ifndef VLCSERVICE_H
#define VLCSERVICE_H

#include "player/MediaEngine.h"

#include <QObject>

struct libvlc_instance_t;
class VLCPlayerControl;
class VLCMetadataControl;

class VLCEnginePrivate;
class VLCEngine : public MediaEngine
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(VLCEngine)
public:
    explicit VLCEngine(QObject *parent = nullptr);
    ~VLCEngine();

    MediaControl *requestControl(const char *name);
    void releaseControl(MediaControl *control);

    libvlc_instance_t *vlcInstance() const;
    VLCPlayerControl *playerControl() const;
    VLCMetadataControl *metadataControl() const;

private:
    QScopedPointer<VLCEnginePrivate> d_ptr;
};

#endif // VLCSERVICE_H
