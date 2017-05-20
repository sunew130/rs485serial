#ifndef QSERIALMASTER_H
#define QSERIALMASTER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QtSerialPort/QSerialPort>
#include <QTime>

#include "crc16.h"
#include "address.h"
#include "serial/myserialport.h"

class QSerialMaster : public QThread
{
    Q_OBJECT
public:
    explicit QSerialMaster(QObject *parent = 0);

    QByteArray getSlaveCmd(QString slave,QString control,QString address,int crc_bit);

    void analysisRevData(QByteArray data,int control);

    void setRockerUser(QString user);

    void closeCom();

    QString masterControler;
    QString userControler;

protected:
        void run();

signals:
    void timeout(QString);

    void sendInfo(QByteArray);

    //发送电量信息
    void sendElecInfo(QString no_enough,QString charge,QString value);

    //发送LED信息
    void sendLedInfo(QString value);

    //发送Key信息
    void sendKeyInfo(QString value);

    //发送Rocker使用者信息
    void sendRockerInfo(QString value);

    //发送Rocker x-y坐标信息
    void sendRockerXYInfo(QString x,QString y);

public slots:
    void serialtimeoutError(QString info);
private:
    bool quit;
    QString portName;
    QString request;
    int waitTimeout;
    QMutex mutex;
    QWaitCondition cond;

    QSerialPort serial;

    bool isWriteRocker;

};

#endif // QSERIALMASTER_H
