#include "QmlPlaylistModel.h"

#include <QFileInfo>

QmlPlaylistModel::QmlPlaylistModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    setPlaylist(new MediaPlaylist);
}

int QmlPlaylistModel::rowCount(const QModelIndex &parent) const
{
    return m_playlist && !parent.isValid() ? m_playlist->mediaCount() : 0;
}

int QmlPlaylistModel::columnCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? Columns : 0;
}

QModelIndex QmlPlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!m_playlist || parent.isValid())
        return QModelIndex();

    if(row >= 0 && row < m_playlist->mediaCount() && column >= 0 && column < Columns)
        return createIndex(row, column);

    return QModelIndex();
}

QModelIndex QmlPlaylistModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

QVariant QmlPlaylistModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role == Qt::DisplayRole)
    {
        QVariant value = m_data[index];
        if (!value.isValid() && index.column() == Title)
        {
            QUrl location = m_playlist->media(index.row()).canonicalUrl();
            return QFileInfo(location.path()).fileName();
        }

        return value;
    }
    return QVariant();
}

bool QmlPlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role)
    m_data[index] = value;
    emit dataChanged(index, index);
    return true;
}

MediaPlaylist *QmlPlaylistModel::playlist() const
{
    return m_playlist.data();
}

void QmlPlaylistModel::setPlaylist(MediaPlaylist *playlist)
{
    if (m_playlist)
    {
        disconnect(m_playlist.data(), &MediaPlaylist::mediaAboutToInserted, this, &QmlPlaylistModel::beginInsertItems);
        disconnect(m_playlist.data(), &MediaPlaylist::mediaInserted, this, &QmlPlaylistModel::endInsertItems);
        disconnect(m_playlist.data(), &MediaPlaylist::mediaAboutToRemoved, this, &QmlPlaylistModel::beginRemoveItems);
        disconnect(m_playlist.data(), &MediaPlaylist::mediaRemoved, this, &QmlPlaylistModel::endRemoveItems);
        disconnect(m_playlist.data(), &MediaPlaylist::mediaChanged, this, &QmlPlaylistModel::changeItems);
    }

    beginResetModel();
    m_playlist.reset(playlist);
    if (m_playlist)
    {
        connect(m_playlist.data(), &MediaPlaylist::mediaAboutToInserted, this, &QmlPlaylistModel::beginInsertItems);
        connect(m_playlist.data(), &MediaPlaylist::mediaInserted, this, &QmlPlaylistModel::endInsertItems);
        connect(m_playlist.data(), &MediaPlaylist::mediaAboutToRemoved, this, &QmlPlaylistModel::beginRemoveItems);
        connect(m_playlist.data(), &MediaPlaylist::mediaRemoved, this, &QmlPlaylistModel::endRemoveItems);
        connect(m_playlist.data(), &MediaPlaylist::mediaChanged, this, &QmlPlaylistModel::changeItems);
    }
    endResetModel();
}

void QmlPlaylistModel::add(const QList<QUrl> &urls)
{
    foreach (auto url, urls)
    {
        m_playlist->addMedia(url);
    }
}

void QmlPlaylistModel::remove(int index)
{
    m_playlist->removeMedia(index);
}

void QmlPlaylistModel::clear()
{
    beginResetModel();

    m_data.clear();
    m_playlist->clear();

    endResetModel();
}

void QmlPlaylistModel::beginInsertItems(int start, int end)
{
    m_data.clear();
    beginInsertRows(QModelIndex(), start, end);
}

void QmlPlaylistModel::endInsertItems()
{
    endInsertRows();
}

void QmlPlaylistModel::beginRemoveItems(int start, int end)
{
    m_data.clear();
    beginRemoveRows(QModelIndex(), start, end);
}

void QmlPlaylistModel::endRemoveItems()
{
    endInsertRows();
}

void QmlPlaylistModel::changeItems(int start, int end)
{
    m_data.clear();
    emit dataChanged(index(start,0), index(end, Columns));
}
