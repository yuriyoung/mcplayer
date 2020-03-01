#ifndef MEDIAOBJECT_H
#define MEDIAOBJECT_H

#include <QObject>
#include <QTimer>
#include <QSet>

class MediaEngine;
class MediaMetadataControl;
class MediaObjectPrivate;

class MediaObject : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MediaObject)
public:
    virtual ~MediaObject();

    virtual bool isAvailable() const;
    virtual MediaEngine *engine() const;

    virtual bool bind(QObject *bindable);
    virtual void unbind(QObject *bindable);

    int notifyInterval() const;
    void setNotifyInterval(int milliSeconds);

    bool isMetadataAvailable() const;

    QVariant metadata(const QString &key) const;
    void setMetadata(const QString &key, const QVariant &value);
    QStringList availableMetadata() const;

signals:
    void notifyIntervalChanged(int milliSeconds);

    void metadataChanged();
    void metadataChanged(const QString &key, const QVariant &value);
    void metadataAvailableChanged(bool available);
    void writableChanged(bool writable);

private slots:
    void notify();

protected:
    explicit MediaObject(QObject *parent, MediaEngine *engine);
    explicit MediaObject(MediaObjectPrivate &dd, MediaEngine *engine, QObject *parent);

    void watchProperty(const QByteArray &name);
    void unwatchProperty(const QByteArray &name);

protected:
    QScopedPointer<MediaObjectPrivate> d_ptr;
};

#endif // MEDIAOBJECT_H
