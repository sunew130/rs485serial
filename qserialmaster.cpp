#include "qserialmaster.h"

QSerialMaster::QSerialMaster(QObject *parent) :
    QThread(parent),quit(false),isWriteRocker(false),masterControler(SLAVE_Y),userControler(SLAVE_Y)
{
    connect(this,SIGNAL(timeout(QString)),this,SLOT(serialtimeoutError(QString)));
}

void QSerialMaster::run()
{
    mutex.lock();
    int currentWaitTimeout = waitTimeout = 40;
    QByteArray request;
    mutex.unlock();
    //! [5] //! [6]
    serial.setPortName(COM);
    serial.setBaudRate(QSerialPort::Baud19200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial.open(QIODevice::ReadWrite)) {
        return;
    }

    int control = 1;

    while (!quit) {

        switch (control) {
        case 1:
            request =  getSlaveCmd(SLAVE,READ,BAT_ADDRESS,1);
            break;
        case 2:
            request =  getSlaveCmd(SLAVE,READ,KEY_ADDRES,2);
            break;
        case 3:
            if (isWriteRocker)
            {
                QString cmd;
                QString crc;
                cmd =  SLAVE + WRITE + Y_ADDRESS + masterControler;
                crc = ToStr(checksum(QByteArray::fromHex(cmd.toLatin1())),4);
                cmd = cmd + crc;
                request = QByteArray::fromHex(cmd.toLatin1());;
                isWriteRocker = false;
            }
            break;
        case 4:
            if(userControler == MASTER_Y)
            {
                request =  getSlaveCmd(SLAVE,READ,ROCKER_X_ADDRESS,2);
                msleep(50);
            }
            break;
        }

        serial.write(request);
        if (serial.waitForBytesWritten(waitTimeout)) {
            //! [8] //! [10]
            // read response
            if (serial.waitForReadyRead(currentWaitTimeout)) {
                QByteArray responseData = serial.readAll();
                while (serial.waitForReadyRead(20))
                    responseData += serial.readAll();

                    this->analysisRevData(responseData,control);
            } else {
                emit timeout(tr("Wait read response timeout %1")
                             .arg(QTime::currentTime().toString()));
            }
            //! [9] //! [11]
        } else {
            emit timeout(tr("Wait write request timeout %1")
                         .arg(QTime::currentTime().toString()));
        }

        if(control == 4)
            control =0;

        control++;

        request.clear();

          msleep(10);
    }
    //! [13]
}

//关闭串口
void QSerialMaster::closeCom()
{
    quit = false;
    this->msleep(500);
    serial.close();
}


QByteArray QSerialMaster::getSlaveCmd(QString slave,QString control,QString address,int crc_bit)
{
    QString cmd;
    QString crc;
    cmd = SLAVE + READ + address + ToStr(crc_bit,4);
    crc = ToStr(checksum(QByteArray::fromHex(cmd.toLatin1())),4);
    cmd = cmd + crc;
    return QByteArray::fromHex(cmd.toLatin1());
}




//设置控制摇杆使用者
void QSerialMaster::setRockerUser(QString user)
{
    masterControler = user;
    isWriteRocker = true;
}


void QSerialMaster::analysisRevData(QByteArray data,int control)
{
    qDebug()<<"rev:"<<QString(crcOk(data))<<"----control:"<<control;

    if(data.size() == 5)
    {
         QString cmd(crcOk(data));

         if(cmd.mid(0,4) == (SLAVE + READ_ERROR) )
         {
                qDebug()<<"SLAVE READ ERROR:" <<cmd.mid(4,2);
         }

         if(cmd.mid(0,4) == (SLAVE + WRITE_ERROR) )
         {
                 qDebug()<<"SLAVE WRITE ERROR:" <<cmd.mid(4,2);
         }
    }

    //获取电池信息
    switch (control) {
    case 1:
        if(data.size() == 7)
        {
            QString cmd(crcOk(data));
            if(cmd.mid(0,6) == (SLAVE + READ + tr("02")))
            {
                //发送电量信息
                emit sendElecInfo(cmd.mid(6,1),cmd.mid(7,1),cmd.mid(9,1));
            }

        }
        break;
    case 2:
        if(data.size() == 9)
        {
            QString cmd(crcOk(data));
            if(cmd.mid(0,6) == (SLAVE + READ + tr("04")))
            {
                //发送Key信息
                emit sendKeyInfo(cmd.mid(6,4));
                emit sendLedInfo(cmd.mid(10,4));
            }
        }

        break;
    case 3:
        if(data.size() == 8)
        {

            QString cmd(crcOk(data));
            if(cmd.mid(0,8) == (SLAVE + WRITE + Y_ADDRESS))
            {
                //发送Rocker信息
                emit sendRockerInfo(cmd.mid(8,4));
            }
        }

        break;
    case 4:
        if(data.size() == 9)
        {
            QString cmd(crcOk(data));
            if(cmd.mid(0,6) == (SLAVE + READ + tr("04")))
            {
                //发送Rocker x y坐标信息
                emit sendRockerXYInfo(cmd.mid(6,4),cmd.mid(10,4));
            }
        }

        break;
    }

}


void QSerialMaster::serialtimeoutError(QString info)
{
   // qDebug()<<info;
}




