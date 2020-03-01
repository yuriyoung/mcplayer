#ifndef QMLMEDIAMETADATA_H
#define QMLMEDIAMETADATA_H

#include "player/MediaObject.h"
#include "Metadata.h"

#include <QObject>
#include <QtQml>

#define QML_PROPERTY(name) \
    Q_PROPERTY( \
        QVariant name \
        READ get ##name \
        WRITE set ##name \
        NOTIFY metadataChanged \
    )

#define METADATA_MEMBER(name) \
    inline void set ##name(const QVariant &value) \
    { \
        setMetadata(Metadata::name, value); \
    } \
    inline QVariant get ##name () const \
    { \
        return metadata(Metadata::name ); \
    }

class QmlMediaMetadata : public QObject
{
    Q_OBJECT

    // Common
    QML_PROPERTY(Title)
    QML_PROPERTY(SubTitle)
    QML_PROPERTY(Author)
    QML_PROPERTY(Comment)
    QML_PROPERTY(Description)
    QML_PROPERTY(Category)
    QML_PROPERTY(Genre)
    QML_PROPERTY(Year)
    QML_PROPERTY(Date)
    QML_PROPERTY(UserRating)
    QML_PROPERTY(Keywords)
    QML_PROPERTY(Language)
    QML_PROPERTY(Publisher)
    QML_PROPERTY(Copyright)
    QML_PROPERTY(ParentalRating)
    QML_PROPERTY(RatingOrganization)

    // Common extra
    QML_PROPERTY(URL)
    QML_PROPERTY(NowPlaying)
    QML_PROPERTY(EncodedBy)

    // Media
    QML_PROPERTY(Size)
    QML_PROPERTY(MediaType)
    QML_PROPERTY(Duration)

    // Audio
    QML_PROPERTY(AudioBitRate)
    QML_PROPERTY(AudioCodec)
    QML_PROPERTY(AverageLevel)
    QML_PROPERTY(ChannelCount)
    QML_PROPERTY(PeakValue)
    QML_PROPERTY(SampleRate)

    // Music
    QML_PROPERTY(AlbumTitle)
    QML_PROPERTY(AlbumArtist)
    QML_PROPERTY(ContributingArtist)
    QML_PROPERTY(Composer)
    QML_PROPERTY(Conductor)
    QML_PROPERTY(Lyrics)
    QML_PROPERTY(Mood)
    QML_PROPERTY(TrackNumber)
    QML_PROPERTY(TrackCount)
    QML_PROPERTY(TrackId) // extra
    QML_PROPERTY(CoverArtUrlSmall)
    QML_PROPERTY(CoverArtUrlLarge)

    // Image/Video
    QML_PROPERTY(Resolution)
    QML_PROPERTY(PixelAspectRatio)

    // Video
    QML_PROPERTY(VideoFrameRate)
    QML_PROPERTY(VideoBitRate)
    QML_PROPERTY(VideoCodec)
    QML_PROPERTY(PosterUrl)

    // Movie
    QML_PROPERTY(ChapterNumber)
    QML_PROPERTY(Director)
    QML_PROPERTY(LeadPerformer)
    QML_PROPERTY(Writer)

    QML_PROPERTY(Season)
    QML_PROPERTY(Episode)
    QML_PROPERTY(ShowName)
    QML_PROPERTY(Actors)
    QML_PROPERTY(DiscNumber)
    QML_PROPERTY(DiscTotal)

    // TODO: add other exrta property
    // ...

public:
    explicit QmlMediaMetadata(MediaObject *player, QObject *parent = nullptr);

    // Common
    METADATA_MEMBER(Title)
    METADATA_MEMBER(SubTitle)
    METADATA_MEMBER(Author)
    METADATA_MEMBER(Comment)
    METADATA_MEMBER(Description)
    METADATA_MEMBER(Category)
    METADATA_MEMBER(Genre)
    METADATA_MEMBER(Year)
    METADATA_MEMBER(Date)
    METADATA_MEMBER(UserRating)
    METADATA_MEMBER(Keywords)
    METADATA_MEMBER(Language)
    METADATA_MEMBER(Publisher)
    METADATA_MEMBER(Copyright)
    METADATA_MEMBER(ParentalRating)
    METADATA_MEMBER(RatingOrganization)

    METADATA_MEMBER(URL)
    METADATA_MEMBER(NowPlaying)
    METADATA_MEMBER(EncodedBy)

    // Media
    METADATA_MEMBER(Size)
    METADATA_MEMBER(MediaType)
    METADATA_MEMBER(Duration)

    // Audio
    METADATA_MEMBER(AudioBitRate)
    METADATA_MEMBER(AudioCodec)
    METADATA_MEMBER(AverageLevel)
    METADATA_MEMBER(ChannelCount)
    METADATA_MEMBER(PeakValue)
    METADATA_MEMBER(SampleRate)

    // Music
    METADATA_MEMBER(AlbumTitle)
    METADATA_MEMBER(AlbumArtist)
    METADATA_MEMBER(ContributingArtist)
    METADATA_MEMBER(Composer)
    METADATA_MEMBER(Conductor)
    METADATA_MEMBER(Lyrics)
    METADATA_MEMBER(Mood)
    METADATA_MEMBER(TrackNumber)
    METADATA_MEMBER(TrackCount)
    METADATA_MEMBER(TrackId) // extra
    METADATA_MEMBER(CoverArtUrlSmall)
    METADATA_MEMBER(CoverArtUrlLarge)

    // Image/Video
    METADATA_MEMBER(Resolution)
    METADATA_MEMBER(PixelAspectRatio)

    // Video
    METADATA_MEMBER(VideoFrameRate)
    METADATA_MEMBER(VideoBitRate)
    METADATA_MEMBER(VideoCodec)
    METADATA_MEMBER(PosterUrl)

    // Movie
    METADATA_MEMBER(ChapterNumber)
    METADATA_MEMBER(Director)
    METADATA_MEMBER(LeadPerformer)
    METADATA_MEMBER(Writer)

    METADATA_MEMBER(Season)
    METADATA_MEMBER(Episode)
    METADATA_MEMBER(ShowName)
    METADATA_MEMBER(Actors)
    METADATA_MEMBER(DiscNumber)
    METADATA_MEMBER(DiscTotal)

    // TODO: add extra meta data
    // ....

signals:
    void metadataChanged();

private:
    QVariant metadata(const QString &key) const;
    void setMetadata(const QString &key, const QVariant &value);

private:
    MediaObject *m_mediaObject = nullptr;
};

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE(QmlMediaMetadata))

#endif // QMLMEDIAMETADATA_H
