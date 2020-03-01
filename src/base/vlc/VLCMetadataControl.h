#ifndef VLCMETADATACONTROL_H
#define VLCMETADATACONTROL_H

#include "VLCEngine.h"
#include "player/MediaMetadataControl.h"
#include "player/Media.h"

#include <QVariant>

struct libvlc_media_t;

class VLCMetadataControlPrivate;
class VLCMetadataControl : public MediaMetadataControl
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(VLCMetadataControl)
public:
    explicit VLCMetadataControl(VLCEngine *engine, QObject *parent = nullptr);
    ~VLCMetadataControl();

    virtual bool isMetadataAvailable() const;
    virtual QStringList availableMetadata() const;

    virtual bool isWritable() const;

    virtual QVariant metadata(const QString &key) const;
    virtual void setMetadata(const QString &key, const QVariant &value);

    void parseMediaAsync(libvlc_media_t *media, int type);

signals:

public slots:

private:
    QScopedPointer<VLCMetadataControlPrivate> d_ptr;
};

#endif // VLCMETADATACONTROL_H
