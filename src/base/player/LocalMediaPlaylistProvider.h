#ifndef LOCALMEDIAPLAYLISTPROVIDER_H
#define LOCALMEDIAPLAYLISTPROVIDER_H

#include "MediaPlaylistProvider.h"

#include <QObject>
#include <QNetworkRequest>
#include <QIODevice>

class LocalMediaPlaylistProviderPrivate;
class LocalMediaPlaylistProvider : public MediaPlaylistProvider
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LocalMediaPlaylistProvider)
public:
    explicit LocalMediaPlaylistProvider(QObject *parent = nullptr);
    ~LocalMediaPlaylistProvider();

    virtual bool load(QIODevice * device, const char *format = nullptr);
    virtual bool save(QIODevice * device, const char *format);

    virtual bool isReadOnly() const;

    virtual int mediaCount() const;
    virtual Media media(int index) const;

    virtual bool addMedia(const Media &media);
    virtual bool addMedia(const QList<Media> &mediaList);
    virtual bool insertMedia(int index, const Media &media);
    virtual bool insertMedia(int index, const QList<Media> &mediaList);
    virtual bool moveMedia(int from, int to);
    virtual bool removeMedia(int pos);
    virtual bool removeMedia(int start, int end);
    virtual bool clear();

public slots:
    virtual void shuffle();

private:
    QScopedPointer<LocalMediaPlaylistProviderPrivate> d_ptr;
};

#endif // LOCALMEDIAPLAYLISTPROVIDER_H
