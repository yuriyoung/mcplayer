#include "MediaResource.h"

#include <QSharedData>
#include <QMap>
#include <QVariant>

static void registerMediaResourceMetaTypes()
{
    qRegisterMetaType<MediaResource>();
}
Q_CONSTRUCTOR_FUNCTION(registerMediaResourceMetaTypes)

class MediaResourcePrivate : public QSharedData
{
public:
    MediaResourcePrivate() {}
    ~MediaResourcePrivate() {}

    enum Property
    {
        Url,
        Request,
        MimeType,
        Language,
        AudioCodec,
        VideoCodec,
        DataSize,
        AudioBitRate,
        VideoBitRate,
        SampleRate,
        ChannelCount,
        Resolution
    };
    QMap<int, QVariant> values;
};

MediaResource::MediaResource()
{
    // A null media resource
}

MediaResource::MediaResource(const QUrl &url, const QString &mimeType)
    : d(new MediaResourcePrivate)
{
    d->values.insert(MediaResourcePrivate::Url, url);
    d->values.insert(MediaResourcePrivate::MimeType, mimeType);
}

MediaResource::MediaResource(const QNetworkRequest &request, const QString &mimeType)
    : d(new MediaResourcePrivate)
{
    d->values.insert(MediaResourcePrivate::Request, QVariant::fromValue(request));
    d->values.insert(MediaResourcePrivate::Url, request.url());
    d->values.insert(MediaResourcePrivate::MimeType, mimeType);
}

MediaResource::MediaResource(const MediaResource &other)
    : d(other.d)
{

}

MediaResource &MediaResource::operator =(const MediaResource &other)
{
    d = other.d;
    return *this;
}

MediaResource::~MediaResource()
{

}

bool MediaResource::operator ==(const MediaResource &other) const
{
    for (auto it = d->values.cbegin(), end = d->values.cend(); it != end; ++it)
    {
        switch (it.key())
        {
        case MediaResourcePrivate::Request:
            if (request() != other.request())
                return false;
        break;
        default:
            if (it.value() != other.d->values.value(it.key()))
                return false;
        }
    }
    return true;
}

bool MediaResource::operator !=(const MediaResource &other) const
{
    return !(*this == other);
}

bool MediaResource::isNull() const
{
    return d.constData() == nullptr;
}

QUrl MediaResource::url() const
{
    return d.constData() != nullptr
            ? qvariant_cast<QUrl>(d->values.value(MediaResourcePrivate::Url))
            : QUrl();
}

QString MediaResource::mimeType() const
{
    return d.constData() != nullptr
            ? qvariant_cast<QString>(d->values.value(MediaResourcePrivate::MimeType))
            : QString();
}

QNetworkRequest MediaResource::request() const
{
    if(isNull())
        return QNetworkRequest();

    auto R = MediaResourcePrivate::Request;
    if(d->values.contains(R))
        return qvariant_cast<QNetworkRequest>(d->values.value(R));

    return QNetworkRequest(url());
}

QString MediaResource::language() const
{
    return d.constData() != nullptr
            ? qvariant_cast<QString>(d->values.value(MediaResourcePrivate::Language))
            : QString();
}

void MediaResource::setLanguage(const QString &language)
{
    if(isNull())
        return;

    if(language.isNull())
        d->values.remove(MediaResourcePrivate::Language);
    else
        d->values.insert(MediaResourcePrivate::Language, language);
}

QString MediaResource::audioCodec() const
{
    return d.constData() != nullptr
            ? qvariant_cast<QString>(d->values.value(MediaResourcePrivate::AudioCodec))
            : QString();
}

void MediaResource::setAudioCodec(const QString &codec)
{
    if(isNull())
        return;

    if(codec.isNull())
        d->values.remove(MediaResourcePrivate::AudioCodec);
    else
        d->values.insert(MediaResourcePrivate::AudioCodec, codec);

}

qint64 MediaResource::dataSize() const
{
    return d.constData() != nullptr
            ? qvariant_cast<qint64>(d->values.value(MediaResourcePrivate::DataSize))
            : 0;
}

void MediaResource::setDataSize(const qint64 size)
{
    if(isNull())
        return;

    if(size == 0)
        d->values.remove(MediaResourcePrivate::DataSize);
    else
        d->values.insert(MediaResourcePrivate::DataSize, size);
}

int MediaResource::audioBitRate() const
{
    return d.constData() != nullptr
            ? qvariant_cast<int>(d->values.value(MediaResourcePrivate::AudioBitRate))
            : 0;
}

void MediaResource::setAudioBitRate(int rate)
{
    if(isNull())
        return;

    if(rate == 0)
        d->values.remove(MediaResourcePrivate::AudioBitRate);
    else
        d->values.insert(MediaResourcePrivate::AudioBitRate, rate);
}

int MediaResource::sampleRate() const
{
    return d.constData() != nullptr
            ? qvariant_cast<int>(d->values.value(MediaResourcePrivate::SampleRate))
            : 0;
}

void MediaResource::setSampleRate(int sampleRate)
{
    if(isNull())
        return;

    if(sampleRate == 0)
        d->values.remove(MediaResourcePrivate::SampleRate);
    else
        d->values.insert(MediaResourcePrivate::SampleRate, sampleRate);
}

int MediaResource::channelCount() const
{
    return d.constData() != nullptr
            ? qvariant_cast<int>(d->values.value(MediaResourcePrivate::ChannelCount))
            : 0;
}

void MediaResource::setChannelCount(int channels)
{
    if(isNull())
        return;

    if(channels == 0)
        d->values.remove(MediaResourcePrivate::ChannelCount);
    else
        d->values.insert(MediaResourcePrivate::ChannelCount, channels);
}
