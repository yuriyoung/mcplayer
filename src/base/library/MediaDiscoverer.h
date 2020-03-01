#ifndef MEDIADISCOVERER_H
#define MEDIADISCOVERER_H

#include <QObject>

class MediaDiscovererPrivate;
class MediaDiscoverer : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(d, MediaDiscoverer)
public:
    explicit MediaDiscoverer(QObject *parent = nullptr);
    ~MediaDiscoverer();

    virtual void add(const QString &entryPoint);
    virtual void remove(const QString &entryPoint);

    virtual void reload();
    virtual void reload(const QString &entryPoint);

    virtual void ban(const QString &entryPoint);
    virtual void unban(const QString &entryPoint);

    virtual bool discover(const QString &path);
    virtual void start();
    virtual void stop();

signals:
    void started();
    void canceled();
    void finished();

    void trackDiscovered();
    void artistDiscovered();
    void albumDiscovered();
    void genreDiscovered();
    void playlistDiscovered();

private:
    QScopedPointer<MediaDiscovererPrivate> d;
};

#endif // MEDIADISCOVERER_H
