#ifndef MEDIACONTROL_H
#define MEDIACONTROL_H

#include <QObject>

class MediaControl : public QObject
{
    Q_OBJECT
public:
    virtual ~MediaControl();

protected:
    explicit MediaControl(QObject *parent = nullptr);
};

template <typename T> const char *mediacontrol_iid() { return nullptr; }
#define MEDIA_DECLARE_CONTROL(Class, IID) \
    template <> inline const char *mediacontrol_iid<Class *>() { return IID; }

#endif // MEDIACONTROL_H
