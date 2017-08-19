#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

    static Dialog *instance() { return smInstance; }

    bool shutdown() { return isShutdown; }

private slots:

    //! 拷贝文件夹
    // 源文件夹浏览
    void on_pushButton_clicked();
    // 目标文件夹浏览
    void on_pushButton_2_clicked();
    // 开始执行
    void on_pushButton_3_clicked();
    // 内容过滤
    void on_pushButton_4_clicked();
    // 原话路径
    void on_lineEdit_textChanged(const QString &arg1);
    // 文件过滤器路径
    void on_lineEdit_3_textChanged(const QString &arg1);

    void on_checkBox_3_clicked(bool checked);

    void on_tabWidget_currentChanged(int index);

    void on_checkBox_10_clicked(bool checked);

    void on_checkBox_11_clicked(bool checked);

    void on_checkBox_12_clicked(bool checked);
    void on_checkBox_6_clicked(bool checked);

    void on_checkBox_13_clicked(bool checked);

private:
    void initDialog();
    void StyleSheet();

    // 分析过滤文件, 将结果输出窗口
    void resolveFilterFile();

    static Dialog *smInstance;
    Ui::Dialog *ui;
    QStringList filters;
    QSettings settings;

    bool isShutdown;
};

#endif // DIALOG_H
