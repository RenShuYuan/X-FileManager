#ifndef PROCESSDIALOG_H
#define PROCESSDIALOG_H

#include <QDialog>
#include <QThread>

class QSFiles;

namespace Ui {
class ProcessDialog;
}

class ProcessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProcessDialog(QWidget *parent = 0);
    ~ProcessDialog();

    /**
    * @brief                    调用QSFiles类函数,执行文件夹拷贝
    * @author                   YuanLong
    * @param source             原始数据完整路径
    * @param target             目标数据完整路径
    * @param filters            过滤器列表, 内容为空则忽略过滤器
    * @param ignoreExisted     是否跳过已有文件
    * @return
    */
    void processCopyFolder(const QString &source, const QString &target,
                           const QStringList &filters, const QString &extList,
                           bool ignoreExisted=false);

    /**
    * @brief        调用QSFiles类函数,执行文件拷贝
    * @author       YuanLong
    * @param        参数太多，难得写了，直接看qsfiles.h文件
    * @return
    */
    void processCopyFlies(const QString &source, const QString &target,
                          const QStringList &filters,
                          const bool ignoreExisted,
                          const bool isCopyFilesToFolder,
                          const bool isAutoCreatFolder,
                          const bool isNotIncludedString, const QString& notIncludedString,
                          const bool isNotIncludedStringF, const QString &notIncludedStringF,
                          const bool isSubStringCreatFolder, const int startIndex, const int endIndex,
                          const bool isAddString, const QString& beforeString, const QString& afterString);

    /**
    * @brief                  批量替换文件
    * @author                 YuanLong
    * @param source          原始文件夹路径
    * @param target          目标文件夹路径
    * @param cs              是否区分大小写, 默认为不区分
    * @param isMultiple     是否替换多个文件
    * @return
    */
    void processReplaceFile(const QString &source, const QString &target,
                            Qt::CaseSensitivity cs = Qt::CaseInsensitive,
                            const bool &isMultiple = false);
public slots:
    // 在PlainText中追加text,
    // 主要用于处理过程中的各种信息输出
    void appendPlainText(const QString &text, const QColor & color = Qt::black);

    /**
    * @brief            清除进度条及QLabel信息
    * @author           YuanLong
    * @return
    */
    void clearDialogInfo();

    // 开始与终止一个繁忙进度条
    void startBusyProgress();
    void endBusyProgress();

    // 关机
    void shutdown();

private slots:
    void on_pushButton_clicked();

signals:
    void stopThread();

private:
    Ui::ProcessDialog *ui;
    QThread thread;
    QThread thread1;
};

#endif // PROCESSDIALOG_H
