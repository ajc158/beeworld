#ifndef RADIAL2_H
#define RADIAL2_H

#include "scenetexture.h"
#include "QDebug"
#include "QImage"
#include "QRgb"
#include "radial.h"

class rad_image : public radial
{
    Q_OBJECT
public:
    explicit rad_image(QObject *parent = 0);
    QColor getColour(QVector3D location, float t);
    void connectTexture(spineMLNetworkServer *, QString);

    sceneTexture * copy();

    void dbgPrint() {
        qDebug() << "Texture " << name;
        qDebug() << "Off = " << offset;
        qDebug() << "Frq = " << frequency;
        qDebug() << "Mtn = " << motion;
        qDebug() << "DkC = " << darkColour;
        qDebug() << "LtC = " << lightColour;
    }
    QImage im;

private:
    float frequency;
    float radialOffset;
    float radialMotion;
    //float motionOffset;
    QColor darkColour;
    QColor lightColour;
    bool isSine;



signals:

public slots:

    /*!
     * \brief setFrequency
     * \param Frequency
     * Controller input for Frequency
     */
    void setFrequency(QVector <float> frequency) {
        if (frequency.size() == 1) {
            this->frequency = frequency[0];
        }
    }

    /*!
     * \brief setRadialOffset
     * \param radialOffset
     * Controller input for radialOffset
     */
    void setRadialOffset(QVector <float> radialOffset) {
        //qDebug () << this->radialOffset << "moo" << this;
        if (radialOffset.size() == 1) {
            this->radialOffset = radialOffset[0];
        }
    }

    /*!
     * \brief addRadialOffset
     * \param radialOffset
     * Controller input for radialOffset
     */
    void addRadialOffset(float radialOffset) {
        this->radialOffset += radialOffset;
    }

    /*!
     * \brief setRadialMotion
     * \param radialMotion
     * Controller input for radialMotion
     */
    void setRadialMotion(QVector <float> radialMotion) {
        if (radialMotion.size() == 1) {
            this->radialMotion = radialMotion[0];
        }
    }

    /*!
     * \brief setDarkCol
     * \param col
     * Controller input for Light Colour
     */
    void setDarkCol(QVector <float> col) {
        if (col.size() == 3) {
            this->darkColour.setRedF(col[0]);
            this->darkColour.setGreenF(col[1]);
            this->darkColour.setBlueF(col[2]);
        }
    }

    /*!
     * \brief setLightCol
     * \param col
     * Controller input for Dark Colour
     */
    void setLightCol(QVector <float> col) {
        if (col.size() == 3) {
            this->lightColour.setRedF(col[0]);
            this->lightColour.setGreenF(col[1]);
            this->lightColour.setBlueF(col[2]);
        }
    }

    /*!
     * \brief setSine
     * \param col
     * Controller input for type of pattern
     */
    void setSine(QVector <float> sine) {
        if (sine.size() == 1) {
            this->isSine = sine[0];
        }
    }

};

#endif // RADIAL2_H
