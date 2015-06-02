#ifndef BEEWORLD_H
#define BEEWORLD_H

// This is a simple raytracer that uses a sherical backplane for the camera rather than a flat one.
// This avoids the issue of pinhole cameras only being useful up to about 90 degrees and allows
// panaoramic vision.

#include <QObject>
#include <QPixmap>
#include <vector>
#include "sceneobject.h"

using namespace std;

class beeworld : public QObject
{
    Q_OBJECT
public:
    explicit beeworld(QObject *parent = 0);

    float scale_factor_for_one_wall;
    float contrast_factor_for_one_wall;
    float offset_for_one_wall;
    int x_walls;
    int y_walls;
    float contrast;

    virtual QImage * getImage(float x, float y, float z, float dir, float pitch, float roll, float curr_t, float scale);

    virtual QImage * getImage(float x, float y, float z, float dir, float pitch, float roll, float curr_t);
    int numElements();
    void setWorldSize(int size);
    void setFOVScale(double fov_scale);
    void setFOV(float vert, float horiz);
    void setWorldSize(int vert, int horiz);
    void setLighting(bool onOff);
    void setBlur(bool onOff);
    void addObject(sceneObject * object);
    void clearObjects();
    sceneObject * getObject(int i);
    int objectCount();
    QVector3D drawnBeeLoc;
    QVector3D drawnBeeRot;
    bool drawBee;

protected:
    int N_ROWS;
    int N_COLS;
    float H_EXTENT;
    float V_EXTENT;
    QVector < sceneObject * > objects;
    float t2;
    bool lighting;
    bool blur;
    QVector <float> jitterX;
    QVector <float> jitterY;
    int jitterSeed;
    void setJitter();
    
signals:
    
public slots:
    
};

#endif // BEEWORLD_H
