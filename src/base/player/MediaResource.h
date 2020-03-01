#ifndef MEDIARESOURCE_H
#define MEDIARESOURCE_H

#include <QObject>
#include <QSharedDataPointer>
#include <QNetworkRequest>
#include <QUrl>

class MediaResourcePrivate;
class MediaResource
{
public:
    MediaResource();
    MediaResource(const QUrl &url, const QString &mimeType = QString());
    MediaResource(const QNetworkRequest &request, const QString &mimeType = QString());
    MediaResource(const MediaResource &other);
    MediaResource &operator =(const MediaResource &other);
    ~MediaResource();

    bool operator ==(const MediaResource &other) const;
    bool operator !=(const MediaResource &other) const;

    bool isNull() const;

    QUrl url() const;
    QString mimeType() const;

    QNetworkRequest request() const;

    QString language() const;
    void setLanguage(const QString &language);

    QString audioCodec() const;
    void setAudioCodec(const QString &codec);

    qint64 dataSize() const;
    void setDataSize(const qint64 size);

    int audioBitRate() const;
    void setAudioBitRate(int rate);

    int sampleRate() const;
    void setSampleRate(int sampleRate);

    int channelCount() const;
    void setChannelCount(int channels);

protected:
    QSharedDataPointer<MediaResourcePrivate> d;
};

typedef QList<MediaResource> MediaResourceList;

Q_DECLARE_METATYPE(MediaResource)
Q_DECLARE_METATYPE(MediaResourceList)

#endif // MEDIARESOURCE_H
