#ifndef APPLICATION_H
#define APPLICATION_H

#include "global.h"

#ifndef DISABLE_GUI
#include <QGuiApplication>
using BaseApplication = QGuiApplication;
#else
#include <QCoreApplication>
using BaseApplication = QCoreApplication;
#endif

#define app() (Application::instance())

class MediaPlayer;

class ApplicationPrivate;
class Application : public BaseApplication
{
    Q_OBJECT
    D_PTR(Application)
    Q_DISABLE_COPY(Application)
public:
    explicit Application(int &argc, char **argv);
    ~Application();

    MediaPlayer *player() const;

    int exec(const QStringList &params = {});

signals:

public slots:

};

#endif // APPLICATION_H
