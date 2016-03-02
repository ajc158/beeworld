#include "cylinder.h"
#include "scenetexture.h"
#include <QtCore/qmath.h>

cylinder::cylinder(QVector3D location, QVector3D scaling, QVector3D rotation,QObject *parent) :
    sceneObject(parent)
{
    this->texture = NULL;
    this->location = location;
    this->scaling = scaling;
    this->invScaling.setX(1.0/scaling.x());
    this->invScaling.setY(1.0/scaling.y());
    this->invScaling.setZ(1.0/scaling.z());
    this->rotation = rotation;
}

bool cylinder::isHit(QVector3D ray_loc, QVector3D ray_dir, float &t) {

    QVector3D ray_loc_trans = this->vectorToObjectSpace(ray_loc);
    QVector3D ray_dir_trans = this->vectorToObjectSpaceRS(ray_dir);

    QVector3D ray_loc_noZ = ray_loc_trans;
    ray_loc_noZ.setZ(0);
    QVector3D ray_dir_noZ = ray_dir_trans;
    ray_dir_noZ.setZ(0);

    // find out if the line intersects the sphere using a quadratic
    float A = ray_dir_noZ.lengthSquared();
    float B = 2.0 * QVector3D::dotProduct(ray_loc_noZ, ray_dir_noZ);
    float C = (ray_loc_noZ).lengthSquared() - 1.0;
    float det = qPow(B,2) - 4*A*C;
    if (det > 0) {
        t = (-B + sqrt(det)) / (2.0 * A);
        float t2 = (-B - sqrt(det)) / (2.0 * A);
        // check z bounds
        float zHit = ray_loc_trans.z() + ray_dir_trans.z() * t2;
        t = ((t2 < t) && (t2 >0) && (zHit > 0.0 && zHit < 1.0)) ? t2 : t;
        zHit = ray_loc_trans.z() + ray_dir_trans.z() * t;
        if (t > 0 && (zHit > 0.0 && zHit < 1.0)) {
            // store location of hit
            this->lastHit = ray_loc + ray_dir * t;
            // angle of normal to sun for use in texturing
            this->light = QVector3D::dotProduct(this->vectorFromObjectSpaceRS(ray_loc_noZ + ray_dir_noZ * t).normalized(), QVector3D(0.707,0,0.707) /* the sun */) * 0.5 + 0.5;
            return true;
        }
    }
    // no hit found
    return false;
}

void cylinder::connectObject(spineMLNetworkServer * src, QString port) {
    if (port == "Location") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setLocation(QVector<float>)));
    }
    if (port == "LocationX") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setLocationX(QVector<float>)));
    }
    if (port == "LocationY") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setLocationY(QVector<float>)));
    }
    if (port == "LocationZ") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setLocationZ(QVector<float>)));
    }
    if (port == "Rotation") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setRotation(QVector<float>)));
    }
    if (port == "RotationX") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setRotationX(QVector<float>)));
    }
    if (port == "RotationY") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setRotationY(QVector<float>)));
    }
    if (port == "RotationZ") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setRotationZ(QVector<float>)));
    }
    if (port == "Scaling") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setScaling(QVector<float>)));
    }
    if (port == "ScalingX") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setScalingX(QVector<float>)));
    }
    if (port == "ScalingY") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setScalingY(QVector<float>)));
    }
    if (port == "ScalingZ") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setScalingZ(QVector<float>)));
    }
}

sceneObject * cylinder::copy() {
    // create a new plane and return a pointer
    cylinder * newObj = new cylinder(this->location, this->scaling, this->rotation);
    newObj->setTextureGenerator(this->texture->copy());
    return newObj;
}
