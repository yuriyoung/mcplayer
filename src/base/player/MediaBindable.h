#ifndef MEDIABINDABLE_H
#define MEDIABINDABLE_H

#include "MediaObject.h"

class MediaBindable
{
    friend class MediaObject;
public:
    explicit MediaBindable();
    virtual ~MediaBindable();
    virtual MediaObject *mediaObject() const = 0;

protected:
    virtual bool setMediaObject(MediaObject *object) = 0;
};

#define MediaBindable_iid "org.mcplayer.mediabindable/1.0"
Q_DECLARE_INTERFACE(MediaBindable, MediaBindable_iid)

#endif // MEDIABINDABLE_H
