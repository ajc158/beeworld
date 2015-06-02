#ifndef SPHERE_H
#define SPHERE_H

#include "sceneobject.h"

class sphere : public sceneObject
{
    Q_OBJECT
public:
    explicit sphere(QVector3D location, QVector3D scaling, QVector3D rotation, QObject *parent = 0);
    bool isHit(QVector3D ray_loc, QVector3D ray_dir, float &t);
    void connectObject(spineMLNetworkServer *, QString);

private:

signals:

public slots:

};

#endif // SPHERE_H
