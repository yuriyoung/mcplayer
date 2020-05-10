#ifndef QMLMEDIAPLAYLIST_H
#define QMLMEDIAPLAYLIST_H

#include "player/MediaPlaylist.h"
#include <QAbstractListModel>
#include <QtQml>

class QmlMediaItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource)
public:
    QmlMediaItem(QObject *parent = nullptr);

    QUrl source() const;
    void setSource(QUrl source);

private:
    QUrl m_source;
};

class QmlMediaPlaylist : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_DISABLE_COPY(QmlMediaPlaylist)
    Q_PROPERTY(PlaybackMode playbackMode READ playbackMode WRITE setPlaybackMode NOTIFY playbackModeChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QUrl currentSource READ currentSource NOTIFY currentSourceChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool readOnly READ readOnly NOTIFY readOnlyChanged)
    Q_PROPERTY(Error error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
    Q_PROPERTY(QQmlListProperty<QmlMediaItem> items READ items)
    Q_ENUMS(PlaybackMode Error)
    Q_INTERFACES(QQmlParserStatus)
    Q_CLASSINFO("DefaultProperty", "items")
public:
    enum Roles
    {
        TitleRole = Qt::UserRole + 1,
        ArtistRole,
        DurationRole,
        SourceRole,

        ColumnsRole,
    };

    enum PlaybackMode
    {
        CurrentItemOnce = MediaPlaylist::CurrentItemOnce,
        CurrentItemRepeat = MediaPlaylist::CurrentItemRepeat,
        Sequential = MediaPlaylist::Sequential,
        Loop = MediaPlaylist::Loop,
        Random = MediaPlaylist::Random
    };

    enum Error
    {
        NoError = MediaPlaylist::NoError,
        FormatError = MediaPlaylist::FormatError,
        FormatNotSupportedError = MediaPlaylist::FormatNotSupportedError,
        NetworkError = MediaPlaylist::NetworkError,
        AccessDeniedError = MediaPlaylist::AccessDeniedError
    };

    explicit QmlMediaPlaylist(QObject *parent = nullptr);
    ~QmlMediaPlaylist() override;

    void classBegin() override;
    void componentComplete() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) override;

    QHash<int, QByteArray> roleNames() const override;

    PlaybackMode playbackMode() const;
    void setPlaybackMode(PlaybackMode mode);

    int currentIndex() const;
    void setCurrentIndex(int index);

    MediaPlaylist *mediaPlaylist() const;

    int itemCount() const;
    bool readOnly() const;
    Error error() const;
    QString errorString() const;
    QUrl currentSource() const;

    QQmlListProperty<QmlMediaItem> items();

signals:
    void currentIndexChanged();
    void playbackModeChanged();
    void currentSourceChanged();
    void readOnlyChanged();
    void itemCountChanged();
    void errorChanged();
    void loadSucceed();
    void loadFailed();

    void itemAboutInsert(int start, int end);
    void itemInserted(int start, int end);
    void itemAboutRemove(int start, int end);
    void itemRemoved(int start, int end);
    void itemChanged(int start, int end);

public slots:
    QUrl source(int index);

    int nextIndex(int steps = 1);
    int previousIndex(int steps = 1);
    void next();
    void previous();
    void shuffle();

    void load(const QUrl &location, const QString &format = QString());
    bool save(const QUrl &location, const QString &format = QString());

    bool addItem(const QUrl &source);
    bool addItems(const QList<QUrl> &sources);

    bool insertItem(int index, const QUrl &source);
    bool insertItems(int index, const QList<QUrl> &sources);

    bool moveItem(int from, int to);
    bool removeItem(int index);
    bool removeItems(int start, int end);

    bool clear();

private slots:
    void beginInsertItems(int start, int end);
    void endInsertItems(int start, int end);
    void beginRemoveItems(int start, int end);
    void endRemoveItems(int start, int end);
    void changeItems(int start, int end);
    void playlistLoadFailed();

private:
    static void item_append(QQmlListProperty<QmlMediaItem> *list, QmlMediaItem* item);
    static int item_count(QQmlListProperty<QmlMediaItem> *list);
    static QmlMediaItem *item_at(QQmlListProperty<QmlMediaItem> *, int);
    static void item_clear(QQmlListProperty<QmlMediaItem> *list);

private:
    QScopedPointer<MediaPlaylist> m_playlist;
    QMap<QModelIndex, QVariant> m_data;
    QString m_errorString;
    MediaPlaylist::Error m_error;
    bool m_readOnly;
};

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE(QmlMediaItem))
QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE(QmlMediaPlaylist))

#endif // QMLMEDIAPLAYLIST_H
