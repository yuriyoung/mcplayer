#include "QmlWindow.h"
#include "QmlMediaPlayer.h"
#include "QmlMediaMetadata.h"
#include "QmlMediaPlaylist.h"
#include "utils/TimeTick.h"

#include <QCoreApplication>

#ifdef QT_DEBUG
    // MacOS: copy the "src/ui" folder (all qml files) to McPlayer.app/Contents/
    static const QString QMLPrefix = QStringLiteral("../src/");
#else
    static const QString QMLPrefix = QStringLiteral("qrc:///");
#endif

QmlWindow::QmlWindow(QObject *parent)
    : QObject(parent)
    , m_qmlEgnine(new QQmlApplicationEngine(this))
{

}

QQmlApplicationEngine *QmlWindow::qmlEgine()
{
    return m_qmlEgnine;
}

void QmlWindow::registerQmlType()
{
    qRegisterMetaType<TimeTick>();

    qmlRegisterType<QmlMediaPlayer>("org.mcplayer", 1, 0, "MediaPlayer");
    qmlRegisterType<QmlMediaPlaylist>("org.mcplayer", 1, 0, "MediaPlaylist");
    qmlRegisterType<QmlMediaItem>("org.mcplayer", 1, 0, "MediaItem");
    qmlRegisterUncreatableType<TimeTick>("org.mcplayer", 0, 1, "TimeTick", "");

    //expose base object to QML, they aren't instanciable from QML side
    qmlRegisterType<QmlMediaMetadata>();
}

void QmlWindow::show()
{
    registerQmlType();

    m_indexUrl = QUrl(QMLPrefix + QStringLiteral("ui/main.qml"));
    QObject::connect(m_qmlEgnine, &QQmlApplicationEngine::objectCreated,
                     this, [this](QObject *obj, const QUrl &objUrl)
    {
        if (!obj && m_indexUrl == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    m_qmlEgnine->load(m_indexUrl);
}
