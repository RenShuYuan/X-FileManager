#include <QTime>
#include <QThread>
#include <QDebug>

#include "filecopyspeedcalculates.h"

fileCopySpeedCalculates::fileCopySpeedCalculates(QObject *parent) : QObject(parent)
{
    mTotaSize=0;
    mCopiedSize=0;
    mByte=0;
}

void fileCopySpeedCalculates::rateCalculation()
{
    emit setLabelText_2("速率: 正在计算中...  MB/秒       大约还需要:  正在计算中...  秒");

    while (m_stopped)
    {
        //开始计时，以ms为单位
        QTime time;
        time.start();

        // 休眠
        if (m_stopped)
            QThread::sleep(3);

        // 统计结束用时
        int time_Diff = time.elapsed();

        double mMs = mByte / time_Diff * 1000.0; // 每秒写入的MB
        int timeLeft = (mTotaSize-mCopiedSize) / mMs; // 剩余秒数

        emit bytReset();

        QString text;
        text = "速率: " + QString::number((mMs/1024.0/1024.0), 'g', 2) +
                " MB/秒          大约还需要: " + QString::number(timeLeft) + " 秒";
        emit setLabelText_2(text);
    }
}

void fileCopySpeedCalculates::setVol(const qint64 totaSize, const qint64 copiedSize, const qint64 byte)
{
    mTotaSize = totaSize;
    mCopiedSize = copiedSize;
    mByte = byte;
}

void fileCopySpeedCalculates::stop()
{
    m_stopped = false;
    qDebug()<< "stop";
}
