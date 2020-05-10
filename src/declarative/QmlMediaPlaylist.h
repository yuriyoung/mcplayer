#ifndef QMLPLAYLISTMODEL_H
#define QMLPLAYLISTMODEL_H

#include "player/MediaPlaylist.h"
#include <QAbstractItemModel>

class QmlPlaylistModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Column
    {
        Title = 0,
        Artist,
        Duration,

        Columns,
    };

    explicit QmlPlaylistModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) override;

    MediaPlaylist *playlist() const;
    void setPlaylist(MediaPlaylist *playlist);

    Q_INVOKABLE void add(const QList<QUrl> &urls);
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void clear();

public slots:

private slots:
    void beginInsertItems(int start, int end);
    void endInsertItems();
    void beginRemoveItems(int start, int end);
    void endRemoveItems();
    void changeItems(int start, int end);

private:
    QScopedPointer<MediaPlaylist> m_playlist;
    QMap<QModelIndex, QVariant> m_data;
};

#endif // QMLPLAYLISTMODEL_H
