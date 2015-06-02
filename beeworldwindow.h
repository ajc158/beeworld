#ifndef BEEWORLDWINDOW_H
#define BEEWORLDWINDOW_H

#include <QMainWindow>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtCore>
#include <QSettings>

class beeworld;
class spineMLNetworkServer;

enum propertyValue {
    TIME,
    X,
    Y,
    Z,
    ROLL,
    PITCH,
    YAW,
    V,
    V_TRANS,
    FOV_V,
    FOV_H,
    REZ_V,
    REZ_H,
    DISP_SCALE
}; // if adding property then update resize() for propertyValues and the propertyStrings (currently 14 props)

struct controller {
    QString src;
    QString dst;
    QString port;

    controller(QString src, QString dst, QString port) {
        this->src = src;
        this->dst = dst;
        this->port = port;
    }

    controller() {}
};

namespace Ui {
class BeeWorldWindow;
}

class BeeWorldWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit BeeWorldWindow(QWidget *parent = 0);
    ~BeeWorldWindow();

    /*!
     * \brief connectWorld
     * Function to allow the object to connect itself to an input from a model
     */
     void connectWorld(spineMLNetworkServer *, QString);

public slots:
    void redrawImage();
    void newConnection();
    void redrawScreen();
    bool loadFile(QString fileName);
    bool loadFile();
    void wallFollow(QVector<float>);
    void sweep(QVector<float>);
    void simple(QVector<float>);
    void roll(QVector<float>);
    void optomotor(QVector<float>);


private:
    Ui::BeeWorldWindow *ui;
    beeworld * world;
    QVector < spineMLNetworkServer * > connections;
    //BeeWorldServer * server;
    QTcpServer * server;
    void send_data();
    char * buffer;
    QImage * image;
    bool started;
    bool server_on;
    bool begun;
    int counter;
    QPixmap pix;
    QTimer * timer;
    QTimer * redrawTimer;

    void applyInitialValues();

    QVector <float> propertyValues;
    QVector <float> initialPropertyValues;
    QVector <QString> propertyStrings;
    QVector <controller> controllers;

    double dt;

    bool printErrors(QString);

    /*float dir;
    float pitch;
    float roll;
    float x;
    float y;
    float z;
    float t;
    float v;
    float v_trans;*/
    int index;
    QVector < float > frequencies;

    // system settings
    int numConnections;
    float displayScale;

    //float curr_t;

    float dir_change;

    int x_walls;

    int count;
    float val;


    std::vector < double > imdata;
    int dataSize;
    std::vector < float > x_log;
    std::vector < float > y_log;

    QVector < QVector <float> > log;

    QString logPath;

};


#endif // BEEWORLDWINDOW_H
