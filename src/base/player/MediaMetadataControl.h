#ifndef MEDIAMETADATACONTROL_H
#define MEDIAMETADATACONTROL_H

#include "MediaControl.h"

class MediaMetadataControl : public MediaControl
{
    Q_OBJECT
public:
    virtual bool isMetadataAvailable() const = 0;
    virtual QStringList availableMetadata() const = 0;

    virtual bool isWritable() const = 0;

    virtual QVariant metadata(const QString &key) const = 0;
    virtual void setMetadata(const QString &key, const QVariant &value) = 0;

signals:
    void metadataChanged();
    void metadataChanged(const QString &key, const QVariant &value);
    void metadataAvailableChanged(bool available);
    void writableChanged(bool writable);

protected:
    explicit MediaMetadataControl(QObject *parent = nullptr);
};

#define MediaMetadataControl_iid "org.mcplayer.mediametadatacontrol/1.0"
MEDIA_DECLARE_CONTROL(MediaMetadataControl, MediaMetadataControl_iid)

#endif // MEDIAMETADATACONTROL_H
