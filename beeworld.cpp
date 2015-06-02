#include "beeworld.h"
#include "math.h"
#include <QDebug>

// include primitives
#include"sphere.h"
#include "cylinder.h"
#include "plane.h"

// include textures
#include "checked.h"
//#include <omp.h>

int a_RNG = 1103515245;
int c_RNG = 12345;

#define ONE_OVER_FLOOR_SIZE 0.5
//1.5

int getValue (int seed) {

        seed = abs(seed*a_RNG+c_RNG);
        float seed2 = seed/2147483648.0;
        return seed2;
}

float randNum(float M, float N) {
    return M + (rand() / ( RAND_MAX / (N-M) ) );
}


beeworld::beeworld(QObject *parent) :
    QObject(parent)
{
    N_ROWS = 120;//160;
    N_COLS = 90;//160;
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
    setJitter();

}

void beeworld::setJitter() {
    srand(this->jitterSeed);
    // set jitters
    jitterY.clear();
    jitterX.clear();
    for (int i = 0; i < this->N_ROWS*3; ++i) {
        for (int j = 0; j < this->N_COLS*3; ++j) {
            jitterX.push_back(randNum(-0.5,0.5));
            jitterY.push_back(randNum(-0.5,0.5));
        }
    }
    qDebug() << "Jitter Set";
}



void beeworld::clearObjects() {
    this->objects.clear();
}

void beeworld::addObject(sceneObject *object) {
    this->objects.push_back(object);
}

sceneObject * beeworld::getObject(int i) {
    return this->objects[i];
}

int beeworld::objectCount() {
    return this->objects.size();
}

QRgb convolute(const QList<int> &kernel, const QImage &image, int x, int y, qreal norm){
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
 * \brief beeworld::getImage
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
QImage * beeworld::getImage(float x, float y, float z, float dir, float pitch, float roll, float curr_t, float scale) {

    int vertTemp = this->N_ROWS;
    int horizTemp = this->N_COLS;

    this->N_ROWS = floor(N_ROWS*scale);
    this->N_COLS = floor(N_COLS*scale);

    QImage * im = this->getImage(x,y,z,dir,pitch,roll,curr_t);

    this->N_COLS = horizTemp;
    this->N_ROWS = vertTemp;

    return im;

}

QImage * beeworld::getImage(float x, float y, float z, float dir, float pitch, float roll, float curr_t) {

    t2 += 0.0; // 0.03

    /*if (objects.size() > 0) {
        objects.back()->debugPrint();
    }*/

    if (jitterX.isEmpty()) this->setJitter();

    //bool stripes = true;

    // we add 2 onto each dimension so that we can blur and end up without edge effects
    QImage * image = new QImage(N_COLS+2*blur,N_ROWS+2*blur,QImage::Format_RGB32);
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
    for (float h = -(N_COLS/2+blur); h < N_COLS/2+blur; ++h) {
        // for each ommatidium in current col
        #pragma omp parallel for
        for (int v = -(N_ROWS/2+blur); v < N_ROWS/2+blur; ++v) {

            // we have higher rez vertically than horizontally, higher rez at the front than the sides and top

            if (i > jitterY.size()-1)
                qDebug() << jitterY.size() << " " << i << " " << N_ROWS << " " << blur;

            float sign = -((v < 0) - 0.5)*2;
            float v_ang = sign*pow(fabs(v_stride * v), 1.3)/pow(V_EXTENT*0.5, 1.3)*V_EXTENT*0.5 + jitterY[i]; /* pow gives higher rez at centre */
            sign = -((h < 0) - 0.5)*2;
            float h_ang = sign*pow(fabs(h_stride * h), 1.3)/pow(H_EXTENT*0.5, 1.3)*H_EXTENT*0.5 + jitterX[i];
            // low rez at top
            h_ang /*+= ((h_ang > 0) - (h_ang < 0))*/ /= cos(v_ang/180.0*M_PI); //0.000385

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
            //omp_set_lock(&writelock);
            for (int k = 0; k < objects.size(); ++k) {
                float t;
                bool isHit = objects[k]->isHit(QVector3D(x,y,z), QVector3D(x_vect,y_vect,z_vect), t);
                if (isHit && t < min_t) {
                    min_t = t;
                    min_ind = k;
                }
            }


            if (min_ind != -1) {
                image->setPixel((int) (h + N_COLS/2+blur), (int) (v+N_ROWS/2+blur),objects[min_ind]->getTexture(curr_t, this->lighting).rgb());
            } else if (z_vect < 0) {
                // generate the procedural floor for use in optic flow
                int valx = abs(int((-z/z_vect * (x_vect) + x) * ONE_OVER_FLOOR_SIZE));
                int valy = abs(int((-z/z_vect * (y_vect) + y) * ONE_OVER_FLOOR_SIZE));
                int val = valx*valx + valy*valy;
                val = abs(val * a_RNG + c_RNG);
                //val = 100;
                val = val % 255;
                image->setPixel((int) (h + N_COLS/2+blur), (int) (v+N_ROWS/2+blur),QColor(val, val, val).rgb());
            }
            //omp_unset_lock(&writelock);
            ++i;
        }
    }

    if (this->drawBee) {
        // remove bee object
        objects.pop_back();
    }

    // blur
    if (blur) {
        QList<int> kern;
        QImage * temp = new QImage(N_COLS, N_ROWS, image->format());

        kern << 1 << 2 << 1 \
             << 2 << 3 << 2 \
             << 1 << 2 << 1;

        for (int r = 1; r < image->height()-1; r++){
            for (int c = 1; c<image->width()-1; c++){
                temp->setPixel(c-1, r-1, convolute(kern, *image, c, r, 15.0));
            }
        }

        return temp;
    } else {
        return image;
    }

}

void beeworld::setWorldSize(int size) {
    N_ROWS = size;
    N_COLS = round(size*4./3.);
    this->setJitter();
}

void beeworld::setWorldSize(int vert, int horiz) {
    N_ROWS = vert;
    N_COLS = horiz;
    this->setJitter();
}

int beeworld::numElements() {
    return N_ROWS * N_COLS;
}

void beeworld::setFOVScale(double fov_scale) {
    H_EXTENT = 260.0*fov_scale;//260
    V_EXTENT = 180.0*fov_scale;
}

void beeworld::setFOV(float vert, float horiz) {
    H_EXTENT = horiz;
    V_EXTENT = vert;
}

void beeworld::setLighting(bool onOff) {
    lighting = onOff;
}

void beeworld::setBlur(bool onOff) {
    blur = onOff;
}

