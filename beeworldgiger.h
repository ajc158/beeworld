#ifndef BEEWORLDGIGER_H
#define BEEWORLDGIGER_H

#include "beeworld.h"

class beeworldgiger : public beeworld
{
    Q_OBJECT
public:
    explicit beeworldgiger(QObject *parent = 0);

    QImage * getImage(float x, float y, float z, float dir, float pitch, float roll, float curr_t, float scale);

    QImage * getImage(float x, float y, float z, float dir, float pitch, float roll, float curr_t);


private:

signals:

public slots:

};

#endif // BEEWORLDGIGER_H
