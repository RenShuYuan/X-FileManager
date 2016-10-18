#ifndef FILECOPYSPEEDCALCULATES_H
#define FILECOPYSPEEDCALCULATES_H

#include <QObject>

class fileCopySpeedCalculates : public QObject
{
    Q_OBJECT
public:
    explicit fileCopySpeedCalculates(QObject *parent = 0);

signals:
    void setLabelText_2(const QString&);
    void bytReset();

public slots:
    /**
    * @brief                计算文件拷贝的速率，及预估剩余时间
    * @author               YuanLong
    * @warning              通过与定时器连接调用,使用该功能需要在文件
    *                       处理函数中对mTotaSize、mCopiedSize、mByte
    *                       进行赋值，结果会构建一个文本通过setLabelText_2
    *                       信号发射。
    * @return
    */
    void rateCalculation();

    void setVol(const qint64 totaSize, const qint64 copiedSize, const qint64 byte);

    void stop();

private:
    volatile bool m_stopped;
    qint64 mTotaSize;                   // 需要拷贝文件总大小
    qint64 mCopiedSize;                 // 已拷贝文件的总大小
    qint64 mByte;                       // 临时保存已拷贝的数据量
};

#endif // FILECOPYSPEEDCALCULATES_H
