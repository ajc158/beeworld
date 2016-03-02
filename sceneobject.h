#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <QObject>
#include <QXmlStreamReader>
#include <QVector3D>
#include <QColor>
#include <QMatrix4x4>
#include <spinemlnetworkserver.h>

class sceneTexture;

class sceneObject : public QObject
{
    Q_OBJECT
public:
    explicit sceneObject(QObject *parent = 0);

    virtual sceneObject * copy();

    // constructor to read from an XML scene description - bool is used to denote an error if set to false
    sceneObject(QXmlStreamReader * /* xmlIn */, bool &ok, QObject * parent = 0) {ok = false;}
    //~sceneObject() {if (texture) delete texture;}
    // default behavior is we don't have a subclass is to say we are not hit
    /*!
     * \brief isHit
     * \return
     * Function that checks if the object is hit - if it is then the object stores the hit location so that
     * it can be used if a texture request is made (i.e. if the object is the closest hit object)
     */
    virtual bool isHit(QVector3D /* ray_loc */, QVector3D /* ray_dir */, float &/* t */) {return false;}

    /*!
     * \brief setTextureGenerator
     * \param texture
     * Set the texture generator
     */
    void setTextureGenerator (sceneTexture * texture) {this->texture = texture;}

    /*!
     * \brief getTexture
     * \param t
     * \return
     * Get the texture colour at the last hit location
     */
    QColor getTexture(float t, bool lighting);

    /*!
     * \brief connectObject
     * Function to allow the object to connect itself to an input from a model
     */
    virtual void connectObject(spineMLNetworkServer *, QString) {return;}

    /*!
     * \brief getSceneTexture
     * \return
     * Accessor for texture
     */
    sceneTexture * getSceneTexture() {return this->texture;}

    QString name;

    void debugPrint();

private:

protected:
   sceneTexture * texture;
   QVector3D location;
   QVector3D rotation;
   QVector3D scaling;
   QVector3D invScaling;

   QVector3D lastHit;
   float light;

   /*!
    * \brief vectorToObjectSpace
    * \param vect
    * \return
    * Transform a vector from the object space
    */
   QVector3D vectorFromObjectSpace(QVector3D vect) {
       QMatrix4x4 m;
       m.scale(scaling);
       m.rotate(rotation.x(), 1,0,0);
       vect = m * vect;
       m.setToIdentity();
       m.rotate(rotation.y(), 0,1,0);
       vect = m * vect;
       m.setToIdentity();
       m.rotate(rotation.z(), 0,0,1);
       vect = m * vect;
       m.setToIdentity();
       m.translate(location);
       vect = m * vect;
       return vect;
   }

   /*!
    * \brief vectorToObjectSpaceRS
    * \param vect
    * \return
    * Transform a vector from the object space
    */
   QVector3D vectorFromObjectSpaceRS(QVector3D vect) {
       QMatrix4x4 m;
       m.scale(scaling);
       m.rotate(rotation.x(), 1,0,0);
       vect = m * vect;
       m.setToIdentity();
       m.rotate(rotation.y(), 0,1,0);
       vect = m * vect;
       m.setToIdentity();
       m.rotate(rotation.z(), 0,0,1);
       //m.translate(location);
       vect = m * vect;
       return vect;
   }

   /*!
    * \brief vectorFromObjectSpace
    * \param vect
    * \return
    * Transform a vector to the object space
    */
   QVector3D vectorToObjectSpace(QVector3D vect) {
       QMatrix4x4 m;
       m.translate(-location);
       vect = m * vect;
       m.setToIdentity();
       m.rotate(-rotation.z(), 0,0,1);
       vect = m * vect;
       m.setToIdentity();
       m.rotate(-rotation.y(), 0,1,0);
       vect = m * vect;
       m.setToIdentity();
       m.rotate(-rotation.x(), 1,0,0);
       vect = m * vect;
       m.setToIdentity();
       m.scale(invScaling);
       vect = m * vect;
       return vect;
   }

   /*!
    * \brief vectorFromObjectSpaceRS
    * \param vect
    * \return
    * Transform a vector to the object space
    */
   QVector3D vectorToObjectSpaceRS(QVector3D vect) {
       QMatrix4x4 m;
       //m.translate(-location);
       m.rotate(-rotation.z(), 0,0,1);
       vect = m * vect;
       m.setToIdentity();
       m.rotate(-rotation.y(), 0,1,0);
       vect = m * vect;
       m.setToIdentity();
       m.rotate(-rotation.x(), 1,0,0);
       vect = m * vect;
       m.setToIdentity();
       m.scale(invScaling);
       vect = m * vect;
       return vect;
   }

signals:

public slots:

   /*!
    * \brief setLocationX
    * \param x
    * Controller input for Location X
    */
   void setLocationX(QVector <float> x) {
       if (x.size() == 1) {
           location.setX(x[0]);
       }
   }
   /*!
    * \brief setLocationY
    * \param y
    * Controller input for Location Y
    */
   void setLocationY(QVector <float> y) {
       if (y.size() == 1) {
           location.setY(y[0]);
       }
   }
   /*!
    * \brief setLocationZ
    * \param z
    * Controller input for Location Z
    */
   void setLocationZ(QVector <float> z) {
       if (z.size() == 1) {
           location.setZ(z[0]);
       }
   }
   /*!
    * \brief setLocation
    * \param Location
    * Controller input for Location
    */
   void setLocation(QVector <float> location) {
       if (location.size() == 3) {
           this->location.setX(location[0]);
           this->location.setY(location[1]);
           this->location.setZ(location[2]);
       }
   }

   /*!
    * \brief setRotationX
    * \param x
    * Controller input for Rotation X
    */
   void setRotationX(QVector <float> x) {
       if (x.size() == 1) {
           rotation.setX(x[0]);
       }
   }
   /*!
    * \brief setRotationY
    * \param y
    * Controller input for Rotation Y
    */
   void setRotationY(QVector <float> y) {
       if (y.size() == 1) {
           rotation.setY(y[0]);
       }
   }
   /*!
    * \brief setRotationZ
    * \param z
    * Controller input for Rotation Z
    */
   void setRotationZ(QVector <float> z) {
       if (z.size() == 1) {
           rotation.setZ(z[0]);
       }
   }
   /*!
    * \brief setRotation
    * \param Rotation
    * Controller input for Rotation
    */
   void setRotation(QVector <float> rotation) {
       if (rotation.size() == 3) {
           this->rotation.setX(rotation[0]);
           this->rotation.setY(rotation[1]);
           this->rotation.setZ(rotation[2]);
       }
   }

   /*!
    * \brief setScalingX
    * \param x
    * Controller input for Scaling X
    */
   void setScalingX(QVector <float> x) {
       if (x.size() == 1) {
           scaling.setX(x[0]);
       }
   }
   /*!
    * \brief setScalingY
    * \param y
    * Controller input for Scaling Y
    */
   void setScalingY(QVector <float> y) {
       if (y.size() == 1) {
           scaling.setY(y[0]);
       }
   }
   /*!
    * \brief setScalingZ
    * \param z
    * Controller input for Scaling Z
    */
   void setScalingZ(QVector <float> z) {
       if (z.size() == 1) {
           scaling.setZ(z[0]);
       }
   }
   /*!
    * \brief setScaling
    * \param Scaling
    * Controller input for Scaling
    */
   void setScaling(QVector <float> scaling) {
       if (scaling.size() == 3) {
           this->scaling.setX(scaling[0]);
           this->scaling.setY(scaling[1]);
           this->scaling.setZ(scaling[2]);
       }
   }

};

#endif // SCENEOBJECT_H
