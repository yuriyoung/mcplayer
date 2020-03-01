#ifndef MEDIAPLAYLISTPROVIDER_H
#define MEDIAPLAYLISTPROVIDER_H

#include "MediaPlaylist.h"
#include "Media.h"

#include <QObject>
#include <QNetworkRequest>
#include <QIODevice>

class MediaPlaylistProvider : public QObject
{
    Q_OBJECT
public:
    explicit MediaPlaylistProvider(QObject *parent = nullptr);
    virtual ~MediaPlaylistProvider();

    virtual bool load(const QNetworkRequest &request, const char *format = nullptr);
    virtual bool load(QIODevice * device, const char *format = nullptr);
    virtual bool save(const QUrl &location, const char *format = nullptr);
    virtual bool save(QIODevice * device, const char *format);

    virtual bool isReadOnly() const;

    virtual int mediaCount() const = 0;
    virtual Media media(int index) const = 0;

    virtual bool addMedia(const Media &media);
    virtual bool addMedia(const QList<Media> &mediaList);
    virtual bool insertMedia(int index, const Media &media);
    virtual bool insertMedia(int index, const QList<Media> &mediaList);
    virtual bool moveMedia(int from, int to);
    virtual bool removeMedia(int pos);
    virtual bool removeMedia(int start, int end);
    virtual bool clear();

signals:
    void mediaAboutToInserted(int start, int end);
    void mediaInserted(int start, int end);
    void mediaAboutToRemoved(int start, int end);
    void mediaRemoved(int start, int end);
    void mediaChanged(int start, int end);
    void loaded();

public slots:
    virtual void shuffle();
};

#endif // MEDIAPLAYLISTPROVIDER_H
