#include "QmlWindow.h"

//
#include "QmlMediaPlayer.h"
#include "QmlMediaMetadata.h"
#include "QmlPlaylistModel.h"
#include "utils/Lazy.h"
#include "utils/TimeTick.h"

#include <QCoreApplication>

#ifdef QT_DEBUG
    static const QString QMLPrefix = QStringLiteral("../src/");
#else
    static const QString QMLPrefix = QStringLiteral("qrc:///");
#endif

// for testing ...
class QmlWindowPrivate
{
public:
    QmlWindowPrivate(QmlWindow *window)
        : lazyPlayer([=]{ return new QmlMediaPlayer(window); })
        , lazyPlaylistModel([=]{ return new QmlPlaylistModel(window); })
    {

    }

    // for testing...
    Util::Lazy<QmlMediaPlayer> lazyPlayer;
    Util::Lazy<QmlPlaylistModel> lazyPlaylistModel;
};

QmlWindow::QmlWindow(QObject *parent)
    : QObject(parent)
    , d(new QmlWindowPrivate(this))
    , m_qmlEgnine(new QQmlApplicationEngine(this))
{

}

QQmlApplicationEngine *QmlWindow::qmlEgine()
{
    return m_qmlEgnine;
}

void QmlWindow::qmlRegisterType()
{
    ::qmlRegisterType<QmlMediaPlayer>("McPlayer", 1, 0, "MediaPlayer");
    qmlRegisterUncreatableType<TimeTick>("McPlayer", 0, 1, "TimeTick", "");
    qRegisterMetaType<TimeTick>();

/*!
 *-----------------------------------------------------------------
 * for play testing...
 *-----------------------------------------------------------------
 */
    // init
    m_player = d->lazyPlayer.get();
    m_player->classBegin();
    m_player->componentComplete();
    m_metaData = m_player->metadata();

    // create a playlist
    m_playlistModel = d->lazyPlaylistModel.get();
    m_playlistModel->add({
        QUrl(QStringLiteral("file:///E:/Users/Yuri/Music/Dearest.flac")),
        QUrl(QStringLiteral("file:///E:/Users/Yuri/Music/hui_yi_de_sha_lou.wav")),
    });
    m_player->setPlaylist(m_playlistModel->playlist());

    // Register qml global properties
    m_qmlEgnine->rootContext()->setContextProperty("mediaPlayer", m_player);
    m_qmlEgnine->rootContext()->setContextProperty("mediaMetaData", m_metaData);
    m_qmlEgnine->rootContext()->setContextProperty("playlistModel", m_playlistModel);
}

void QmlWindow::show()
{
    m_indexUrl = QUrl(QMLPrefix + QStringLiteral("ui/main.qml"));
    QObject::connect(m_qmlEgnine, &QQmlApplicationEngine::objectCreated,
                     this, [this](QObject *obj, const QUrl &objUrl)
    {
        if (!obj && m_indexUrl == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    m_qmlEgnine->load(m_indexUrl);
}
