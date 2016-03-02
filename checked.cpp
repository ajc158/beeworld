#include "checked.h"
#include <QtCore/qmath.h>

checked::checked(QObject *parent) :
    sceneTexture(parent)
{
    this->darkColour = QColor(255,200,200);
    this->lightColour = QColor(255,200,200);
    this->frequency = QVector3D(.5,.5,.0001);
    this->offset = QVector3D(0.01,0.01,0.01);
    this->motion = QVector3D(0,0,0);
}

QColor checked::getColour(QVector3D location, float t) {

    // get the value at the location and time
    QVector3D vals;
    QVector3D vals_for_sin = (location * (this->frequency * M_PI * 2)) - this->offset - this->motion * t;
    //QVector3D vals;

    // generate detail to add
    /*QVector3D vals_for_detail = (location * QVector3D(1.00234,1.008848,1.0066848));
    int v_x = (int) vals_for_detail.x();
    int v_y = (int) vals_for_detail.y();
    int v_z = (int) vals_for_detail.z();

    int val = v_x*v_y + v_x*v_z + v_z*v_y;
    val = ((val * 1103515245 + 12345) % 100)+50;
    //QColor detail = QColor(val,val,val);*/

    if (isSine) {
        vals.setX(qSin(vals_for_sin.x()));
        vals.setY(qSin(vals_for_sin.y()));
        vals.setZ(qSin(vals_for_sin.z()));

        float valsXYZ = vals.x()*vals.y()*vals.z();

        valsXYZ=valsXYZ*0.5+0.5;

        return QColor::fromRgbF((lightColour.redF() - darkColour.redF())*valsXYZ + darkColour.redF(), \
                                (lightColour.greenF() - darkColour.greenF())*valsXYZ + darkColour.greenF(), \
                                (lightColour.blueF() - darkColour.blueF())*valsXYZ + darkColour.blueF());
    } else {
        vals.setX(qCeil(qSin(vals_for_sin.x())));
        vals.setY(qCeil(qSin(vals_for_sin.y())));
        vals.setZ(qCeil(qSin(vals_for_sin.z())));

        if (vals.x() + vals.y() == 0 || (vals.x() * vals.y()) == 1) {
            if (vals.z()) {
                return darkColour;
                //return darkColour.lighter(val);
            } else {
                return lightColour;
                //return lightColour.darker(val);
            }
        } else {
            if (vals.z()) {
                return lightColour;
                //return lightColour.darker(val);
            } else {
                return darkColour;
                //return darkColour.lighter(val);
            }
        }
    }

}

void checked::connectTexture(spineMLNetworkServer * src, QString port) {
    if (port == "Frequency") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setFrequency(QVector<float>)));
    }
    if (port == "FrequencyX") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setFrequencyX(QVector<float>)));
    }
    if (port == "FrequencyY") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setFrequencyY(QVector<float>)));
    }
    if (port == "FrequencyZ") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setFrequencyZ(QVector<float>)));
    }
    if (port == "Offset") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setOffset(QVector<float>)));
    }
    if (port == "OffsetX") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setOffsetX(QVector<float>)));
    }
    if (port == "OffsetY") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setOffsetY(QVector<float>)));
    }
    if (port == "OffsetZ") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setOffsetZ(QVector<float>)));
    }
    if (port == "Motion") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setMotion(QVector<float>)));
    }
    if (port == "MotionX") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setMotionX(QVector<float>)));
    }
    if (port == "MotionY") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setMotionY(QVector<float>)));
    }
    if (port == "MotionZ") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setMotionZ(QVector<float>)));
    }
    if (port == "DarkCol") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setDarkCol(QVector<float>)));
    }
    if (port == "LightCol") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setLightCol(QVector<float>)));
    }
}

sceneTexture * checked::copy() {
    checked * newTex = new checked();
    QVector<float>vals;
    vals.push_back(this->darkColour.redF());
    vals.push_back(this->darkColour.greenF());
    vals.push_back(this->darkColour.blueF());
    newTex->setDarkCol(vals);
    vals.clear();
    vals.push_back(this->lightColour.redF());
    vals.push_back(this->lightColour.greenF());
    vals.push_back(this->lightColour.blueF());
    newTex->setLightCol(vals);
    vals.clear();
    vals.push_back(this->frequency.x());
    vals.push_back(this->frequency.y());
    vals.push_back(this->frequency.z());
    newTex->setFrequency(vals);
    vals.clear();
    vals.push_back(this->offset.x());
    vals.push_back(this->offset.y());
    vals.push_back(this->offset.z());
    newTex->setOffset(vals);
    vals.clear();
    vals.push_back(this->motion.x());
    vals.push_back(this->motion.y());
    vals.push_back(this->motion.z());
    newTex->setMotion(vals);
    vals.clear();
    vals.push_back(this->isSine);
    newTex->setIsSine(vals);
    return newTex;
}
