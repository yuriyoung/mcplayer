#ifndef MEDIAENGINE_H
#define MEDIAENGINE_H

#include "MediaControl.h"
#include <QObject>

class MediaEngine : public QObject
{
    Q_OBJECT
public:
    explicit MediaEngine(QObject *parent = nullptr);

    virtual MediaControl *requestControl(const char *name) = 0;
    template <typename T> inline T requestControl()
    {
        if (MediaControl *control = requestControl(mediacontrol_iid<T>()))
        {
            if (T typedControl = qobject_cast<T>(control))
                return typedControl;
            releaseControl(control);
        }
        return 0;
    }

    virtual void releaseControl(MediaControl *control) = 0;
};

#endif // MEDIAENGINE_H
