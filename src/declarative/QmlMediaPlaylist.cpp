#include "QmlMediaPlaylist.h"

#include <QQmlListProperty>
#include <QFileInfo>

QmlMediaItem::QmlMediaItem(QObject *parent) : QObject(parent)
{

}

QUrl QmlMediaItem::source() const
{
    return m_source;
}

void QmlMediaItem::setSource(QUrl source)
{
    m_source = source;
}

/**
 * @brief QmlMediaPlaylist::QmlMediaPlaylist
 * @param parent
 */
QmlMediaPlaylist::QmlMediaPlaylist(QObject *parent)
    : QAbstractListModel(parent)
    , m_error(MediaPlaylist::NoError)
    , m_readOnly(false)
{

}

QmlMediaPlaylist::~QmlMediaPlaylist()
{

}

void QmlMediaPlaylist::classBegin()
{
    m_playlist.reset(new MediaPlaylist());

    // connect signals-signals
    connect(m_playlist.data(), &MediaPlaylist::currentIndexChanged, this, &QmlMediaPlaylist::currentIndexChanged);
    connect(m_playlist.data(), &MediaPlaylist::playbackModeChanged, this, &QmlMediaPlaylist::playbackModeChanged);
    connect(m_playlist.data(), &MediaPlaylist::currentMediaChanged, this, &QmlMediaPlaylist::currentSourceChanged);
    connect(m_playlist.data(), &MediaPlaylist::loadSucceed, this, &QmlMediaPlaylist::loadSucceed);

    // connect signals-slots
    connect(m_playlist.data(), &MediaPlaylist::mediaAboutToInserted, this, &QmlMediaPlaylist::beginInsertItems);
    connect(m_playlist.data(), &MediaPlaylist::mediaInserted, this, &QmlMediaPlaylist::endInsertItems);
    connect(m_playlist.data(), &MediaPlaylist::mediaAboutToRemoved, this, &QmlMediaPlaylist::beginRemoveItems);
    connect(m_playlist.data(), &MediaPlaylist::mediaRemoved, this, &QmlMediaPlaylist::endRemoveItems);
    connect(m_playlist.data(), &MediaPlaylist::mediaChanged, this, &QmlMediaPlaylist::changeItems);
    connect(m_playlist.data(), &MediaPlaylist::loadFailed, this, &QmlMediaPlaylist::playlistLoadFailed);

    if(m_playlist->isReadOnly())
    {
        m_readOnly = true;
        emit readOnlyChanged();
    }
}

void QmlMediaPlaylist::componentComplete()
{
    // do nothing
}

int QmlMediaPlaylist::rowCount(const QModelIndex &parent) const
{
    return m_playlist && !parent.isValid() ? m_playlist->mediaCount() : 0;
}

int QmlMediaPlaylist::columnCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? ColumnsRole : 0;
}

QModelIndex QmlMediaPlaylist::index(int row, int column, const QModelIndex &parent) const
{
    if(!m_playlist || parent.isValid())
        return QModelIndex();

    if(row >= 0 && row < m_playlist->mediaCount() && column >= 0 && column < ColumnsRole)
        return createIndex(row, column);

    return QModelIndex();
}

QVariant QmlMediaPlaylist::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role)
    if (index.isValid() /*&& role == Qt::DisplayRole*/)
    {
        QVariant value = m_data[index];
        if (!value.isValid())
        {
            QUrl location = m_playlist->media(index.row()).canonicalUrl();
            return role == TitleRole ? QFileInfo(location.path()).fileName() : location;
        }

        return value;
    }
    return QVariant();
}

bool QmlMediaPlaylist::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role)
    m_data[index] = value;
    emit dataChanged(index, index);
    return true;
}

QHash<int, QByteArray> QmlMediaPlaylist::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames[TitleRole]    = "title";
    roleNames[ArtistRole]   = "artist";
    roleNames[DurationRole] = "duration";
    roleNames[SourceRole]   = "source";
    return roleNames;
}

QmlMediaPlaylist::PlaybackMode QmlMediaPlaylist::playbackMode() const
{
    return PlaybackMode(m_playlist->playbackMode());
}

void QmlMediaPlaylist::setPlaybackMode(QmlMediaPlaylist::PlaybackMode mode)
{
    if(playbackMode() == mode)
        return;

    m_playlist->setPlaybackMode(MediaPlaylist::PlaybackMode(mode));
}

int QmlMediaPlaylist::currentIndex() const
{
    return m_playlist->currentIndex();
}

void QmlMediaPlaylist::setCurrentIndex(int index)
{
    if(currentIndex() == index)
        return;

    m_playlist->setCurrentIndex(index);
}

MediaPlaylist *QmlMediaPlaylist::mediaPlaylist() const
{
    return m_playlist.get();
}

int QmlMediaPlaylist::itemCount() const
{
    return m_playlist->mediaCount();
}

bool QmlMediaPlaylist::readOnly() const
{
    return m_readOnly;
}

QmlMediaPlaylist::Error QmlMediaPlaylist::error() const
{
    return Error(m_error);
}

QString QmlMediaPlaylist::errorString() const
{
    return m_errorString;
}

QUrl QmlMediaPlaylist::currentSource() const
{
    return m_playlist->currentMedia().canonicalUrl();
}

QQmlListProperty<QmlMediaItem> QmlMediaPlaylist::items()
{
    return QQmlListProperty<QmlMediaItem>(this, this,
        &QmlMediaPlaylist::item_append,
        &QmlMediaPlaylist::item_count,
        nullptr, // item at
        &QmlMediaPlaylist::item_clear
    );
}

QUrl QmlMediaPlaylist::source(int index)
{
    return m_playlist->media(index).canonicalUrl();
}

int QmlMediaPlaylist::nextIndex(int steps)
{
    return m_playlist->nextIndex(steps);
}

int QmlMediaPlaylist::previousIndex(int steps)
{
    return m_playlist->previousIndex(steps);
}

void QmlMediaPlaylist::next()
{
    m_playlist->next();
}

void QmlMediaPlaylist::previous()
{
    m_playlist->previous();
}

void QmlMediaPlaylist::shuffle()
{
    m_playlist->shuffle();
}

void QmlMediaPlaylist::load(const QUrl &location, const QString &format)
{
    m_error = MediaPlaylist::NoError;
    m_errorString = QString();
    emit errorChanged();
    m_playlist->load(location, format.toLatin1().constData());
}

bool QmlMediaPlaylist::save(const QUrl &location, const QString &format)
{
    return m_playlist->save(location, format.toLatin1().constData());
}

bool QmlMediaPlaylist::addItem(const QUrl &source)
{
    return m_playlist->addMedia(Media(source));
}

bool QmlMediaPlaylist::addItems(const QList<QUrl> &sources)
{
    if(sources.isEmpty())
        return false;

    QList<Media> contents;
    QList<QUrl>::const_iterator it = sources.constBegin();
    while (it != sources.constEnd())
    {
        contents.push_back(Media(*it));
        ++it;
    }

    return m_playlist->addMedia(contents);
}

bool QmlMediaPlaylist::insertItem(int index, const QUrl &source)
{
    return m_playlist->insertMedia(index, Media(source));
}

bool QmlMediaPlaylist::insertItems(int index, const QList<QUrl> &sources)
{
    if (sources.empty())
            return false;

    QList<Media> contents;
    QList<QUrl>::const_iterator it = sources.constBegin();
    while (it != sources.constEnd())
    {
        contents.push_back(Media(*it));
        ++it;
    }

    return m_playlist->insertMedia(index, contents);
}

bool QmlMediaPlaylist::moveItem(int from, int to)
{
    return m_playlist->moveMedia(from, to);
}

bool QmlMediaPlaylist::removeItem(int index)
{
    return m_playlist->removeMedia(index);
}

bool QmlMediaPlaylist::removeItems(int start, int end)
{
    return m_playlist->removeMedia(start, end);
}

bool QmlMediaPlaylist::clear()
{
    beginResetModel();

    m_data.clear();
    bool ok = m_playlist->clear();

    endResetModel();

    return ok;
}

void QmlMediaPlaylist::beginInsertItems(int start, int end)
{
    emit itemAboutInsert(start, end);

    m_data.clear();
    beginInsertRows(QModelIndex(), start, end);
}

void QmlMediaPlaylist::endInsertItems(int start, int end)
{
    endInsertRows();

    emit itemCountChanged();
    emit itemInserted(start, end);
}

void QmlMediaPlaylist::beginRemoveItems(int start, int end)
{
    emit itemAboutRemove(start, end);

    m_data.clear();
    beginRemoveRows(QModelIndex(), start, end);
}

void QmlMediaPlaylist::endRemoveItems(int start, int end)
{
    endInsertRows();

    emit itemCountChanged();
    emit itemRemoved(start, end);
}

void QmlMediaPlaylist::changeItems(int start, int end)
{
    m_data.clear();
    emit dataChanged(index(start,0), index(end, ColumnsRole));
    emit itemChanged(start, end);
}

void QmlMediaPlaylist::playlistLoadFailed()
{
    m_error = m_playlist->error();
    m_errorString = m_playlist->errorString();

    emit errorChanged();
    emit loadFailed();
}

void QmlMediaPlaylist::item_append(QQmlListProperty<QmlMediaItem> *list, QmlMediaItem *item)
{
    static_cast<QmlMediaPlaylist*>(list->object)->addItem(item->source());
}

int QmlMediaPlaylist::item_count(QQmlListProperty<QmlMediaItem> *list)
{
    return static_cast<QmlMediaPlaylist*>(list->object)->itemCount();
}

/**
 * @brief unused
 * @return
 */
QmlMediaItem *QmlMediaPlaylist::item_at(QQmlListProperty<QmlMediaItem> *, int)
{
    return nullptr;
}

void QmlMediaPlaylist::item_clear(QQmlListProperty<QmlMediaItem> *list)
{
    static_cast<QmlMediaPlaylist*>(list->object)->clear();
}
