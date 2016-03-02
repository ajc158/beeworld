#ifndef PLANE_H
#define PLANE_H

#include "sceneobject.h"

class plane : public sceneObject
{
    Q_OBJECT
public:
    explicit plane(QVector3D location, QVector3D scaling, QVector3D rotation,QObject *parent = 0);
    bool isHit(QVector3D ray_loc, QVector3D ray_dir, float &t);
    void connectObject(spineMLNetworkServer *, QString);
    sceneObject * copy();

signals:

public slots:

};

#endif // PLANE_H
