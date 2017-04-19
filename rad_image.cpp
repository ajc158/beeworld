#include "rad_image.h"
#include <QtCore/qmath.h>

rad_image::rad_image(QObject *parent) :
    radial(parent)
{
    this->darkColour = QColor(0,0,0);
    this->lightColour = QColor(255,200,200);
    this->frequency = 0.5;
    this->offset = QVector3D(0.01,0.01,0.01);
    this->motion = QVector3D(0,-40,0);
    this->isSine = false;
    //im = NULL;
}

QColor rad_image::getColour(QVector3D location, float t) {

    Q_UNUSED(t)

    // set the location centre
    QVector3D adjLoc = location - this->offset;

    // calculate the angles
    float angle_h = qAtan2(adjLoc.x(),adjLoc.y());
    //float angle_v = qAtan2(qSqrt(qPow(adjLoc.x(),2)+qPow(adjLoc.y(),2)),adjLoc.z());

    if (im.isNull()) {
        return QColor::fromRgbF(1.0f,0.0f,0.0f);
    }

    // convert into Pixels
    int y = adjLoc.z()/30.0*im.height();
    if (y > im.height()-1) y = im.height()-1;
    if (y < 0) y = 0;
    QRgb pix = im.pixel((angle_h/(2.0f*M_PI)+0.5)*im.width(),y);
    return QColor::fromRgb(pix);

    // apply the scaling and offset to the angle
    /*if (isSine) {
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
    }*/

}

void rad_image::connectTexture(spineMLNetworkServer * src, QString port) {
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

sceneTexture * rad_image::copy() {
    rad_image * newTex = new rad_image();
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
    newTex->im = this->im.copy();
    return newTex;
}
