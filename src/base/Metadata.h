#ifndef MEDIAMETADATA_H
#define MEDIAMETADATA_H

#include <QString>

#define DECLARE_METADATA(key) extern const QString key

namespace Metadata
{
    // Common
    DECLARE_METADATA(Title);
    DECLARE_METADATA(SubTitle);
    DECLARE_METADATA(Author);
    DECLARE_METADATA(Comment);
    DECLARE_METADATA(Description);
    DECLARE_METADATA(Category);
    DECLARE_METADATA(Genre);
    DECLARE_METADATA(Year);
    DECLARE_METADATA(Date);
    DECLARE_METADATA(UserRating);
    DECLARE_METADATA(Keywords);
    DECLARE_METADATA(Language);
    DECLARE_METADATA(Publisher);
    DECLARE_METADATA(Copyright);
    DECLARE_METADATA(ParentalRating);
    DECLARE_METADATA(RatingOrganization);

    DECLARE_METADATA(URL);
    DECLARE_METADATA(NowPlaying);
    DECLARE_METADATA(EncodedBy);

    // Media
    DECLARE_METADATA(Size);
    DECLARE_METADATA(MediaType);
    DECLARE_METADATA(Duration);

    // Audio
    DECLARE_METADATA(AudioBitRate);
    DECLARE_METADATA(AudioCodec);
    DECLARE_METADATA(AverageLevel);
    DECLARE_METADATA(ChannelCount);
    DECLARE_METADATA(PeakValue);
    DECLARE_METADATA(SampleRate);

    // Music
    DECLARE_METADATA(AlbumTitle);
    DECLARE_METADATA(AlbumArtist);
    DECLARE_METADATA(ContributingArtist);
    DECLARE_METADATA(Composer);
    DECLARE_METADATA(Conductor);
    DECLARE_METADATA(Lyrics);
    DECLARE_METADATA(Mood);
    DECLARE_METADATA(TrackNumber);
    DECLARE_METADATA(TrackCount);
    DECLARE_METADATA(TrackId); // extra
    DECLARE_METADATA(CoverArtUrlSmall);
    DECLARE_METADATA(CoverArtUrlLarge);

    // Image/Video
    DECLARE_METADATA(Resolution);
    DECLARE_METADATA(PixelAspectRatio);

    // Video
    DECLARE_METADATA(VideoFrameRate);
    DECLARE_METADATA(VideoBitRate);
    DECLARE_METADATA(VideoCodec);
    DECLARE_METADATA(PosterUrl);

    // Movie
    DECLARE_METADATA(ChapterNumber);
    DECLARE_METADATA(Director);
    DECLARE_METADATA(LeadPerformer);
    DECLARE_METADATA(Writer);

    DECLARE_METADATA(Season);
    DECLARE_METADATA(Episode);
    DECLARE_METADATA(ShowName);
    DECLARE_METADATA(Actors);
    DECLARE_METADATA(DiscNumber);
    DECLARE_METADATA(DiscTotal);
}

#undef DECLARE_METADATA
#endif // MEDIAMETADATA_H
