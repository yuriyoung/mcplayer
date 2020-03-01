#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H

#include <QObject>

class MediaLibraryPrivate;

/**
 * @brief The MediaLibrary class provide a media library (local or netword media)
 * Artists
 * Albums
 * Tracks
 * Collections
 * Videos
 *
 * features:
 * add/remove devices
 * add/remove folders
 * add/remove files
 * list files
 * list playlist
 * search files
 *
 */
class MediaLibrary : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(d, MediaLibrary)
public:
    explicit MediaLibrary(QObject *parent = nullptr);
    ~MediaLibrary();

    /*!
     * \brief add file system folder, device, network url or other entry point
     * \param entryPoint
     */
    void addEntryPoint(const QString &entryPoint);
    void removeEntryPoint(const QString &entryPoint);

    void banFolder(const QString &entryPoint);
    void unbanFolder(const QString &entryPoint);

    void addDevice(const QString &uuid, const QString &path, bool removable);
    void removeDevice(const QString &uuid, const QString &path);

    /*!
     * \brief discover medis by specified path
     */
    void discover(const QString &entryPoint);

    /*!
     * \brief reload all medias
     */
    void reload();

    /*!
     * \brief reload media by specisfied entry point
     * \param entryPoint
     */
    void reload(const QString &entryPoint);

    /*!
     * \brief clean all media from library that are no longer reachable
     */
    void clean();

    bool supportedMediaExtension(const QString &ext);
    bool supportedPlaylistExtension(const QString &ext);

    /*!
     * \brief History
     * TODO:
     * - play history
     * - ...
     */

    /*!
     * \brief Database operator
     * TODO:
     *  - search : media/playlist/album/genre/artist/folder
     *  - list all tracks
     *  - list albums
     *  - list artists
     *  - list genres
     *  - list playlists
     *  - list tracks from album
     *  - list media from artist
     *  - list albums from artist
     *  - list albums from genre
     *  - list media from playlist
     *  - list media from folder
     *  - list folders
     *  - list sub folders
     *  - ...
     */
    // ...


signals:
    void trackDiscovered();
    void artistDiscovered();
    void albumDiscovered();
    void genreDiscovered();
    void playlistDiscovered();

public slots:

private:
    QScopedPointer<MediaLibraryPrivate> d;
};

#endif // MEDIALIBRARY_H
