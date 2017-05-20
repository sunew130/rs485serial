#ifndef CRC16_H
#define CRC16_H

#include <QByteArray>
#include <QString>

quint16 checksum(QByteArray data);

QString ToStr(int num, int size);

QByteArray crcOk(QByteArray data);

#endif // CRC16_H
