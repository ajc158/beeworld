#ifndef CYLINDER_H
#define CYLINDER_H

#include "sceneobject.h"

class cylinder : public sceneObject
{
    Q_OBJECT
public:
    explicit cylinder(QVector3D location, QVector3D scaling, QVector3D rotation, QObject *parent = 0);
    bool isHit(QVector3D ray_loc, QVector3D ray_dir, float &t);
    void connectObject(spineMLNetworkServer *, QString);
    sceneObject * copy();

signals:

public slots:
};

#endif // CYLINDER_H
