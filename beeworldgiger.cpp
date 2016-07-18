#include "beeworld.h"
#include "beeworldgiger.h"
#include "math.h"
#include <QDebug>
#include <QThread>
#include <QMutex>

// include primitives
#include"sphere.h"
#include "cylinder.h"
#include "plane.h"

// include textures
#include "checked.h"

#include "gigerdata.h"


int a_RNGg = 1103515245;
int c_RNGg = 12345;


#define ONE_OVER_FLOOR_SIZE 0.5
//1.5

class beethread : public QThread
{
public:
    int starti;
    int end;
    int eye;
    QImage * image;
    QVector < sceneObject * > objects;
    float roll;
    float dir;
    float pitch;
    float x;
    float y;
    float z;
    float curr_t;
    bool lighting;
    QVector <float> * jitterX;
    QVector <float> * jitterY;

private:
    void run() {
        for (int currIndex=starti; currIndex < end; ++currIndex) {
            float v_ang = gdata[currIndex][1]-70-(*jitterX)[currIndex];
            float h_ang = gdata[currIndex][0]*-eye-(*jitterY)[currIndex];; // flip angle for different eyes

            int h = (70-gdata[currIndex][2])*-eye;
            int v = gdata[currIndex][3];

            int shift = eye<0 ? 0 : 140; // shift pixels for right eye

            // work out vector
            float x_vect = sin(h_ang/180.0*3.14);
            float y_vect = cos(h_ang/180.0*3.14);
            float z_vect = -sin(v_ang/180.0*3.14);

            // normalise
            float sum = sqrt(pow(x_vect, 2) + pow(y_vect, 2) + pow(z_vect, 2));
            x_vect /= sum;
            y_vect /= sum;
            z_vect /= sum;

            // convert from bee co-ordinates to world co-ordinates
            // roll
            float x_temp = x_vect;
            float z_temp = z_vect;
            x_vect = cos(roll) * x_temp - sin(roll) * z_temp;
            z_vect = sin(roll) * x_temp + cos(roll) * z_temp;
            // pitch
            float y_temp = y_vect;
            z_temp = z_vect;
            y_vect = cos(pitch) * y_temp - sin(pitch) * z_temp;
            z_vect = sin(pitch) * y_temp + cos(pitch) * z_temp;
            // direction
            x_temp = x_vect;
            y_temp = y_vect;
            x_vect = cos(dir) * x_temp - sin(dir) * y_temp;
            y_vect = sin(dir) * x_temp + cos(dir) * y_temp;

            // test all objects in the world for interaction

            float min_t = 10000000.0;
            int min_ind = -1;
            for (int k = 0; k < objects.size(); ++k) {
                float t;
                bool isHit = objects[k]->isHit(QVector3D(x,y,z), QVector3D(x_vect,y_vect,z_vect), t);
                if (isHit && t < min_t) {
                    min_t = t;
                    min_ind = k;
                }
            }


            if (min_ind != -1) {
                image->setPixel(h+shift, v,objects[min_ind]->getTexture(curr_t, this->lighting).rgb());
            } else if (z_vect < 0) {
                // generate the procedural floor for use in optic flow
                int valx = abs(int((-z/z_vect * (x_vect) + x) * ONE_OVER_FLOOR_SIZE));
                int valy = abs(int((-z/z_vect * (y_vect) + y) * ONE_OVER_FLOOR_SIZE));
                int val = valx*valx + valy*valy;
                val = abs(val * a_RNGg + c_RNGg);
                //val = 100;
                val = val % 255;
                image->setPixel(h+shift, v,QColor(val, val, val).rgb());
            }
            //omp_unset_lock(&writelock);


        }
        // clear up objects
        for (int i = 0; i < this->objects.size(); ++i) {
            delete objects[i];
        }
    }
};


int getValueG (int seed) {

        seed = abs(seed*a_RNGg+c_RNGg);
        float seed2 = seed/2147483648.0;
        return seed2;
}

float randNumG(float M, float N) {
    return M + (rand() / ( RAND_MAX / (N-M) ) );
}


beeworldgiger::beeworldgiger(QObject *parent) :
    beeworld(parent)
{
    N_ROWS = 90;//160;
    N_COLS = 120;//160;
    H_EXTENT = 260.0;//260
    V_EXTENT = 180.0;

    sphere * s = new sphere(QVector3D(0,4,4),QVector3D(2,1,1),QVector3D(45.0,0.0,0.0));
    s->setTextureGenerator(new checked());
    objects.push_back(s);

    cylinder * c = new cylinder(QVector3D(-4,4,3),QVector3D(0.5,0.5,8),QVector3D(0,90.0,0.0));
    c->setTextureGenerator(new checked());
    objects.push_back(c);

    plane * p = new plane(QVector3D(4,-100,0),QVector3D(4,200,6),QVector3D(0,0.0,0.0));
    p->setTextureGenerator(new checked());
    objects.push_back(p);

    p = new plane(QVector3D(-4,-100,0),QVector3D(4,200,6),QVector3D(0,0.0,0.0));
    p->setTextureGenerator(new checked());
    objects.push_back(p);


    scale_factor_for_one_wall = 1.01;
    contrast_factor_for_one_wall = 1.0;
    offset_for_one_wall = 0;

    t2 = 0;

    x_walls = 5;
    y_walls = 0;

    contrast = 1;
    lighting = false;
    blur = false;
    jitterSeed = 123;
    this->jitterSize = 0.0;
    setJitter(this->jitterSize);


}


QRgb convoluteG(const QList<int> &kernel, const QImage &image, int x, int y, qreal norm){
    qreal red = 0;
    qreal green = 0;
    qreal blue = 0;
    int count = 0;
    for(int r = -1 ; r<=1; ++r){
        for(int c = -1; c<=1; ++c){
            int kerVal = kernel.at(count);
            ++count;
            red += qRed(image.pixel(x+c, y+r))*kerVal;
            green += qGreen(image.pixel(x+c, y+r))*kerVal;
            blue += qBlue(image.pixel(x+c, y+r))*kerVal;
        }
    }
    return qRgb(qBound(0, qRound(red/norm), 255), qBound(0, qRound(green/norm), 255), qBound(0, qRound(blue/norm), 255));
}

/*!
 * \brief beeworldgiger::getImage
 * \param x
 * \param y
 * \param z
 * \param dir
 * \param pitch
 * \param roll
 * \param curr_t
 * \param scale
 * \return
 * Get an image for display as a scaled version of the actual image - can be used for pretty demos...
 */
QImage * beeworldgiger::getImage(float x, float y, float z, float dir, float pitch, float roll, float curr_t, float scale) {

    int vertTemp = this->N_ROWS;
    int horizTemp = this->N_COLS;

    this->N_ROWS = floor(N_ROWS*scale);
    this->N_COLS = floor(N_COLS*scale);

    QImage * im = this->getImage(x,y,z,dir,pitch,roll,curr_t);

    this->N_COLS = horizTemp;
    this->N_ROWS = vertTemp;

    return im;

}

QImage * beeworldgiger::getImage(float x, float y, float z, float dir, float pitch, float roll, float curr_t) {

    t2 += 0.0; // 0.03

    /*if (objects.size() > 0) {
        objects.back()->debugPrint();
    }*/

    if (jitterX.isEmpty()) this->setJitter(this->jitterSize);

    //bool stripes = true;

    // we add 2 onto each dimension so that we can blur and end up without edge effects
    QImage * image = new QImage(140,100,QImage::Format_RGB32);
    image->fill(Qt::white);

    // the raytracer:

    float h_stride = H_EXTENT/N_COLS;
    float v_stride = V_EXTENT/N_ROWS;

    //omp_lock_t writelock;
    //omp_init_lock(&writelock);

    // for each horizontal ommatidium col

    if (this->drawBee) {
        // add a bee object
        QVector3D scaling;
        scaling.setX(0.3);
        scaling.setY(1.3);
        scaling.setZ(0.3);
        this->drawnBeeRot.setZ(this->drawnBeeRot.z()*180/M_PI);
        sphere * bee = new sphere(this->drawnBeeLoc, scaling,this->drawnBeeRot);
        bee->setObjectName("bee");
        checked * tex = new checked();
        tex->setOffsetX(QVector <float> (0.12));
        tex->setFrequencyX(QVector <float> (0.00012));
        tex->setFrequencyY(QVector <float> (0.00012));
        tex->setMotionY(QVector <float> (0.0));
        bee->setTextureGenerator(tex);
        objects.push_back(bee);
    }

    int i = 0;
    // for each ommatidium in List for each eye
    //#pragma omp parallel for
    QVector < beethread * > threads;
    for (int eye=-1; eye < 2; eye +=2) {
        // do in blocks of 2000
        for (int start = 0; start < gdataLength; start=start+2000) {
            int end = start+2000 > gdataLength ? gdataLength : start + 2000;
            // launch thread
            threads.push_back(new beethread());
            threads.back()->starti = start;
            threads.back()->end = end;
            threads.back()->eye = eye;
            threads.back()->image = image;
            for (uint i = 0; i < this->objects.size(); ++i) {
                threads.back()->objects.push_back(objects[i]->copy());
            }
            threads.back()->dir = dir;
            threads.back()->jitterX = &jitterX;
            threads.back()->jitterY = &jitterY;
            threads.back()->roll = roll;
            threads.back()->pitch = pitch;
            threads.back()->x = x;
            threads.back()->y = y;
            threads.back()->z = z;
            threads.back()->curr_t = curr_t;
            threads.back()->lighting = lighting;
            threads.back()->start();
        }
    }

    // wait for threads
    for (uint i = 0; i < threads.size(); ++i) {
        threads[i]->wait();
        delete threads[i];
    }

    if (this->drawBee) {
        // remove bee object
        objects.pop_back();
    }

    /*QTransform myTrans;
    myTrans.rotate(180);
    *image = image->transformed(myTrans);*/

    // blur
    if (blur) {
        return image;
    } else {
        return image;
    }

}


