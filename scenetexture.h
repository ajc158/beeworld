#ifndef SCENETEXTURE_H
#define SCENETEXTURE_H

#include <QObject>
#include <QVector3D>
#include <QColor>
#include <QVector>
#include "spinemlnetworkserver.h"

class sceneTexture : public QObject
{
    Q_OBJECT
public:
    explicit sceneTexture(QObject *parent = 0);
    /*!
     * \brief getColour
     * \return
     * Obtain the colour of the 3D procedural texture at the specified location in 3D space and time
     */
    virtual QColor getColour(QVector3D /* location */, float /* t */) {return QColor(0,0,0);}
    /*!
     * \brief connectObject
     * Function to allow the texture to connect itself to an input from a model
     */
    virtual void connectTexture(spineMLNetworkServer *, QString) {return;}

    virtual void dbgPrint() {}
    
    virtual sceneTexture * copy() {return new sceneTexture();}

    QString name;

private:

protected:
    QVector3D offset;
    QVector3D motion;

signals:

public slots:
    /*!
     * \brief setOffsetX
     * \param x
     * Controller input for offset X
     */
    void setOffsetX(QVector <float> x) {
        if (x.size() == 1) {
            offset.setX(x[0]);
        }
    }
    /*!
     * \brief setOffsetY
     * \param y
     * Controller input for offset Y
     */
    void setOffsetY(QVector <float> y) {
        if (y.size() == 1) {
            offset.setY(y[0]);
        }
    }
    /*!
     * \brief setOffsetZ
     * \param z
     * Controller input for offset Z
     */
    void setOffsetZ(QVector <float> z) {
        if (z.size() == 1) {
            offset.setZ(z[0]);
        }
    }
    /*!
     * \brief setOffset
     * \param offset
     * Controller input for offset
     */
    void setOffset(QVector <float> offset) {
        if (offset.size() == 3) {
            this->offset.setX(offset[0]);
            this->offset.setY(offset[1]);
            this->offset.setZ(offset[2]);
        }
    }

    /*!
     * \brief setMotionX
     * \param x
     * Controller input for Motion X
     */
    void setMotionX(QVector <float> x) {
        if (x.size() == 1) {
            motion.setX(x[0]);
        }
    }
    /*!
     * \brief setMotionY
     * \param y
     * Controller input for Motion Y
     */
    void setMotionY(QVector <float> y) {
        if (y.size() == 1) {
            motion.setY(y[0]);
        }
    }
    /*!
     * \brief setMotionZ
     * \param z
     * Controller input for Motion Z
     */
    void setMotionZ(QVector <float> z) {
        if (z.size() == 1) {
            motion.setZ(z[0]);
        }
    }
    /*!
     * \brief setMotion
     * \param Motion
     * Controller input for Motion
     */
    void setMotion(QVector <float> motion) {
        if (motion.size() == 3) {
            this->motion.setX(motion[0]);
            this->motion.setY(motion[1]);
            this->motion.setZ(motion[2]);
        }
    }
};

#endif // SCENETEXTURE_H
