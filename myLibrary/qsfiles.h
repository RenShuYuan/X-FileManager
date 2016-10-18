#ifndef QSFILES_H
#define QSFILES_H

#include <QObject>
#include <QMutex>

//  需要连接postMessage与setLabelText
class QSFiles : public QObject
{
    Q_OBJECT
public:
    explicit QSFiles(QObject *parent = 0);

    /**
    * @brief            设置源路径与目标路径
    * @author           YuanLong
    * @param source 	源文件夹绝对路径
    * @param target 	目标文件夹绝对路径
    * @warning          QSFiles类对文件夹与文件的处理，
    *                   基本都需要提前设置，否则在处理
    *                   时无法找到正确路径.
    * @return
    */
    void setSourcePath(const QString &source);
    void setTargetPath(const QString &target);

    /**
    * @brief            设置路径前端过滤列表
    * @author           YuanLong
    * @param filters 	过滤器列表, 如*.*, *.txt
    * @warning          如果调用该函数设置了过滤内容，则在调用文件夹
    *                   与文件处理函数时会根据过滤器列表内容过滤掉
    *                   路径前端不符合该要求的子文件夹与文件路径。
    * @return
    */
    void setFilters(const QStringList &filters);

    /**
    * @brief            是否跳过已存在的文件
    * @author           YuanLong
    * @param ignore 	true则为跳过已存在的文件
    * @return
    */
    void setExisted(const bool ignore);

    /**
    * @brief            遍历目录下所有文件夹
    * @author           YuanLong
    * @warning          使用该功能前需要先利用setSourcePath设置文件夹路径,
    *                   该函数会自己调用一个进度对话框显示搜索状态.
    * @return			返回搜索到的文件夹列表（相对路径）, 返回的结果受过
    *                   滤器列表影响。
    */
    QStringList getAllFolder();

    /**
    * @brief            搜索文件（包括子目录）
    * @author           YuanLong
    * @param filters	过滤器列表，搜索所有文件用*.*
    * @warning          使用该功能前需要先利用setSourcePath设置文件夹路径,
    *                   该函数会自己调用一个进度对话框显示搜索状态.
    * @return			返回搜索到的文件列表，返回的结果受过滤器列表影响。
    */
    QStringList searchFiles(const QString &path, QStringList &filters);

    /**
    * @brief                利用list列表中的相对路径在磁盘上批量创建文件夹
    * @author               YuanLong
    * @param list           文件夹相对路径列表
    * @param successCount  返回成功创建文件夹的数量
    * @warning              使用该功能前需要先利用setSourcePath设置文件夹路径,
    *                       再通过与每个相对路径结合而得到绝对路径。
    *                       该函数会发射与进度条相关的信号用于更新处理进度，要
    *                       使用该功能需要连接QProgressBar类对应的槽；
    * @return
    */
    bool createFolders(const QStringList& list, int &successCount);

    /**
    * @brief            拷贝单个文件
    * @author           YuanLong
    * @param source     原始数据完整路径
    * @param target     目标数据完整路径
    * @warning          需要连接QProgressBar_2类对应的槽, 用于显示当前进度;
    *
    * @return           成功返回true,失败false
    */
    bool copyFile(const QString &source, const QString &target);

public slots:

    /**
    * @brief            批量处理文件夹拷贝
    * @author           YuanLong
    * @return
    */
    void batchCopyFolder();

    /**
    * @brief            批量替换文件
    * @author           YuanLong
    * @return
    */
    void batchReplaceFile();

    /**
    * @brief            重置mByte,用于速率计算
    * @author           YuanLong
    * @return
    */
    void setByteReset();

    /**
    * @brief            多线程处理时,用于结束当前操作
    * @author           YuanLong
    * @return
    */
    void stop();

private slots:


signals:
    // 发送一个文本信息
    void postMessage(const QString&);

    // 设置标签文本
    void setLabelText(const QString&);
    void setLabelText_2(const QString&);

    // 发送信号，设置与进度条的相关信息，
    // 用于显示当前的执行总进度
    void setVisiblePd(bool);
    void setMaximumPd(int);
    void setValuePd(int);

    // 用于显示当前单个文件进度
    void setVisiblePd_2(bool);
    void setMaximumPd_2(int);
    void setValuePd_2(int);

    // 开始与终止一个繁忙进度条
    void startBusyProgress();
    void endBusyProgress();

    void setVol(const qint64 totaSize, const qint64 copiedSize, const qint64 byte);

    // 操作正常结束后发送次消息
    void finished();

private:
    /**
    * @brief                输出统计信息
    * @author               YuanLong
    * @param title          输出标题
    * @warning              通过发射postMessage信号来输出信息。
    * @return
    */
    void outputStatistics(const QString& title);

private:
    QWidget* mWidget;       // 用于保存父窗口的QWidget指针
    QString mSource;        // 原始路径
    QString mTarget;        // 目标路径
    QStringList mFilters;   // 文件夹过滤器
    bool    ignoreExisted;  // 是否忽略已存在的文件
    qint64 totalCopySize;

    // 保存速率计算的参数
    static const int interval = 2000;   // 定时器间隔(毫秒)
    qint64 mTotaSize;                   // 需要拷贝文件总大小
    qint64 mCopiedSize;                 // 已拷贝文件的总大小
    volatile qint64 mByte;              // 临时保存已拷贝的数据量

    // 用于统计信息输出
    int folders_count;
    int files_count;
    int folders_success_count;
    int files_success_count;
    int time_Diff;
    QString startTime;
    QString endTime;

    volatile bool m_stopped;
    mutable QMutex mutex;
};

#endif // QSFILES_H
