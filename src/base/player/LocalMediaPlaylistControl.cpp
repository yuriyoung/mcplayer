#include "LocalMediaPlaylistControl.h"
#include "LocalMediaPlaylistProvider.h"
#include "MediaPlaylistNavigator.h"

LocalMediaPlaylistControl::LocalMediaPlaylistControl(QObject *parent)
    : MediaPlaylistControl(parent)
{
    LocalMediaPlaylistProvider *playlist = new LocalMediaPlaylistProvider(this);
    m_navigator = new MediaPlaylistNavigator(playlist, this);
    m_navigator->setPlaybackMode(MediaPlaylist::Sequential);

    connect(m_navigator, &MediaPlaylistNavigator::currentIndexChanged,
            this, &MediaPlaylistControl::currentIndexChanged);
    connect(m_navigator, &MediaPlaylistNavigator::activated,
            this, &MediaPlaylistControl::currentMediaChanged);
    connect(m_navigator, &MediaPlaylistNavigator::playbackModeChanged,
           this, &MediaPlaylistControl::playbackModeChanged);
}

LocalMediaPlaylistControl::~LocalMediaPlaylistControl()
{
    m_navigator->deleteLater();
}

MediaPlaylistProvider *LocalMediaPlaylistControl::playlistProvider() const
{
    return m_navigator->playlist();
}

bool LocalMediaPlaylistControl::setPlaylistProvider(MediaPlaylistProvider *provider)
{
    m_navigator->setPlaylist(provider);
    emit playlistProviderChanged();
    return true;
}

int LocalMediaPlaylistControl::currentIndex() const
{
    return m_navigator->currentIndex();
}

void LocalMediaPlaylistControl::setCurrentIndex(int index)
{
    m_navigator->jump(index);
}

int LocalMediaPlaylistControl::nextIndex(int steps) const
{
    return m_navigator->nextIndex(steps);
}

int LocalMediaPlaylistControl::previousIndex(int steps) const
{
    return m_navigator->previousIndex(steps);
}

void LocalMediaPlaylistControl::next()
{
    m_navigator->next();
}

void LocalMediaPlaylistControl::previous()
{
    m_navigator->previous();
}

MediaPlaylist::PlaybackMode LocalMediaPlaylistControl::playbackMode() const
{
    return m_navigator->playbackMode();
}

void LocalMediaPlaylistControl::setPlaybackMode(MediaPlaylist::PlaybackMode mode)
{
    m_navigator->setPlaybackMode(mode);
}
