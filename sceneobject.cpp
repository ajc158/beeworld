#include "sceneobject.h"
#include "scenetexture.h"

sceneObject::sceneObject(QObject *parent) :
    QObject(parent)
{
}

sceneObject::~sceneObject()
{
    if (this->texture != NULL) delete this->texture;
}

sceneObject * sceneObject::copy() {
    return new sceneObject();
}

QColor sceneObject::getTexture(float t, bool lighting){

   // something has gone wrong!
   if (this->texture == NULL) {
       return QColor(0,0,0);
   }

   // get the QColor from the texture, then apply the lighting condition if required
   QColor tempC = this->texture->getColour(this->lastHit, t);
   if (lighting) {
       tempC.setHsvF(tempC.hueF(), tempC.saturationF(), tempC.valueF()*this->light);
   }
   return tempC;

}

void sceneObject::debugPrint() {
    qDebug() << "Object " << name;
    qDebug() << "Loc = " << location;
    qDebug() << "Rot = " << rotation;
    qDebug() << "Scl = " << scaling;
    if (this->texture != NULL) {
        this->texture->dbgPrint();
    }
}
