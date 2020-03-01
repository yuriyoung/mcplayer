#ifndef MEDIA_H
#define MEDIA_H

#include "MediaResource.h"

#include <QObject>
#include <QUrl>
#include <QSharedDataPointer>

class MediaPlaylist;
class MediaPrivate;

/**
 * @brief The Media class provide mediadata or properties
 * eg:
 * metadata / thumbnail / chapter / tracks / favorite / play count
 *
 */
class Media
{
public:
    // do not use explicit
    Media();
    Media(const QUrl &location);
    Media(const MediaResource &resource);
    Media(const QNetworkRequest &request);
    Media(const MediaResourceList &resources);
    Media(MediaPlaylist *playlist, const QUrl &contentUrl = QUrl(), bool takeOwnership = false);
    Media(const Media &other);
    ~Media();

    Media &operator=(const Media &other);
    bool operator==(const Media &other) const;
    bool operator!=(const Media &other) const;

    bool isNull() const;
    QUrl canonicalUrl() const;
    QNetworkRequest canonicalRequest() const;
    MediaResource canonicalResource() const;
    MediaResourceList resources() const;
    MediaPlaylist *playlist() const;

private:
    QSharedDataPointer<MediaPrivate> d;
};

Q_DECLARE_METATYPE(Media)

#endif // MEDIA_H
