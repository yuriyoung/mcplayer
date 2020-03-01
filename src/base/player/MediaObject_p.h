#ifndef MEDIAOBJECT_P_H
#define MEDIAOBJECT_P_H

#include "MediaObject.h"
#include "MediaEngine.h"
#include "MediaMetadataControl.h"

#include <QTimer>
#include <QSet>

class MediaObject;
class MediaEngine;
class MediaMetadataControl;

class MediaObjectPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(MediaObject)
public:
    MediaObjectPrivate() {}
    virtual ~MediaObjectPrivate() {}

    MediaEngine *engine = nullptr;
    MediaMetadataControl *metadataControl = nullptr;

    QTimer* notifyTimer = nullptr;
    QSet<int> notifyProperties;

    MediaObject *q_ptr = nullptr;
};

#endif // MEDIAOBJECT_P_H
