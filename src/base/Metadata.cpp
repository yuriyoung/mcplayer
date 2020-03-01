#include "Metadata.h"

#if defined(Q_CC_GNU) && defined(Q_COMPILER_LAMBDA)
#    define DEFINE_METADATA(key) const QString Metadata::key(QStringLiteral(#key))
#else
#    define DEFINE_METADATA(key) const QString Metadata::key(QLatin1String(#key))
#endif

// Common
DEFINE_METADATA(Title);
DEFINE_METADATA(SubTitle);
DEFINE_METADATA(Author);
DEFINE_METADATA(Comment);
DEFINE_METADATA(Description);
DEFINE_METADATA(Category);
DEFINE_METADATA(Genre);
DEFINE_METADATA(Year);
DEFINE_METADATA(Date);
DEFINE_METADATA(UserRating);
DEFINE_METADATA(Keywords);
DEFINE_METADATA(Language);
DEFINE_METADATA(Publisher);
DEFINE_METADATA(Copyright);
DEFINE_METADATA(ParentalRating);
DEFINE_METADATA(RatingOrganization);
DEFINE_METADATA(URL);
DEFINE_METADATA(NowPlaying);
DEFINE_METADATA(EncodedBy);

// Media
DEFINE_METADATA(Size);
DEFINE_METADATA(MediaType);
DEFINE_METADATA(Duration);

// Audio
DEFINE_METADATA(AudioBitRate);
DEFINE_METADATA(AudioCodec);
DEFINE_METADATA(AverageLevel);
DEFINE_METADATA(ChannelCount);
DEFINE_METADATA(PeakValue);
DEFINE_METADATA(SampleRate);

// Music
DEFINE_METADATA(AlbumTitle);
DEFINE_METADATA(AlbumArtist);
DEFINE_METADATA(ContributingArtist);
DEFINE_METADATA(Composer);
DEFINE_METADATA(Conductor);
DEFINE_METADATA(Lyrics);
DEFINE_METADATA(Mood);
DEFINE_METADATA(TrackNumber);
DEFINE_METADATA(TrackCount);
DEFINE_METADATA(TrackId); // extra
DEFINE_METADATA(CoverArtUrlSmall);
DEFINE_METADATA(CoverArtUrlLarge);

// Image/Video
DEFINE_METADATA(Resolution);
DEFINE_METADATA(PixelAspectRatio);

// Video
DEFINE_METADATA(VideoFrameRate);
DEFINE_METADATA(VideoBitRate);
DEFINE_METADATA(VideoCodec);
DEFINE_METADATA(PosterUrl);

// Movie
DEFINE_METADATA(ChapterNumber);
DEFINE_METADATA(Director);
DEFINE_METADATA(LeadPerformer);
DEFINE_METADATA(Writer);
DEFINE_METADATA(Season);
DEFINE_METADATA(Episode);
DEFINE_METADATA(ShowName);
DEFINE_METADATA(Actors);
DEFINE_METADATA(DiscNumber);
DEFINE_METADATA(DiscTotal);
