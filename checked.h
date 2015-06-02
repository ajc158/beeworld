#ifndef CHECKED_H
#define CHECKED_H

#include "scenetexture.h"
#include "QDebug"

class checked : public sceneTexture
{
    Q_OBJECT
public:
    explicit checked(QObject *parent = 0);
    QColor getColour(QVector3D location, float t);
    void connectTexture(spineMLNetworkServer *, QString);

    void dbgPrint() {
        qDebug() << "Texture " << name;
        qDebug() << "Off = " << offset;
        qDebug() << "Frq = " << frequency;
        qDebug() << "Mtn = " << motion;
        qDebug() << "DkC = " << darkColour;
        qDebug() << "LtC = " << lightColour;
    }

private:
    QVector3D frequency;
    QColor darkColour;
    QColor lightColour;
    bool isSine;

signals:

public slots:
    /*!
     * \brief setFrequencyX
     * \param x
     * Controller input for Frequency X
     */
    void setFrequencyX(QVector <float> x) {
        if (x.size() == 1) {
            frequency.setX(x[0]);
        }
    }
    /*!
     * \brief setFrequencyY
     * \param y
     * Controller input for Frequency Y
     */
    void setFrequencyY(QVector <float> y) {
        if (y.size() == 1) {
            frequency.setY(y[0]);
        }
    }
    /*!
     * \brief setFrequencyZ
     * \param z
     * Controller input for Frequency Z
     */
    void setFrequencyZ(QVector <float> z) {
        if (z.size() == 1) {
            frequency.setZ(z[0]);
        }
    }
    /*!
     * \brief setFrequency
     * \param Frequency
     * Controller input for Frequency
     */
    void setFrequency(QVector <float> frequency) {
        if (frequency.size() == 3) {
            this->frequency.setX(frequency[0]);
            this->frequency.setY(frequency[1]);
            this->frequency.setZ(frequency[2]);
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
     * \brief setIsSine
     * \param sine
     * Controller input setting if sine waves used
     */
    void setIsSine(QVector <float> sine) {
        if (sine.size() == 1) {
            this->isSine = sine[0];
        }
    }

};

#endif // CHECKED_H
