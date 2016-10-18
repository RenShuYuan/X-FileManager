#include "qsfiles.h"
#include <QDir>
#include <QPoint>
#include <QWidget>
#include <QDirIterator>
#include <QProgressDialog>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QRegExp>
#include <QTime>
#include <QDateTime>
#include <QThread>
#include <QMutexLocker>

QSFiles::QSFiles(QObject *parent) : QObject(parent)
{
    // 获得父窗口的QWidget指针
    mWidget = dynamic_cast< QWidget* >(parent);

    mWidget = NULL;
    m_stopped = false;
    ignoreExisted = false;
    folders_count = 0;
    files_count = 0;
    folders_success_count = 0;
    files_success_count = 0;
    time_Diff = 0;
    mTotaSize = 0;
    mCopiedSize = 0;
    mByte = 0;
    totalCopySize = 0;
}

void QSFiles::setSourcePath(const QString &source)
{
    mSource = source;
}

void QSFiles::setTargetPath(const QString &target)
{
    mTarget = target;
}

void QSFiles::setFilters(const QStringList &filters)
{
    mFilters = filters;
}

void QSFiles::setExisted(const bool ignore)
{
    ignoreExisted = ignore;
}

QStringList QSFiles::getAllFolder()
{
    QStringList folderList;

    if (!m_stopped)
        return folderList;

    // 构建过滤器
    QDirIterator iterator(mSource, QDir::Dirs | QDir::NoSymLinks |
                          QDir::NoDotAndDotDot | QDir::Hidden,
                          QDirIterator::Subdirectories |
                          QDirIterator::FollowSymlinks);

    // 设置过滤器相关信息
    bool isFilter = !mFilters.isEmpty();
    QStringList tmpFilter;
    if (isFilter)
        tmpFilter = mFilters;

    if (!m_stopped)
        return folderList;

    emit startBusyProgress();

    QDir dir(mSource);
    while (iterator.hasNext())
    {
        if (!m_stopped)
        {
            emit endBusyProgress();
            folderList.clear();
            return folderList;
        }

        iterator.next();
        QFileInfo file_info = iterator.fileInfo();
        QString absolute_file_path = file_info.absoluteFilePath();
        QString relative_file_path = dir.relativeFilePath(absolute_file_path);

        // 使用过滤器
        if ( isFilter )
        {
            QString subName = relative_file_path.left( relative_file_path.indexOf("/") );
            if (mFilters.contains(subName, Qt::CaseInsensitive) )
            {
                folderList.append(relative_file_path);
                tmpFilter.removeOne(subName);
            }
        }
        else
        {
            folderList.append(relative_file_path);
        }
        emit setLabelText(relative_file_path);
    }

    emit endBusyProgress();

    if (isFilter)
    {
        foreach (QString str, tmpFilter)
        {
            emit postMessage("\t**在\""+QDir::toNativeSeparators(mSource)+"\"路径下未找到\""+str.toLocal8Bit().data()+"\"文件夹.");
        }
    }

    return folderList;
}

QStringList QSFiles::searchFiles(const QString &path, QStringList &filters)
{
    QStringList list;

    if (!m_stopped)
        return list;

    QDirIterator dir_iterator(path, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

    emit startBusyProgress();

    while (dir_iterator.hasNext())
    {
        if (!m_stopped)
        {
            emit endBusyProgress();
            list.clear();
            return list;
        }

        dir_iterator.next();
        QFileInfo file_info = dir_iterator.fileInfo();
        QString filePath = file_info.filePath();
        qint64 dataSize = file_info.size();

        if ( !mFilters.isEmpty() )
        {
            QDir dir(path);
            QString relative_file_path = dir.relativeFilePath(filePath);
            QString subName = relative_file_path.left( relative_file_path.indexOf("/") );
            if (mFilters.contains(subName, Qt::CaseInsensitive) )
            {
                list.append(filePath);
                mTotaSize += dataSize;
            }
        }
        else
        {
            list.append(filePath);
            mTotaSize += dataSize;
        }
        emit setLabelText(filePath);
    }

    emit endBusyProgress();

    return list;
}

bool QSFiles::createFolders(const QStringList &list, int &successCount)
{
    if (!m_stopped)
        return false;

    emit setValuePd(0);
    emit setMaximumPd(list.size());

    int isuccess = 0;
    int prCount = 0;
    foreach (QString path, list)
    {
        if (!m_stopped)
            break;

        QDir dir;
        if (!dir.mkpath(mTarget + '/' + path))
        {
            emit postMessage("\t** 子目录构建失败: "+path);
            return false;
        }
        else
        {
            ++isuccess;
        }
        emit setValuePd(++prCount);
    }
    successCount = isuccess;
    return true;
}

bool QSFiles::copyFile(const QString &source, const QString &target)
{
    if (!m_stopped)
        return false;

    // 缓存字节数组
    qint64 fileSize = 0;
    totalCopySize = 0;

    // 初始化进度条
    emit setValuePd_2(0);

    // 写数据
    QFile tofile(target);
    if (!tofile.open(QIODevice::ReadWrite))
    {
        QMessageBox::StandardButton sb = QMessageBox::information(
                    mWidget, "读写错误",
                    "写入文件时发生错误,\n请检查目标路径能否写入、磁盘空间是否足够,\n是否要重试?",
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if ( sb == QMessageBox::Yes )
        {
            return copyFile(source, target);
        }
        else
        {
            m_stopped = false;
            emit postMessage(QString("\t**文件复制失败, 无法写入: %1\n程序已终止.").arg(QFileInfo(target).fileName()));
            return false;
        }
    }

    // 读数据
    QFile fromfile(source);
    if(!fromfile.open(QFile::ReadOnly))
    {
        tofile.close();
        QMessageBox::StandardButton sb = QMessageBox::information(
                    mWidget, "读写错误",
                    "读取文件时发生错误,请检查数据源能否正常读取,\n是否要重试?",
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if ( sb == QMessageBox::Yes )
        {
            return copyFile(source, target);
        }
        else
        {
            m_stopped = false;
            emit postMessage(QString("\t**文件复制失败, 无法读取: %1\n程序已终止.").arg(QFileInfo(source).baseName()));
            return false;
        }
    }
    fileSize = fromfile.size();

    // 设置进度最大值
    emit setMaximumPd_2(fileSize);

    int count = 0;
    char byteTemp[4096];
    while(!fromfile.atEnd())
    {
        if (!m_stopped)
        {
            tofile.close();
            fromfile.close();

            if (QFile::exists(target))
                if (!QFile::remove(target))
                    mCopiedSize += totalCopySize;

            return false;
        }

        qint64 readSize = 0;
        //读入字节数组,返回读取的字节数量，如果小于4096，则到了文件尾
        readSize = fromfile.read(byteTemp, sizeof(byteTemp));
        if (readSize <= -1)
        {
            tofile.close();
            fromfile.close();

            if ( QMessageBox::Yes == QMessageBox::information(mWidget, "读写错误",
                                                              "读取文件时发生错误,请检查数据源能否正常读取,\n是否要重试?",
                                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) )
            {
                return copyFile(source, target);
            }
            else
            {
                QDir dir(target);
                if (dir.exists())
                    if (!dir.remove(target))
                        mCopiedSize += totalCopySize;

                m_stopped = false;
                return false;
            }
        }

        qint64 writeSize = tofile.write(byteTemp, readSize);
        if (writeSize <= -1)
        {
            tofile.close();
            fromfile.close();

            QMessageBox::StandardButton sb = QMessageBox::information(
                        mWidget, "读写错误",
                        "写入文件时发生错误,\n请检查目标路径能否写入、磁盘空间是否足够,\n是否要重试?",
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if ( sb == QMessageBox::Yes )
            {
                return copyFile(source, target);
            }
            else
            {
                QDir dir(target);
                if (dir.exists())
                    if (!dir.remove(target))
                        mCopiedSize += totalCopySize;

                m_stopped = false;
                return false;
            }
        }

        totalCopySize += writeSize;
        mByte += writeSize;

        // 更新进度
        if ((++count)%10 == 0)
        {
            emit setValuePd_2(totalCopySize);
            emit setVol(mTotaSize, mCopiedSize, mByte);
        }
    }
    tofile.close();
    fromfile.close();

    if (totalCopySize == fileSize)
    {
        mCopiedSize += totalCopySize;
        return true;
    }
    else
    {
        QDir dir(target);
        if (dir.exists())
            if (!dir.remove(target))
                mCopiedSize += totalCopySize;

        emit postMessage(QString("\t**文件复制失败, 检测到文件未复制完整: %1").arg(QFileInfo(source).fileName()));
        return false;
    }
}

void QSFiles::stop()
{
    m_stopped = false;
}

void QSFiles::batchCopyFolder()
{
    m_stopped = true;

    emit postMessage("执行批量文件夹拷贝:");

    //开始计时，以ms为单位
    QTime time;
    startTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    time.start();

    // 原始路径下所有子文件夹，相对路径
    emit postMessage("\t检索原始目录结构...");
    QStringList list = getAllFolder();
    folders_count = list.size();
    emit postMessage(QString("\t共搜索到%1个文件夹.").arg(folders_count));

    if (!list.isEmpty() && m_stopped)
    {
        // 创建子文件夹
        emit postMessage("\t重新构建文件夹结构...");
        if ( !createFolders(list, folders_success_count) || !m_stopped )
        {
            // 统计结束用时
            time_Diff = time.elapsed();
            endTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            outputStatistics("拷贝文件夹结束");
            return;
        }

        // 获得所有子文件
        list.clear();

        if (m_stopped)
        {
            emit postMessage("\t检索子文件...");
            list = searchFiles(mSource, QStringList()<<"*.*");
            files_count = list.size();
            emit postMessage(QString("\t共搜索到%1个子文件待拷贝.").arg(files_count));
        }

        // 复制文件
        if (!list.isEmpty() && m_stopped)
        {
            int prCount = 0;
            int isuccess = 0;

            // 初始化主进度条
            emit setValuePd(0);
            emit setMaximumPd(list.size());

            emit postMessage("\t开始拷贝文件...");

            foreach (QString fileName, list)
            {
                if (!m_stopped)
                    break;

                emit setLabelText(QFileInfo(fileName).fileName());

                QDir dir(mSource);
                QString relative_file_path = dir.relativeFilePath(fileName);
                QString targetPath = mTarget + "/" + relative_file_path;

                // 忽略已存在的文件
                if (ignoreExisted)
                {
                    if (!QFileInfo(targetPath).exists()) // 目标文件不存在
                    {
                        if (copyFile(fileName, targetPath))
                            ++isuccess;
                    }
                }
                else
                {
                    if (copyFile(fileName, targetPath))
                        ++isuccess;
                }

                emit setValuePd(++prCount);
            }

            files_success_count = isuccess;
        }
    }

    // 统计结束用时
    time_Diff = time.elapsed();
    endTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    outputStatistics("拷贝文件夹结束");

    emit finished();
}

void QSFiles::batchReplaceFile()
{
    m_stopped = true;

    bool isbool = false;
    int nIndex = -2;

    emit postMessage("执行批量文件替换:");

    //开始计时，以ms为单位
    QTime time;
    startTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    time.start();

    // 分别获得原始、目标文件夹路径下所有文件
    QStringList sourceList = searchFiles(mSource, QStringList()<<"*.*");
    QStringList targetList = searchFiles(mTarget, QStringList()<<"*.*");

    // 检查有效性
    if (sourceList.isEmpty() || targetList.isEmpty())
    {
        emit postMessage("\t** 在原始文件夹或目标文件夹中未搜索到待处理文件.");
        // 统计结束用时
        time_Diff = time.elapsed();
        endTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        outputStatistics("替换文件结束");
        return;
    }

    bool isMultiple=true;

    // 替换文件
    for (int i = 0; i < sourceList.size(); ++i)
    {
        // 是否忽略大小写
        QString sourceFile = sourceList.at(i);
//        if (cs==Qt::CaseSensitive)
//            sourceFile = sourceFile.toLower();

        for (int j = 0; j < targetList.size(); ++j)
        {
            // 是否忽略大小写
            QString targetFile = targetList.at(i);
//            if (cs==Qt::CaseSensitive)
//                targetFile = targetFile.toLower();

            if (QFileInfo(sourceFile).fileName() == QFileInfo(targetFile).fileName())
            {
                if (isMultiple) // 对应多个文件时不替换
                {
                    if (nIndex==-2)
                        nIndex = j;
                    else
                    {
                        nIndex = -1;
                        break;
                    }
                }
                else
                {
                    if (copyFile(sourceFile, targetFile))
                        ++files_success_count;
                }
            }
        }
    }
}

void QSFiles::setByteReset()
{
    QMutexLocker locker(&mutex);
    mByte = 0;
}

void QSFiles::outputStatistics(const QString &title)
{
    // 输出汇总结果
    emit setLabelText("");      // 清空标签
    emit setLabelText_2("");    // 清空标签
    emit postMessage("\n");
    emit postMessage(title);
    emit postMessage("输出汇总结果");
    emit postMessage(QString("\t共找到    : %1个文件夹, %2个文件, %3 字节").arg(folders_count).arg(files_count).arg(mTotaSize));
    emit postMessage(QString("\t成功处理: %1个文件夹, %2个文件, %3 字节").arg(folders_success_count).arg(files_success_count).arg(mCopiedSize));
    emit postMessage(QString("\t开始于: %1\n\t结束于: %2\n\t共用时: %3 /分钟").arg(startTime).arg(endTime).arg(QString::number(time_Diff/1000.0/60.0, 'g', 2)));
}
