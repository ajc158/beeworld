#include "radial.h"
#include <QtCore/qmath.h>

radial::radial(QObject *parent) :
    sceneTexture(parent)
{
    this->darkColour = QColor(0,0,0);
    this->lightColour = QColor(255,200,200);
    this->frequency = 0.5;
    this->offset = QVector3D(0.01,0.01,0.01);
    this->motion = QVector3D(0,-40,0);
    this->isSine = false;
}

QColor radial::getColour(QVector3D location, float t) {

    // set the location centre
    QVector3D adjLoc = location - this->offset;

    // calculate the angle
    float angle = qAtan2(adjLoc.x(),adjLoc.y());

    // apply the scaling and offset to the angle
    if (isSine) {
        float sine = 0.5*qSin((angle+(radialOffset+radialMotion*t/1000.0)/180.0*M_PI)*frequency)+0.5;
        return QColor::fromRgbF((lightColour.redF() - darkColour.redF())*sine + darkColour.redF(), \
                                (lightColour.greenF() - darkColour.greenF())*sine + darkColour.greenF(), \
                                (lightColour.blueF() - darkColour.blueF())*sine + darkColour.blueF());
    } else {
        if (round(0.5*qSin((angle+(radialOffset+radialMotion*t/1000.0)/180.0*M_PI)*frequency)+0.5)) {
            return darkColour;
        } else {
            return lightColour;
        }
    }

}

void radial::connectTexture(spineMLNetworkServer * src, QString port) {
    if (port == "Frequency") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setFrequency(QVector<float>)));
    }
    if (port == "RadialOffset") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setRadialOffset(QVector<float>)));
    }
    if (port == "RadialMotion") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setRadialMotion(QVector<float>)));
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
    if (port == "IsSine") {
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(setSine(QVector<float>)));
    }
}

sceneTexture * radial::copy() {
    radial * newTex = new radial();
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
    vals.push_back(this->frequency);
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
    vals.push_back(this->radialOffset);
    newTex->setRadialOffset(vals);
    vals.clear();
    vals.push_back(this->radialMotion);
    newTex->setRadialMotion(vals);
    vals.clear();
    vals.push_back(this->isSine);
    newTex->setSine(vals);
    return newTex;
}
