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
    * @param ignoreCase        是否忽略文件夹名称大小写
    * @return
    */
    void processCopyFolder(const QString &source, const QString &target,
                           const QStringList &filters,bool ignoreExisted=false);

    /**
    * @brief            批量替换文件
    * @author           YuanLong
    * @param source     原始文件夹路径
    * @param target     目标文件夹路径
    * @param cs         是否区分大小写, 默认为不区分
    * @return
    */
    void processReplaceFile(const QString &source, const QString &target,
                            Qt::CaseSensitivity cs = Qt::CaseInsensitive,
                            const bool &isMultiple = false);
public slots:
    // 在PlainText中追加text,
    // 主要用于处理过程中的各种信息输出
    void appendPlainText(const QString &text);

    /**
    * @brief            清除进度条及QLabel信息
    * @author           YuanLong
    * @return
    */
    void clearDialogInfo();

    // 开始与终止一个繁忙进度条
    void startBusyProgress();
    void endBusyProgress();

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
