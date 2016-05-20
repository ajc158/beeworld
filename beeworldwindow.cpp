#include "beeworldwindow.h"
#ifndef IS_COMMANDLINE
#include "ui_beeworldwindow.h"
#endif
#include <QTimer>
#include "beeworld.h"
#include "beeworldgiger.h"
#include <QDebug>
#include "math.h"
#include <QThread>
#include <QFileDialog>
#include <QMessageBox>

// include primitives
#include"sphere.h"
#include "cylinder.h"
#include "plane.h"

// include textures
#include "checked.h"
#include "radial.h"

// for all the sockets stuff ('nix ONLY)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "spinemlnetworkserver.h"

#include <QLineEdit>

#ifndef IS_COMMANDLINE
#define PRINTDETAILS(string) ui->console->appendPlainText(string);
#else
#define PRINTDETAILS(string) QTextStream(stdout) << string << endl;
#endif


#ifndef IS_COMMANDLINE
BeeWorldWindow::BeeWorldWindow(QString file, uint port, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BeeWorldWindow)
{
    ui->setupUi(this);
#else
  BeeWorldWindow::BeeWorldWindow(QString file, uint port, QWidget *parent) :
      QObject(parent)
{
#endif

    world = new beeworldgiger;

    // resize the arrays to hold the current and initial state of the system
    propertyValues.resize(14);
    initialPropertyValues.resize(propertyValues.size());
    propertyStrings.push_back("Time");
    propertyStrings.push_back("X");
    propertyStrings.push_back("Y");
    propertyStrings.push_back("Z");
    propertyStrings.push_back("Roll");
    propertyStrings.push_back("Pitch");
    propertyStrings.push_back("Yaw");
    propertyStrings.push_back("Speed");
    propertyStrings.push_back("TransverseSpeed");
    propertyStrings.push_back("Vertical FOV");
    propertyStrings.push_back("Horizontal FOV");
    propertyStrings.push_back("Vertical Resolution");
    propertyStrings.push_back("Horizontal Resolution");
    propertyStrings.push_back("Display scaling");

    displayScale = 1.0;

#ifndef IS_COMMANDLINE
    connect(ui->actionLoad_Config, SIGNAL(triggered()), this, SLOT(loadFile()));
#endif

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(redrawImage()));
    //timer->start(0);

#ifndef IS_COMMANDLINE
    redrawTimer = new QTimer(this);
    connect(redrawTimer, SIGNAL(timeout()), this, SLOT(redrawScreen()));
    redrawTimer->start(30);
#endif

    dataSize = 1024;

    started = false;

    image = new QImage(400, 400, QImage::Format_RGB32);

    /*for (int i = 0; i < 400; ++i) {
        QRgb value;
        value = qRgb(100, 255, 100); //
        image->setPixel(190,i,value);
        image->setPixel(210,i,value);
    }*/

    int x_walls = 5;
    int y_walls = 0;

    for (int i = 0; i < 400; ++i) {
        QRgb value;
        value = qRgb(100, 255, 100); //
            if (x_walls) {
            image->setPixel(200-x_walls*5,i,value);
            image->setPixel(200+x_walls*5,i,value);
        }
        if (y_walls) {
            image->setPixel(i,200-y_walls*5,value);
            image->setPixel(i,200+y_walls*5,value);
        }
    }

    x_log.push_back(0);
    y_log.push_back(0);

    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    server->listen(QHostAddress::Any, port);

    count = 0;
    val = 0;
    counter = 0;

    dt = 0.1;

    this->standalone = false;


    if (file != "") {
        // load startup file
        this->loadFile(file);
        qDebug() << "Load CL file";
        this->standalone = true;
    } else {
#ifndef IS_COMMANDLINE
        qDebug() << "no file";
#else
        exit(-1);
#endif
    }

}

BeeWorldWindow::~BeeWorldWindow()
{
    delete world;
    #ifndef IS_COMMANDLINE
    delete ui;
    #endif
}

#ifndef IS_COMMANDLINE
void BeeWorldWindow::redrawScreen() {

    if (this->displayScale < 0.001) return;

    // render the world Hi-Rez for fun!
    QImage * im = world->getImage(propertyValues[X],propertyValues[Y],propertyValues[Z],propertyValues[YAW],propertyValues[PITCH],propertyValues[ROLL], propertyValues[TIME], this->displayScale);
    /*world->setLighting(true);
    world->drawBee = true;
    world->drawnBeeLoc.setX(propertyValues[X]);
    world->drawnBeeLoc.setY(propertyValues[Y]);
    world->drawnBeeLoc.setZ(propertyValues[Z]);
    world->drawnBeeRot.setX(propertyValues[ROLL]);
    world->drawnBeeRot.setY(propertyValues[PITCH]);
    world->drawnBeeRot.setZ(propertyValues[YAW]);
    QImage * im = world->getImage(-1,-1,13,-0.78,-1.3,0, this->propertyValues[TIME], this->displayScale);
    world->setLighting(false);
    world->drawBee = false;*/

    pix = QPixmap::fromImage(*im);
    //pix = pix.scaled(45.0,45.0/4.0*3.0,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    pix = pix.scaled(ui->view->size());

    ui->view->setPixmap(pix);
    ui->view->repaint();
    delete im;
}
#endif

void BeeWorldWindow::newConnection() {

    spineMLNetworkServer * newConn = new spineMLNetworkServer(this->server);

    // try a new connection
    if (!newConn->getClient()) {
#ifndef IS_COMMANDLINE
        PRINTDETAILS("Failed incoming connection");
#else
        QTextStream(stdout) << "Failed incoming connection" << endl;
#endif
    }

    // check sanity
    if (newConn->isSource() && newConn->size != this->world->numElements()) {
#ifndef IS_COMMANDLINE
        PRINTDETAILS("Input rejected due to incorrect size");
#else
        QTextStream(stdout) << "Input rejected due to incorrect size" << endl;
#endif
    }

    /*if (newConn->isTarget() && newConn->size != 4) {
        PRINTDETAILS("Output rejected due to incorrect size");
    }*/

    // found one!
    connections.push_back(newConn);
    if (newConn->isSource()) {
#ifndef IS_COMMANDLINE
        PRINTDETAILS("Connection found! - new input");
#else
        QTextStream(stdout) << "Connection found! - new input" << endl;
#endif
    } else {
#ifndef IS_COMMANDLINE
        PRINTDETAILS("Connection found! - new output");
#else
        QTextStream(stdout) << "Connection found! - new output" << endl;
#endif
    }

    // describe connection:
    if (newConn->dataType == ANALOG) {
#ifndef IS_COMMANDLINE
        PRINTDETAILS("Analog connection");
#else
        QTextStream(stdout) << "Analog connection" << endl;
#endif
    } else {
#ifndef IS_COMMANDLINE
        PRINTDETAILS("NOT Analog connection");
#else
        QTextStream(stdout) << "NOT Analog connection" << endl;
#endif
    }

#ifndef IS_COMMANDLINE
    PRINTDETAILS(QString("Size = ") + QString::number(newConn->size) +  QString(""));
#else
    QTextStream(stdout) << QString("Size = ") + QString::number(newConn->size) +  QString("")  << endl;
#endif

    // do we have enough connections?
    if (this->connections.size() == this->numConnections) {
        // connect up controllers
        for (int i = 0; i < this->controllers.size(); ++i) {
            for (int j = 0; j < this->connections.size(); ++j) {
                qDebug() << this->connections[j]->name << this->controllers[i].src;
                if (this->connections[j]->name == this->controllers[i].src) {
                    for (int k = 0; k < world->objectCount(); ++k) {
                        qDebug() << this->world->getObject(k)->name << this->controllers[i].dst;
                        if (this->world->getObject(k)->name == this->controllers[i].dst) {
                            this->world->getObject(k)->connectObject(this->connections[j], this->controllers[i].port);
                        }
                        if (this->world->getObject(k)->getSceneTexture()->name == this->controllers[i].dst) {
                            this->world->getObject(k)->getSceneTexture()->connectTexture(this->connections[j], this->controllers[i].port);
                        }
                    }
                    if (this->controllers[i].dst == "bee") {
                        this->connectWorld(this->connections[j], this->controllers[i].port);
                    }
                }
            }
        }
        //start the timers for the simulation
        this->timer->start(0);
        this->redrawTimer->start(15);
        srand(time(NULL));
    }

}

void BeeWorldWindow::redrawImage() {

    // only run while we have a connection
    if (connections.size() < 2) {
        this->timer->stop();
        this->redrawTimer->stop();
        return;
    }
    imdata.resize(this->world->numElements(), 0);

    // do data transfer
    this->send_data();

    QImage * im = world->getImage(propertyValues[X],propertyValues[Y],propertyValues[Z],propertyValues[YAW],propertyValues[PITCH],propertyValues[ROLL], propertyValues[TIME]);

    imdata.clear();

    for (int i = 0; i < im->height(); ++i) {
        for (int j = 0; j < im->width(); ++j) {
            QRgb col = im->pixel(j,i);
            imdata.push_back(((double) qGray(col))/255.0);
        }
    }

    delete im;
    //QPixmap pix;
    //pix = QPixmap::fromImage(*im);

    //pix = pix.scaledToHeight(160);
    //pix = pix.scaled(ui->view->size());





//    QRgb value;
//    value = qRgb(100, 100, 255); //
//    image->fill(0);
//    for (int i = 0; i < x_log.size();++i) {
//        if ((x_log.back()-x_log[i])*5+200 > 0 && (x_log.back()-x_log[i])*5+200 < 400 && (y_log.back()-y_log[i])*5+200 > 0 && (y_log.back()-y_log[i])*5+200 < 400){
//            image->setPixel((x_log.back()-x_log[i])*5+200,(y_log.back()-y_log[i])*5+200,value);
//        }
//    }

//    //x_walls = 5;
//    int y_walls = 0;

//    value = qRgb(100, 255, 100);

//    for (int i = 0; i < 400; ++i) {
//        if (x_walls) {
//            image->setPixel(x_log.back()+200-x_walls*5,i,value);
//            image->setPixel(x_log.back()+200+x_walls*5,i,value);
//        }
//        if (y_walls) {
//            image->setPixel(i,y_log.back()+200-y_walls*5,value);
//            image->setPixel(i,y_log.back()+200+y_walls*5,value);
//        }
//    }

//    pix = QPixmap::fromImage(*image);
//    ui->image->setPixmap(pix);



}


void BeeWorldWindow::connectWorld(spineMLNetworkServer * src, QString port) {
    if (port == "WallFollow") {
        PRINTDETAILS("Connected WallFollow...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(wallFollow(QVector<float>)));
    }
    if (port == "Test") {
        PRINTDETAILS("Connected Test...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(test(QVector<float>)));
    }
    if (port == "Paper1") {
        PRINTDETAILS("Connected Paper1...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(paper1(QVector<float>)));
    }
    if (port == "Paper2") {
        PRINTDETAILS("Connected Paper2...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(paper2(QVector<float>)));
    }
    if (port == "Paper3") {
        PRINTDETAILS("Connected Paper3...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(paper3(QVector<float>)));
    }
    if (port == "Paper4") {
        PRINTDETAILS("Connected Paper4...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(paper4(QVector<float>)));
    }
    if (port == "Sweep") {
        PRINTDETAILS("Connected Sweep...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(sweep(QVector<float>)));
    }
    if (port == "SideSweep") {
        PRINTDETAILS("Connected SideSweep...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(sidesweep(QVector<float>)));
    }
    if (port == "Simple") {
        PRINTDETAILS("Connected Simple...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(simple(QVector<float>)));
    }
    if (port == "Roll") {
        PRINTDETAILS("Connected Roll...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(roll(QVector<float>)));
    }
    if (port == "Yaw") {
        PRINTDETAILS("Connected Yaw...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(yaw(QVector<float>)));
    }
    if (port == "Pitch") {
        PRINTDETAILS("Connected Pitch...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(pitch(QVector<float>)));
    }
    if (port == "Optomotor") {
        PRINTDETAILS("Connected Optomotor...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(optomotor(QVector<float>)));
    }
    if (port == "InitialLocation") {
        PRINTDETAILS("Connected InitialLocation...");
        QObject::connect(src,SIGNAL(dataReceived(QVector<float>)), this, SLOT(initloc(QVector<float>)));
    }
}

void BeeWorldWindow::paper1(QVector<float> data) {

    if (val > 200) {

        //dir_change = (dataIn[0] - dataIn[2]); // left - right (I think!)
        float mag = data[0] > data[2] ? data[0] : data[2];
        float sign = (data[0] > data[2])*2-1;

        //mag = mag > 10.0 ? 1 : -1;

        //float setPoint = (dataIn[0] + dataIn[2])/2.0;
        float scale = 1.0;
        float side_setPoint = 1.0*scale;// 0.97 # 1.0 for 0 balance and 0.25 balance, 0.7 for 1.0 balance
        float height_setPoint = 0.5*scale;//1.3
        float velocity_setPoint = (2.0*side_setPoint /*+ height_setPoint*/);

        mag = mag - side_setPoint;

        dir_change = mag*sign;

        this->propertyValues[Y]-=sin(this->propertyValues[YAW])*dir_change/2.4;
        this->propertyValues[X]+=cos(this->propertyValues[YAW])*dir_change/2.4;

        // height regulation:
        this->propertyValues[Z] += (data[1]-height_setPoint)/18.0;

        // speed regulation
        this->propertyValues[V] -= ((data[0] + /*data[1] +*/ data[2]) - velocity_setPoint)/2.4; // a value of 20 ensures that we start moving before the height regulation takes us through the floor!
        //this->propertyValues[V] -= (2.0*mag - velocity_setPoint)/12.4; // a value of 20 ensures that we start moving before the height regulation takes us through the floor!

    }
    this->propertyValues[Y]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[X]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V];

}

void BeeWorldWindow::paper2(QVector<float> data) {

    if (val > 200) {

        //dir_change = (dataIn[0] - dataIn[2]); // left - right (I think!)
        float mag = data[0] > data[2] ? data[0] : data[2];
        float sign = (data[0] > data[2])*2-1;

        //mag = mag > 10.0 ? 1 : -1;

        //float setPoint = (dataIn[0] + dataIn[2])/2.0;
        float scale = 1.0;
        float side_setPoint = 0.9*scale;// 0.97 # 1.0 for 0 balance and 0.25 balance, 0.7 for 1.0 balance
        float height_setPoint = 0.3*scale;//1.3
        float velocity_setPoint = (2.0*side_setPoint /*+ height_setPoint*/);

        mag = mag - side_setPoint;

        dir_change = mag*sign;

        this->propertyValues[Y]-=sin(this->propertyValues[YAW])*dir_change/2.3;
        this->propertyValues[X]+=cos(this->propertyValues[YAW])*dir_change/2.3;

        // height regulation:
        this->propertyValues[Z] += (data[1]-height_setPoint)/30.0;

        // speed regulation
        this->propertyValues[V] -= ((data[0] + /*data[1] +*/ data[2]) - velocity_setPoint)/2.4; // a value of 20 ensures that we start moving before the height regulation takes us through the floor!
        //this->propertyValues[V] -= (2.0*mag - velocity_setPoint)/12.4; // a value of 20 ensures that we start moving before the height regulation takes us through the floor!

    }
    this->propertyValues[Y]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[X]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V];


}

void BeeWorldWindow::paper3(QVector<float> data) {

    if (val > 100) {
    }
    this->propertyValues[Y]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[X]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V];

}

void BeeWorldWindow::paper4(QVector<float> data) {

    if (val > 100) {
    }
    this->propertyValues[Y]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[X]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V];

}


void BeeWorldWindow::wallFollow(QVector<float> data) {

    if (val > 100) {

        //dir_change = (dataIn[0] - dataIn[2]); // left - right (I think!)
        float mag = data[0] > data[2] ? data[0] : data[2];
        float mag_min = data[0] < data[2] ? data[0] : data[2];
        float sign = ((data[0] < data[2])*2-1);

        //mag = mag > 10.0 ? 1 : -1;

        //float setPoint = (dataIn[0] + dataIn[2])/2.0;
        float scale = 1;
        float side_setPoint = 1.0*scale;// 0.97 # 1.0 for 0 balance and 0.25 balance, 0.7 for 1.0 balance
        float height_setPoint = 0.5*scale;//1.3
        float velocity_setPoint = (2.0*side_setPoint /*+ height_setPoint*/);

        // avoidance
        // this uses the 6th index of the data, and is based on the experimental findings
        // of the Straw lab
        float z = data[5]; // z must be -1 -> 1
        //qDebug() << "z=" << z;
        // do we have a right turn?
        // transform z with linear piecewise...
        float rate_right = z > -0.75 ? -(z-1.0)/1.75 : z > -0.9 ? -(z+0.75)*12.0+1.0 : 100;
        // now use the rate to work out the p of a turn this timestep... given that we have a timestep of 1ms and rate is /s
        float p_right = rate_right/1000.0;
        // same, but opposite linear piecewise for left...
        float rate_left = z < 0.75 ? (z+1.0)/1.75 : z < 0.9 ? (z-0.75)*12.0+1.0 : 100;
        float p_left = rate_left/1000.0;

        // now compare these values to rand() and trigger saccade
        float r_right = rand() / (RAND_MAX + 1.);
        float r_left = rand() / (RAND_MAX + 1.);

        if (r_right < p_right) {
            this->propertyValues[YAW] += 0.3 + (rand() / (RAND_MAX + 1.))*0.1;
        }
        if (r_left < p_left) {
            this->propertyValues[YAW] -= 0.3 + (rand() / (RAND_MAX + 1.))*0.1;
        }

        mag = mag - side_setPoint;

        dir_change = mag*sign/1.0;

        this->propertyValues[Y]-=sin(this->propertyValues[YAW])*dir_change/1.0;
        this->propertyValues[X]+=cos(this->propertyValues[YAW])*dir_change/1.0;

        // height regulation:
        //this->propertyValues[Z] += (data[1]-height_setPoint)/1200.0;

        // speed regulation
        this->propertyValues[V] -= ((data[0] + /*data[1] +*/ data[2]) - velocity_setPoint)/4.0; // a value of 20 ensures that we start moving before the height regulation takes us through the floor!
        // decay to 30...
        this->propertyValues[V] -= (this->propertyValues[V] - 60.0)/3.0;
        //qDebug() << this->propertyValues[V] << " " << this->propertyValues[X] << " " << (data[0] + /*data[1] +*/ data[2]);
        //this->propertyValues[V] = 100;

    }
    //this->propertyValues[YAW] += 0.001;
    this->propertyValues[Y]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[X]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V];

}

void BeeWorldWindow::test(QVector<float> data) {

    if (val > 100) {

        float velocity_setPoint = 1.3;

        float avoid_right_scale = 1.0;
        float avoid_left_scale = 1.0;

        // angled avoidance
        // this uses the 6th index of the data, and is based on the experimental findings
        // of the Straw lab
        float z_orig = data[5]; // z must be -1 -> 1
        //qDebug() << "z=" << z;
        // do we have a right turn?
        // transform z with linear piecewise...
        float z = z_orig * avoid_right_scale;
        float rate_right = z > -0.75 ? -(z-1.0)/1.75 : z > -0.9 ? -(z+0.75)*12.0+1.0 : 100;
        // now use the rate to work out the p of a turn this timestep... given that we have a timestep of 1ms and rate is /s
        float p_right = rate_right/1000.0;
        // same, but opposite linear piecewise for left...
        z = z_orig * avoid_left_scale;
        float rate_left = z < 0.75 ? (z+1.0)/1.75 : z < 0.9 ? (z-0.75)*12.0+1.0 : 100;
        float p_left = rate_left/1000.0;

        // head on collision avoidance
        z = data[6];
        float rate = z < 0.75 ? (z+1.0)/1.75 : z < 0.9 ? (z-0.75)*12.0+1.0 : 100;
        float p_headon = rate_left/1000.0;

        // now compare these values to rand() and trigger saccade
        float r_right = rand() / (RAND_MAX + 1.);
        float r_left = rand() / (RAND_MAX + 1.);
        float r_headon = rand() / (RAND_MAX + 1.);

        this->propertyValues[YAW] += data[8]-data[9];

        this->propertyValues[YAW] += data[10]-data[11];

        if (r_right < p_right) {
            //this->propertyValues[YAW] += 0.2 + (rand() / (RAND_MAX + 1.))*0.1;
        }
        if (r_left < p_left) {
            //this->propertyValues[YAW] -= 0.2 + (rand() / (RAND_MAX + 1.))*0.1;
        }
        if (r_headon < p_headon) {
            if (rand() / (RAND_MAX + 1.) > 0.5) {
                //this->propertyValues[YAW] += 0.4 + (rand() / (RAND_MAX + 1.))*0.1;
            } else {
                //this->propertyValues[YAW] -= 0.4 + (rand() / (RAND_MAX + 1.))*0.1;
            }
        }
        // speed regulation
        //this->propertyValues[V] -= (data[7] - velocity_setPoint)*1.0; // a value of 20 ensures that we start moving before the height regulation takes us through the floor!
        // decay to 30...
        this->propertyValues[V] -= (this->propertyValues[V] - 60.0)/80.0;
        //qDebug() << this->propertyValues[V] << " " << this->propertyValues[X] << " " << (data[0] + /*data[1] +*/ data[2]);
        //this->propertyValues[V] = 100;

    }
    //this->propertyValues[YAW] += 0.001;
    this->propertyValues[Y]+=cos(this->propertyValues[YAW])*0.001*this->propertyValues[V];
    this->propertyValues[X]-=sin(this->propertyValues[YAW])*0.001*this->propertyValues[V];

}


void BeeWorldWindow::sweep(QVector<float> data) {

    this->propertyValues[V] = this->propertyValues[V] * 1.001;

    // move the bee according to the velocity
    this->propertyValues[Y]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[X]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[Y]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V_TRANS];
    this->propertyValues[X]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V_TRANS];

}

void BeeWorldWindow::sidesweep(QVector<float> data) {

    this->propertyValues[V_TRANS] = this->propertyValues[V_TRANS] * 1.001;

    // move the bee according to the velocity
    this->propertyValues[Y]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[X]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[Y]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V_TRANS];
    this->propertyValues[X]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V_TRANS];

}

void BeeWorldWindow::simple(QVector<float> data) {

    qDebug() << "SIMPLE...";

    this->propertyValues[V] = data[0];

    // move the bee according to the velocity
    this->propertyValues[Y]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[X]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V];
    this->propertyValues[Y]-=sin(this->propertyValues[YAW])*0.003*this->propertyValues[V_TRANS];
    this->propertyValues[X]+=cos(this->propertyValues[YAW])*0.003*this->propertyValues[V_TRANS];

}

void BeeWorldWindow::roll(QVector<float> data) {

    if (data.size() > 0) {
        this->propertyValues[ROLL] = data[0]/180*M_PI;
    }

}

void BeeWorldWindow::yaw(QVector<float> data) {

    if (data.size() > 0) {
        this->propertyValues[YAW] = data[0]/180*M_PI;
    }

}


void BeeWorldWindow::pitch(QVector<float> data) {

    if (data.size() > 0) {
        this->propertyValues[PITCH] = data[0]/180*M_PI;
    }

}


void BeeWorldWindow::optomotor(QVector<float> data) {

    if (data.size() == 2) {
        //this->propertyValues[YAW] -= (data[0]-data[1])*0.05;
    }

}

void BeeWorldWindow::initloc(QVector<float> data) {

    if (data.size() == 3 && val < 3) {
        this->propertyValues[X] = data[0];
        //this->propertyValues[Y] = data[1];
        //this->propertyValues[Z] = data[2];
    }

}


void BeeWorldWindow::send_data() {

    ++val;
    this->propertyValues[TIME] += dt; // update time in ms

    // service the connections
    bool haveIn = false;
    bool haveOut = false;

    // only a source
    if (connections.size() == 1) {
        haveIn = true;
    }

    QVector < double > dataIn;

    while (1) {
        // loop until we do both transactions
        counter++;
        if (counter > 40) {
            // we have disconnected - reset
            for (int i = 0; i < this->connections.size(); ++i) {
                delete connections[i];
            }
            connections.clear();
            this->counter = 0;
            PRINTDETAILS("Connection broken");
            PRINTDETAILS("v = " + QString::number(this->propertyValues[V]));
            val = 0;
            // reset beeworld
            this->applyInitialValues();
            // write out logs if needed and clear log structures
            // write out
            if (this->logPath != "") {
                QFile output;
                output.setFileName(this->logPath + "/beelog" + QDateTime::currentDateTime().toString("ddMMyy_hh_mm_ss") + ".csv");
                output.open(QFile::WriteOnly);
                QTextStream ts_out(&output);
                for (int i = 0; i < this->propertyStrings.size(); ++i) {
                    ts_out << this->propertyStrings[i];
                    if (i != this->propertyStrings.size()-1) {
                        ts_out << ",";
                    }
                }
                ts_out << "\n";
                for (int i = 0; i < this->log.size(); ++i) {
                    for (int j = 0; j < this->log[i].size(); ++j) {
                        ts_out << QString::number(this->log[i][j]);
                        if (j != this->log[i].size()-1) {
                            ts_out << ",";
                        }
                    }
                    if (i != this->log.size()-1) {
                        ts_out << "\n";
                    }
                }
            }
            // clear
            this->log.clear();
            if (this->standalone) qApp->exit();
            return;
        }
        for (int i = 0; i < this->connections.size(); ++i) {
            // see what connection type we have
            if (this->connections[i]->isSource()) {

                // are we waiting on a response?
                if (this->connections[i]->sendDataConfirm()) {
                    // send
                    this->connections[i]->sendData((char *) &imdata[0],this->connections[i]->size);
                    haveOut = true;
                }

                // TODO: check for errors

            }
            if (this->connections[i]->isTarget()) {

                dataIn.resize(this->connections[i]->size);
                if (this->connections[i]->recvData((char *) &dataIn[0], this->connections[i]->size)) {
                    // Woo got some data!
                    haveIn = true;
                }

            }
        }

        // we have taken up the lag
        if (haveIn && haveOut) {
            counter = 0;
            break;
        } else {
            // wait a little bit - we may be faster than the model!
            usleep(10);
        }
    }


    // add values to log
    log.push_back(this->propertyValues);

}

#ifndef IS_COMMANDLINE
bool BeeWorldWindow::loadFile() {

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose the config file to load"),
                                                    qgetenv("HOME"),
                                                    tr("Config files (*.xml);; All files (*)"));
    return loadFile(fileName);
}
#endif

bool BeeWorldWindow::loadFile(QString fileName) {

    QFile file( fileName );
    if( !file.open( QIODevice::ReadOnly ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open the world file");
        msgBox.exec();
        return false;}

    // reset everything
    controllers.clear();
    world->clearObjects();
    this->logPath = "";

    // get a streamreader
    QXmlStreamReader * reader = new QXmlStreamReader;

    // set the stream reader device to the file
    reader->setDevice( &file );

    // read elements
    while (reader->readNextStartElement()) {

        if (reader->name() == "BeeworldConfig" || reader->name() == "BeeworldConfigOld") {

            if (reader->name() == "BeeworldConfigOld") {
                delete this->world;
                this->world = new beeworld;
                world->clearObjects();
            }

            while (reader->readNextStartElement()) {

                if (reader->name() == "BeeEye") {

                    while (reader->readNextStartElement()) {

                        if (reader->name() == "Vertical") {

                            if (reader->attributes().hasAttribute("FOV"))
                                initialPropertyValues[FOV_V] = reader->attributes().value("FOV").toString().toFloat();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'FOV' in tag Vertical");
                                settings.endArray();
                            }
                            if (reader->attributes().hasAttribute("Resolution"))
                                initialPropertyValues[REZ_V] = reader->attributes().value("Resolution").toString().toFloat();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'Resolution' in tag Vertical");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else if (reader->name() == "Horizontal") {

                            if (reader->attributes().hasAttribute("FOV"))
                                initialPropertyValues[FOV_H] = reader->attributes().value("FOV").toString().toFloat();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'FOV' in tag Horizontal");
                                settings.endArray();
                            }
                            if (reader->attributes().hasAttribute("Resolution"))
                                initialPropertyValues[REZ_H] = reader->attributes().value("Resolution").toString().toFloat();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'Resolution' in tag Horizontal");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else {
                            QSettings settings;
                            int num_errs = settings.beginReadArray("errors");
                            settings.endArray();
                            settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText", "XML Error in World File - unknown tag '" + reader->name().toString() + "'");
                            settings.endArray();
                        }
                    }
                } else if (reader->name() == "BeeInitialState") {

                    while (reader->readNextStartElement()) {

                        // for each of the bee initial state properties (which all have the same format) - this is a little dodgy as the alteration of
                        // the enum of properties can bork it unless the QVector is also adjusted
                        // need a bool to check if a given tag is found
                        bool notFound = true;
                        for (int i = 0; i < this->propertyStrings.size(); ++i) {
                            if (reader->name() == propertyStrings[i]) {
                                notFound = false;
                                if (reader->attributes().hasAttribute("value"))
                                    initialPropertyValues[i/* It is vital this lines up with the enum */] = reader->attributes().value("value").toString().toFloat();
                                else {
                                    QSettings settings;
                                    int num_errs = settings.beginReadArray("errors");
                                    settings.endArray();
                                    settings.beginWriteArray("errors");
                                        settings.setArrayIndex(num_errs + 1);
                                        settings.setValue("errorText", "XML Error in World File - missing attribute 'value' in tag " + propertyStrings[i]);
                                    settings.endArray();
                                }
                                reader->skipCurrentElement();
                            }
                        }
                        if (notFound) {
                            QSettings settings;
                            int num_errs = settings.beginReadArray("errors");
                            settings.endArray();
                            settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText", "XML Error in World File - unknown tag '" + reader->name().toString() + "'");
                            settings.endArray();
                        }
                    }
                } else if (reader->name() == "Controllers") {
                    while (reader->readNextStartElement()) {

                        if (reader->name() == "Controller") {

                            QString src;
                            QString dst;
                            QString port;
                            if (reader->attributes().hasAttribute("source"))
                                src = reader->attributes().value("source").toString();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'source' in tag Controller");
                                settings.endArray();
                            }
                            if (reader->attributes().hasAttribute("target"))
                                dst = reader->attributes().value("target").toString();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'target' in tag Controller");
                                settings.endArray();
                            }
                            if (reader->attributes().hasAttribute("port"))
                                port = reader->attributes().value("port").toString();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'port' in tag Controller");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                            // add controller
                            controller newController(src,dst,port);
                            this->controllers.push_back(newController);

                        } else {
                            QSettings settings;
                            int num_errs = settings.beginReadArray("errors");
                            settings.endArray();
                            settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText", "XML Error in World File - unknown tag '" + reader->name().toString() + "'");
                            settings.endArray();
                        }
                    }
                } else if (reader->name() == "World") {
                    while (reader->readNextStartElement()) {

                        QString tagName = reader->name().toString();

                        if (tagName == "Plane" || tagName == "Cylinder" || tagName == "Sphere") {

                            QVector3D location;
                            QVector3D rotation;
                            QVector3D scaling;
                            QString name;
                            sceneTexture * tex = NULL;

                            // extract the object name
                            if (reader->attributes().hasAttribute("name"))
                                name = reader->attributes().value("name").toString();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'name' in tag " + tagName);
                                settings.endArray();
                            }

                            while (reader->readNextStartElement()) {

                                if (reader->name() == "Location") {
                                    if (reader->attributes().hasAttribute("x"))
                                        location.setX(reader->attributes().value("x").toString().toFloat());
                                    else {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'x' in tag Location");
                                        settings.endArray();
                                    }
                                    if (reader->attributes().hasAttribute("y"))
                                        location.setY(reader->attributes().value("y").toString().toFloat());
                                    else {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'y' in tag Location");
                                        settings.endArray();
                                    }
                                    if (reader->attributes().hasAttribute("z"))
                                        location.setZ(reader->attributes().value("z").toString().toFloat());
                                    else {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'z' in tag Location");
                                        settings.endArray();
                                    }
                                    reader->skipCurrentElement();
                                } else if (reader->name() == "Rotation") {
                                    if (reader->attributes().hasAttribute("xAxis"))
                                        rotation.setX(reader->attributes().value("xAxis").toString().toFloat());
                                    else {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'xAxis' in tag Rotation");
                                        settings.endArray();
                                    }
                                    if (reader->attributes().hasAttribute("yAxis"))
                                        rotation.setY(reader->attributes().value("yAxis").toString().toFloat());
                                    else {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'yAxis' in tag Rotation");
                                        settings.endArray();
                                    }
                                    if (reader->attributes().hasAttribute("zAxis"))
                                        rotation.setZ(reader->attributes().value("zAxis").toString().toFloat());
                                    else {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'zAxis' in tag Rotation");
                                        settings.endArray();
                                    }
                                    reader->skipCurrentElement();
                                } else if (reader->name() == "Scaling") {
                                    if (reader->attributes().hasAttribute("x"))
                                        scaling.setX(reader->attributes().value("x").toString().toFloat());
                                    else {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'x' in tag Scaling");
                                        settings.endArray();
                                    }
                                    if (reader->attributes().hasAttribute("y"))
                                        scaling.setY(reader->attributes().value("y").toString().toFloat());
                                    else {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'y' in tag Scaling");
                                        settings.endArray();
                                    }
                                    if (reader->attributes().hasAttribute("z"))
                                        scaling.setZ(reader->attributes().value("z").toString().toFloat());
                                    else {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'z' in tag Scaling");
                                        settings.endArray();
                                    }
                                    reader->skipCurrentElement();
                                } else if (reader->name() == "Texture") {
                                    // read in the texture

                                    while (reader->readNextStartElement()) {

                                        if (reader->name() == "Checked") {

                                            tex = new checked();

                                            // get the name
                                            // extract the object name
                                            if (reader->attributes().hasAttribute("name"))
                                                tex->name = reader->attributes().value("name").toString();
                                            else {
                                                QSettings settings;
                                                int num_errs = settings.beginReadArray("errors");
                                                settings.endArray();
                                                settings.beginWriteArray("errors");
                                                    settings.setArrayIndex(num_errs + 1);
                                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'name' in tag Checked");
                                                settings.endArray();
                                            }

                                            // properties:

                                            while (reader->readNextStartElement()) {

                                                QVector <float> freq;

                                                if (reader->name() == "Frequency") {
                                                    if (reader->attributes().hasAttribute("x"))
                                                        freq.push_back(reader->attributes().value("x").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'x' in tag Frequency");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("y"))
                                                        freq.push_back(reader->attributes().value("y").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'y' in tag Frequency");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("z"))
                                                        freq.push_back(reader->attributes().value("z").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'z' in tag Frequency");
                                                        settings.endArray();
                                                    }
                                                    ((checked *) tex)->setFrequency(freq);
                                                    reader->skipCurrentElement();
                                                } else if (reader->name() == "Offset") {
                                                    QVector <float> offset;
                                                    if (reader->attributes().hasAttribute("x"))
                                                        offset.push_back(reader->attributes().value("x").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'x' in tag Offset");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("y"))
                                                        offset.push_back(reader->attributes().value("y").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'y' in tag Offset");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("z"))
                                                        offset.push_back(reader->attributes().value("z").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'z' in tag Offset");
                                                        settings.endArray();
                                                    }
                                                    tex->setOffset(offset);
                                                    reader->skipCurrentElement();
                                                } else if (reader->name() == "Motion") {
                                                    QVector <float> motion;
                                                    if (reader->attributes().hasAttribute("x"))
                                                        motion.push_back(reader->attributes().value("x").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'x' in tag Motion");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("y"))
                                                        motion.push_back(reader->attributes().value("y").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'y' in tag Motion");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("z"))
                                                        motion.push_back(reader->attributes().value("z").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'z' in tag Motion");
                                                        settings.endArray();
                                                    }
                                                    tex->setMotion(motion);
                                                    reader->skipCurrentElement();
                                                } else if (reader->name() == "LightCol") {
                                                    QVector <float> col;
                                                    if (reader->attributes().hasAttribute("r"))
                                                        col.push_back(reader->attributes().value("r").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'r' in tag LightCol");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("g"))
                                                        col.push_back(reader->attributes().value("g").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'g' in tag LightCol");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("b"))
                                                        col.push_back(reader->attributes().value("b").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'b' in tag LightCol");
                                                        settings.endArray();
                                                    }
                                                    ((checked *) tex)->setLightCol(col);
                                                    reader->skipCurrentElement();
                                                }  else if (reader->name() == "DarkCol") {
                                                    QVector <float> col;
                                                    if (reader->attributes().hasAttribute("r"))
                                                        col.push_back(reader->attributes().value("r").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'r' in tag DarkCol");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("g"))
                                                        col.push_back(reader->attributes().value("g").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'g' in tag DarkCol");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("b"))
                                                        col.push_back(reader->attributes().value("b").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'b' in tag DarkCol");
                                                        settings.endArray();
                                                    }
                                                    ((checked *) tex)->setDarkCol(col);
                                                    reader->skipCurrentElement();
                                                } else if (reader->name() == "isSine") {
                                                    QVector <float> temp;
                                                    if (reader->attributes().hasAttribute("value"))
                                                        temp.push_back(reader->attributes().value("value").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'value' in tag isSine");
                                                        settings.endArray();
                                                    }
                                                    ((checked *) tex)->setIsSine(temp);
                                                    reader->skipCurrentElement();
                                                } else {
                                                    QSettings settings;
                                                    int num_errs = settings.beginReadArray("errors");
                                                    settings.endArray();
                                                    settings.beginWriteArray("errors");
                                                        settings.setArrayIndex(num_errs + 1);
                                                        settings.setValue("errorText", "XML Error in World File - unknown tag '" + reader->name().toString() + "'");
                                                    settings.endArray();
                                                }

                                            }

                                        } else if (reader->name() == "Radial") {

                                            tex = new radial();

                                            // get the name
                                            // extract the object name
                                            if (reader->attributes().hasAttribute("name"))
                                                tex->name = reader->attributes().value("name").toString();
                                            else {
                                                QSettings settings;
                                                int num_errs = settings.beginReadArray("errors");
                                                settings.endArray();
                                                settings.beginWriteArray("errors");
                                                    settings.setArrayIndex(num_errs + 1);
                                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'name' in tag Radial");
                                                settings.endArray();
                                            }

                                            // properties:

                                            while (reader->readNextStartElement()) {

                                                QVector <float> freq;
                                                QVector <float> rOff;
                                                QVector <float> rMtn;

                                                if (reader->name() == "Frequency") {
                                                    if (reader->attributes().hasAttribute("value"))
                                                        freq.push_back(reader->attributes().value("value").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'value'' in tag Frequency");
                                                        settings.endArray();
                                                    }
                                                    ((radial *) tex)->setFrequency(freq);
                                                    reader->skipCurrentElement();
                                                } else if (reader->name() == "RadialOffset") {
                                                    if (reader->attributes().hasAttribute("value"))
                                                        rOff.push_back(reader->attributes().value("value").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'value'' in tag RadialOffset");
                                                        settings.endArray();
                                                    }
                                                    ((radial *) tex)->setRadialOffset(rOff);
                                                    reader->skipCurrentElement();
                                                } else if (reader->name() == "RadialMotion") {
                                                    if (reader->attributes().hasAttribute("value"))
                                                        rMtn.push_back(reader->attributes().value("value").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'value'' in tag RadialOffset");
                                                        settings.endArray();
                                                    }
                                                    ((radial *) tex)->setRadialMotion(rMtn);
                                                    reader->skipCurrentElement();
                                                } else if (reader->name() == "Offset") {
                                                    QVector <float> offset;
                                                    if (reader->attributes().hasAttribute("x"))
                                                        offset.push_back(reader->attributes().value("x").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'x' in tag Offset");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("y"))
                                                        offset.push_back(reader->attributes().value("y").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'y' in tag Offset");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("z"))
                                                        offset.push_back(reader->attributes().value("z").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'z' in tag Offset");
                                                        settings.endArray();
                                                    }
                                                    tex->setOffset(offset);
                                                    reader->skipCurrentElement();
                                                } else if (reader->name() == "Motion") {
                                                    QVector <float> motion;
                                                    if (reader->attributes().hasAttribute("x"))
                                                        motion.push_back(reader->attributes().value("x").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'x' in tag Motion");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("y"))
                                                        motion.push_back(reader->attributes().value("y").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'y' in tag Motion");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("z"))
                                                        motion.push_back(reader->attributes().value("z").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'z' in tag Motion");
                                                        settings.endArray();
                                                    }
                                                    tex->setMotion(motion);
                                                    reader->skipCurrentElement();
                                                } else if (reader->name() == "LightCol") {
                                                    QVector <float> col;
                                                    if (reader->attributes().hasAttribute("r"))
                                                        col.push_back(reader->attributes().value("r").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'r' in tag LightCol");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("g"))
                                                        col.push_back(reader->attributes().value("g").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'g' in tag LightCol");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("b"))
                                                        col.push_back(reader->attributes().value("b").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'b' in tag LightCol");
                                                        settings.endArray();
                                                    }
                                                    ((checked *) tex)->setLightCol(col);
                                                    reader->skipCurrentElement();
                                                }  else if (reader->name() == "DarkCol") {
                                                    QVector <float> col;
                                                    if (reader->attributes().hasAttribute("r"))
                                                        col.push_back(reader->attributes().value("r").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'r' in tag DarkCol");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("g"))
                                                        col.push_back(reader->attributes().value("g").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'g' in tag DarkCol");
                                                        settings.endArray();
                                                    }
                                                    if (reader->attributes().hasAttribute("b"))
                                                        col.push_back(reader->attributes().value("b").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'b' in tag DarkCol");
                                                        settings.endArray();
                                                    }
                                                    ((checked *) tex)->setDarkCol(col);
                                                    reader->skipCurrentElement();
                                                } else if (reader->name() == "isSine") {
                                                    QVector <float> val;
                                                    if (reader->attributes().hasAttribute("value"))
                                                        val.push_back(reader->attributes().value("value").toString().toFloat());
                                                    else {
                                                        QSettings settings;
                                                        int num_errs = settings.beginReadArray("errors");
                                                        settings.endArray();
                                                        settings.beginWriteArray("errors");
                                                            settings.setArrayIndex(num_errs + 1);
                                                            settings.setValue("errorText", "XML Error in World File - missing attribute 'value'' in tag isSine");
                                                        settings.endArray();
                                                    }
                                                    ((radial *) tex)->setSine(val);
                                                    reader->skipCurrentElement();
                                                } else {
                                                    QSettings settings;
                                                    int num_errs = settings.beginReadArray("errors");
                                                    settings.endArray();
                                                    settings.beginWriteArray("errors");
                                                        settings.setArrayIndex(num_errs + 1);
                                                        settings.setValue("errorText", "XML Error in World File - unknown tag '" + reader->name().toString() + "'");
                                                    settings.endArray();
                                                }

                                            }

                                        } else {
                                            QSettings settings;
                                            int num_errs = settings.beginReadArray("errors");
                                            settings.endArray();
                                            settings.beginWriteArray("errors");
                                                settings.setArrayIndex(num_errs + 1);
                                                settings.setValue("errorText", "XML Error in World File - unknown tag '" + reader->name().toString() + "'");
                                            settings.endArray();
                                        }

                                    }

                                }
                            }

                            // add the object to the world
                            sceneObject * object;

                            if (tagName == "Plane") {
                                object = new plane(location, scaling, rotation);
                            }
                            if (tagName == "Cylinder") {
                                object = new cylinder(location, scaling, rotation);
                            }
                            if (tagName == "Sphere") {
                                object = new sphere(location, scaling, rotation);
                            }
                            object->name = name;
                            object->setTextureGenerator(tex);
                            world->addObject(object);

                        } else {
                            QSettings settings;
                            int num_errs = settings.beginReadArray("errors");
                            settings.endArray();
                            settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText", "XML Error in World File - unknown tag '" + reader->name().toString() + "'");
                            settings.endArray();
                        }

                    }
                } else if (reader->name() == "System") {
                    while (reader->readNextStartElement()) {

                        if (reader->name() == "NumConnections") {

                            if (reader->attributes().hasAttribute("value"))
                                this->numConnections = reader->attributes().value("value").toString().toInt();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'value' in tag NumConnections");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else if (reader->name() == "DisplayScaling") {

                            if (reader->attributes().hasAttribute("value"))
                                this->displayScale = reader->attributes().value("value").toString().toFloat();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'value' in tag DisplayScaling");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else if (reader->name() == "Blur") {

                            if (reader->attributes().hasAttribute("value")) {
                                this->world->setBlur(reader->attributes().value("value").toString() == "true");
                            }
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'value' in tag Blur");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else if (reader->name() == "Lighting") {

                            if (reader->attributes().hasAttribute("value"))
                                this->world->setLighting(reader->attributes().value("value").toString() == "true");
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'value' in tag Lighting");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else if (reader->name() == "Log") {

                            if (reader->attributes().hasAttribute("path"))
                                this->logPath = reader->attributes().value("path").toString();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'path' in tag Log");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else if (reader->name() == "DT") {

                            if (reader->attributes().hasAttribute("value"))
                                this->dt = reader->attributes().value("value").toString().toDouble();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in World File - missing attribute 'value' in tag DT");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else {
                            QSettings settings;
                            int num_errs = settings.beginReadArray("errors");
                            settings.endArray();
                            settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText", "XML Error in World File - unknown tag '" + reader->name().toString() + "'");
                            settings.endArray();
                        }
                    }
                } else {
                    QSettings settings;
                    int num_errs = settings.beginReadArray("errors");
                    settings.endArray();
                    settings.beginWriteArray("errors");
                        settings.setArrayIndex(num_errs + 1);
                        settings.setValue("errorText", "XML Error in Project File - unknown tag '" + reader->name().toString() + "'");
                    settings.endArray();
                }

            }
        }  else {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML Error in Project File - incorrect start tag");
            settings.endArray();
        }

    }


    // evaluate errors...
    if (this->printErrors("Errors loading config file")) {
        return false;
    } else {
        // apply initial values
        this->applyInitialValues();
        return true;
    }
}

void BeeWorldWindow::applyInitialValues() {

    this->propertyValues = this->initialPropertyValues;

    this->world->setFOV(initialPropertyValues[FOV_V], initialPropertyValues[FOV_H]);
    this->world->setWorldSize(initialPropertyValues[REZ_V], initialPropertyValues[REZ_H]);

#ifndef IS_COMMANDLINE
    this->redrawScreen();
#endif

}


bool BeeWorldWindow::printErrors(QString title) {

    QString errors;

    // collate errors:
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();

    if (num_errs != 0) {

        // list errors
        settings.beginReadArray("errors");
        for (int j = 1; j < num_errs; ++j) {
            settings.setArrayIndex(j);
            errors = errors + settings.value("errorText", "").toString();
            errors = errors + "<br/>";
        }
        settings.endArray();

        // clear errors
        settings.remove("errors");

    } else {
        return false;
    }

    // display errors:
    if (!errors.isEmpty()) {
        // display errors
        QMessageBox msgBox;
        msgBox.setText("<P><b>" + title + "</b></P>" + errors);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }

    return true;

}
