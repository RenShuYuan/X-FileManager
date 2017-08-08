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
    isCopyFilesToFolder = false;
    isAutoCreatFolder = false;
    isNotIncludedString = false;
    isSubStringCreatFolder = false;
    isAddString = false;
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

void QSFiles::setExtList(const QString &extList)
{
    mExtList = extList;
}

void QSFiles::setMultiple(const bool isMultiple)
{
    this->isMultiple = isMultiple;
}

void QSFiles::setCaseSensitivity(const Qt::CaseSensitivity cs)
{
    this->cs = cs;
}

void QSFiles::setCopyFilesToFolder(const bool isCopyFilesToFolder)
{
    this->isCopyFilesToFolder = isCopyFilesToFolder;
}

void QSFiles::setAutoCreatFolder(const bool isAutoCreatFolder)
{
    this->isAutoCreatFolder = isAutoCreatFolder;
}

void QSFiles::setNotIncludedString(const bool isNotIncludedString, const QString &string)
{
    this->isNotIncludedString = isNotIncludedString;
    notIncludedString = string;
}

void QSFiles::setSubString(const bool isSubStringCreatFolder, const int startIndex, const int endIndex)
{
    this->isSubStringCreatFolder = isSubStringCreatFolder;
    this->startIndex = startIndex;
    this->endIndex = endIndex;
}

void QSFiles::setAddString(const bool isAddString, const QString &beforeString, const QString &afterString)
{
    this->isAddString = isAddString;
    this->beforeString = beforeString;
    this->afterString = afterString;
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
            QString dirName = relative_file_path.left( relative_file_path.indexOf("/") );

            if (mFilters.contains(dirName, Qt::CaseInsensitive) )
            {
                folderList.append(relative_file_path);
                tmpFilter.removeOne(dirName);
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
            emit postMessage("\t**在\""+QDir::toNativeSeparators(mSource)+"\"路径下未找到\""+str+"\"文件夹.", Qt::red);
        }
    }

    return folderList;
}

QStringList QSFiles::searchFiles(const QString &path, QStringList &filters, const QString &type,
                                 const QString &excludeExtension)
{
    QStringList list;

    if (!m_stopped)
        return list;

    QDirIterator dir_iterator(path, filters, QDir::Files | QDir::NoSymLinks,
                                             QDirIterator::Subdirectories);

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

        // 分隔扩展名列表
        QStringList sxtList = excludeExtension.split(';', QString::SkipEmptyParts,
                                                            Qt::CaseInsensitive);
        if (!sxtList.isEmpty())
        {
            if ( sxtList.contains(file_info.suffix()) )
            {
                continue;
            }
        }

        if ( !mFilters.isEmpty() )
        {
            QString subName;

            if (type == "batchCopyFolder")
            {
                QDir dir(path);
                QString relative_file_path = dir.relativeFilePath(filePath);
                subName = relative_file_path.left( relative_file_path.indexOf("/") );
            }
            else
            {
                subName = file_info.fileName();
            }
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
            emit postMessage("\t** 子目录构建失败: "+path, Qt::red);
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
            emit postMessage(QString("\t**文件复制失败, 无法写入: %1\n程序已终止.").arg(QFileInfo(target).fileName()), Qt::red);
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
            emit postMessage(QString("\t**文件复制失败, 无法读取: %1\n程序已终止.").arg(QFileInfo(source).baseName()), Qt::red);
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

        emit postMessage(QString("\t**文件复制失败, 检测到文件未复制完整: %1").arg(QFileInfo(source).fileName()), Qt::red);
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

    emit postMessage("执行批量文件夹拷贝:", Qt::black);

    //开始计时，以ms为单位
    QTime time;
    startTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    time.start();

    // 原始路径下所有子文件夹，相对路径
    emit postMessage("\t检索原始目录结构...", Qt::black);
    QStringList list = getAllFolder();
    folders_count = list.size();
    emit postMessage(QString("\t共搜索到%1个文件夹.").arg(folders_count), Qt::black);

    if (!list.isEmpty() && m_stopped)
    {
        // 创建子文件夹
        emit postMessage("\t重新构建文件夹结构...", Qt::black);
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
            emit postMessage("\t检索子文件...", Qt::black);
            list = searchFiles(mSource, QStringList()<<"*.*", "batchCopyFolder", mExtList);
            files_count = list.size();
            emit postMessage(QString("\t共搜索到%1个子文件待拷贝.").arg(files_count), Qt::black);
        }

        // 复制文件
        if (!list.isEmpty() && m_stopped)
        {
            int prCount = 0;
            int isuccess = 0;

            // 初始化主进度条
            emit setValuePd(0);
            emit setMaximumPd(list.size());

            emit postMessage("\t开始拷贝文件...", Qt::black);

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
                        else
                            emit postMessage(QString("\t%1 : 拷贝失败,未知错误 : %2").arg(prCount+1).arg(QFileInfo(fileName).fileName()), Qt::red);
                    }
                }
                else
                {
                    if (copyFile(fileName, targetPath))
                        ++isuccess;
                    else
                        emit postMessage(QString("\t%1 : 替换失败,未知错误 : %2").arg(prCount+1).arg(QFileInfo(fileName).fileName()), Qt::red);
                }

                emit setValuePd(++prCount);
            }

            files_success_count = isuccess;
        }
    }

    // 统计结束用时
    time_Diff = time.elapsed();
    endTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    outputStatistics("拷贝文件夹结束.");
    emit finished();
}

void QSFiles::batchCopyFlies()
{
    m_stopped = true;

    emit postMessage("执行批量文件拷贝:", Qt::black);

    //开始计时，以ms为单位
    QTime time;
    startTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    time.start();

    // 分别获得原始文件夹路径下所有文件
    QStringList sourceList = searchFiles(mSource, QStringList()<<"*.*", "batchCopyFlies");

    // 检查有效性
    if (sourceList.isEmpty())
    {
        emit postMessage("\t** 在原始文件夹中未搜索到待处理文件.", Qt::black);
        // 统计结束用时
        time_Diff = time.elapsed();
        endTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        outputStatistics("拷贝文件结束.");
        return;
    }

    files_count = sourceList.size();

    // 初始化主进度条
    int prCount = 0;
    emit setValuePd(0);
    emit setMaximumPd(files_count);
    emit postMessage("开始拷贝文件...", Qt::black);

    // 文件完整路径
    QString sourceFile;
    QString targetFile;

    if (isCopyFilesToFolder)
    {
        // 开始拷贝文件
        for (int i = 0; i < files_count; ++i)
        {
            if (!m_stopped)
                break;

            // 获得文件名称，不带扩展名
            sourceFile = sourceList.at(i);
            QString folderName = QFileInfo(sourceFile).baseName();
            QString sourceFileName = QFileInfo(sourceFile).fileName();

            emit setLabelText(sourceFileName);

            // 根据规则修改文件夹名称
            if (isNotIncludedString) // 不包含字符
            {
                folderName = folderName.remove(notIncludedString, Qt::CaseInsensitive);
            }
            else if (isSubStringCreatFolder) // 截取部分文件名
            {
                folderName = folderName.mid(startIndex-1, endIndex-1);
            }
            else if (isAddString) // 在文件名前后增加字段
            {
                folderName = beforeString + folderName + afterString;
            }

            targetFile = mTarget + "/" + folderName;
            if (!QDir(targetFile).exists()) // 检查该路径下文件目录是否存在
            {
                if (isAutoCreatFolder) // 自动创建文件夹
                {
                    QDir dir;
                    if (!dir.mkpath(targetFile))  // 创建文件夹
                    {
                        emit postMessage(QString("\t%1 : 拷贝失败，目录创建失败 : %2").arg(i+1).arg(targetFile), Qt::red);
                        emit setValuePd(++prCount);
                        continue;
                    }
                }
                else
                {
                    emit postMessage(QString("\t%1 : 拷贝失败，没有对应文件夹 : %2").arg(i+1).arg(targetFile), Qt::red);
                    emit setValuePd(++prCount);
                    continue;
                }
            }

            targetFile = targetFile + "/" + sourceFileName;

            // 跳过已存在的文件
            if (ignoreExisted)
            {
                if (QFileInfo(targetFile).exists()) // 目标文件不存在
                {
                    emit postMessage(QString("\t%1 : 拷贝失败，该路径下已存在相同名称的文件 : %2").arg(i+1).arg(sourceFileName), Qt::red);
                }
                else
                {
                    if (copyFile(sourceFile, targetFile))
                    {
                        ++files_success_count;
                        emit postMessage(QString("\t%1 : 拷贝成功 : %2").arg(i+1).arg(sourceFileName), Qt::black);
                    }
                    else
                        emit postMessage(QString("\t%1 : 拷贝失败，未知错误 : %2").arg(i+1).arg(sourceFileName), Qt::red);
                }
            }
            else
            {
                if (copyFile(sourceFile, targetFile))
                {
                    ++files_success_count;
                    emit postMessage(QString("\t%1 : 拷贝成功 : %2").arg(i+1).arg(sourceFileName), Qt::black);
                }
                else
                    emit postMessage(QString("\t%1 : 替换失败,未知错误 : %2").arg(prCount+1).arg(sourceFileName), Qt::red);
            }

            emit setValuePd(++prCount);
        }
    }
    else
    {   // 开始拷贝文件
        for (int i = 0; i < files_count; ++i)
        {
            if (!m_stopped)
                break;

            sourceFile = sourceList.at(i);
            QString sourceFileName = QFileInfo(sourceFile).fileName();
            targetFile = mTarget + "/" + QFileInfo(sourceFile).fileName();

            emit setLabelText(sourceFileName);

            // 忽略已存在的文件
            if (ignoreExisted)
            {
                if (!QFileInfo(targetFile).exists()) // 目标文件不存在
                {
                    if (copyFile(sourceFile, targetFile))
                    {
                        ++files_success_count;
                        emit postMessage(QString("\t%1 : 拷贝成功 : %2").arg(i+1).arg(sourceFileName), Qt::black);
                    }
                    else
                        emit postMessage(QString("\t%1 : 拷贝失败，未知错误 : %2").arg(i+1).arg(sourceFileName), Qt::red);
                }
                else
                    emit postMessage(QString("\t%1 : 拷贝失败，文件已存在 : %2").arg(i+1).arg(sourceFileName), Qt::red);
            }
            else
            {
                if (copyFile(sourceFile, targetFile))
                {
                    ++files_success_count;
                    emit postMessage(QString("\t%1 : 拷贝成功 : %2").arg(i+1).arg(targetFile), Qt::black);
                }
                else
                    emit postMessage(QString("\t%1 : 拷贝失败，未知错误 : %2").arg(i+1).arg(sourceFileName), Qt::red);
            }
            emit setValuePd(++prCount);
        }
    }

    // 统计结束用时
    time_Diff = time.elapsed();
    endTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    outputStatistics("拷贝文件结束.");
    emit finished();
}

void QSFiles::batchReplaceFile()
{
    m_stopped = true;

    emit postMessage("执行批量文件替换:", Qt::black);

    //开始计时，以ms为单位
    QTime time;
    startTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    time.start();

    // 分别获得原始、目标文件夹路径下所有文件
    QStringList sourceList = searchFiles(mSource, QStringList()<<"*.*", "batchCopyFlies");
    QStringList targetList = searchFiles(mTarget, QStringList()<<"*.*", "batchCopyFlies");

    // 检查有效性
    if (sourceList.isEmpty() || targetList.isEmpty())
    {
        emit postMessage("\t** 在原始文件夹或目标文件夹中未搜索到待处理文件.", Qt::red);
        // 统计结束用时
        time_Diff = time.elapsed();
        endTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        outputStatistics("替换文件结束.");
        return;
    }

    files_count = sourceList.size();

    // 初始化主进度条
    int prCount = 0;
    emit setValuePd(0);
    emit setMaximumPd(files_count);
    emit postMessage("开始替换文件...", Qt::black);

    // 替换文件
    for (int i = 0; i < files_count; ++i)
    {
        if (!m_stopped)
            break;

        emit setLabelText(QFileInfo(sourceList.at(i)).fileName());

        int nIndex = -2;

        // 文件完整路径
        QString sourceFile;
        QString targetFile;

        // 文件名称
        QString sourceFileName;
        QString targetFileName;

        sourceFile = sourceList.at(i);
        sourceFileName = QFileInfo(sourceFile).fileName();

        for (int j = 0; j < targetList.size(); ++j)
        {
            targetFile = targetList.at(j);
            targetFileName = QFileInfo(targetFile).fileName();

            // 是否忽略大小写
            if (cs==Qt::CaseInsensitive)
            {
                sourceFileName = sourceFileName.toLower();
                targetFileName = targetFileName.toLower();
            }

            if (sourceFileName == targetFileName)
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
                    {
                        ++files_success_count;
                        ++nIndex;
                    }
                    else
                        emit postMessage(QString("\t%1 : 替换失败,未知错误 : %2").arg(i+1).arg(sourceFileName), Qt::red);
                }
            }
        }
        if (isMultiple)
        {
            if (nIndex == -2)
            {
                emit postMessage(QString("\t%1 : 替换失败,没有找到对应文件 : %2").arg(i+1).arg(sourceFileName), Qt::red);
            }
            else if (nIndex == -1)
            {
                emit postMessage(QString("\t%1 : 替换失败,找到多个对应文件 : %2").arg(i+1).arg(sourceFileName), Qt::red);
            }
            else
            {
                if (copyFile(sourceFile, targetList.at(nIndex)))
                {
                    ++files_success_count;
                    emit postMessage(QString("\t%1 : 替换成功,替换1处 : %2").arg(i+1).arg(sourceFileName), Qt::black);
                }
                else
                    emit postMessage(QString("\t%1 : 替换失败,未知错误 : %2").arg(i+1).arg(sourceFileName), Qt::red);
            }
        }
        else
        {
            if (!(nIndex + 2))
            {
                emit postMessage(QString("\t%1 : 替换失败,没有找到对应文件 : %2").arg(i+1).arg(sourceFileName), Qt::red);
            }
            else
            {
                emit postMessage(QString("\t%1 : 替换成功,替换%3处 : %2").arg(i+1).arg(sourceFileName).arg(nIndex+2), Qt::black);
            }
        }
        emit setValuePd(++prCount);
    }

    // 统计结束用时
    time_Diff = time.elapsed();
    endTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    outputStatistics("替换文件结束.");
    emit finished();
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
    emit postMessage(title, Qt::black);
    emit postMessage("输出汇总结果:", Qt::black);
    emit postMessage(QString("\t共找到    : %1个文件夹, %2个文件, %3 字节").arg(folders_count).arg(files_count).arg(mTotaSize), Qt::black);
    emit postMessage(QString("\t成功处理: %1个文件夹, %2个文件, %3 字节").arg(folders_success_count).arg(files_success_count).arg(mCopiedSize), Qt::black);
    emit postMessage(QString("\t开始于    : %1\n\t结束于    : %2\n\t共用时    : %3 /分钟").arg(startTime).arg(endTime).arg(QString::number(time_Diff/1000.0/60.0, 'g', 2)), Qt::black);
}
