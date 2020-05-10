#ifndef QMLWINDOW_H
#define QMLWINDOW_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QPointer>

class QmlWindow : public QObject
{
    Q_OBJECT
public:
    explicit QmlWindow(QObject *parent = nullptr);

    QQmlApplicationEngine *qmlEgine();

signals:

public slots:
    void show();

private:
    void registerQmlType();

private:
    QUrl m_indexUrl;
    QPointer<QQmlApplicationEngine> m_qmlEgnine;
};

#endif // QMLWINDOW_H
