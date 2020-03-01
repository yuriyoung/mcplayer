#include "QmlWindow.h"

#include <QCoreApplication>

#ifdef QT_DEBUG
    static const QString QMLPrefix = QStringLiteral("../src/");
#else
    static const QString QMLPrefix = QStringLiteral("qrc:///");
#endif

QmlWindow::QmlWindow(QObject *parent)
    : QObject(parent)
    , m_qmlEgnine(new QQmlApplicationEngine(this))
{
    m_indexUrl = QUrl(QMLPrefix + QStringLiteral("ui/main.qml"));
    QObject::connect(m_qmlEgnine, &QQmlApplicationEngine::objectCreated,
                     this, [this](QObject *obj, const QUrl &objUrl)
    {
        if (!obj && m_indexUrl == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
}

QQmlApplicationEngine *QmlWindow::qmlEgine()
{
    return m_qmlEgnine;
}

void QmlWindow::qmlRegisterType()
{

}

void QmlWindow::show()
{
    m_qmlEgnine->load(m_indexUrl);
}
