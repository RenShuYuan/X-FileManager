#include "dialog.h"
#include "ui_dialog.h"
#include "processdialog.h"

#include <QDir>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QPropertyAnimation>
#include <QDebug>

Dialog *Dialog::smInstance = NULL;

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    smInstance = this;

    setWindowFlags(Qt::Widget);

    isShutdown = false;

    //界面动画，改变透明度的方式出现0 - 1渐变
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(1000);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();

    initDialog();
    StyleSheet();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::initDialog()
{
    ui->tabWidget->setTabText(0, "拷贝文件夹");
    ui->tabWidget->setTabText(1, "拷贝文件");
    ui->tabWidget->setTabText(2, "替换文件");
    ui->tabWidget->setCurrentIndex(0);

    ui->lineEdit_5->setEnabled(false);
    ui->lineEdit_6->setEnabled(false);
    ui->lineEdit_7->setEnabled(false);
    ui->spinBox->setEnabled(false);
    ui->spinBox_2->setEnabled(false);
}

void Dialog::StyleSheet()
{
    QString ss;

    ss += QString( "* { font: 9pt \"微软雅黑\"} " );

    setStyleSheet(ss);

}

void Dialog::resolveFilterFile()
{
    ui->listWidget->clear();
    filters.clear();

    if (ui->lineEdit->text().isEmpty() ||
            ui->lineEdit_3->text().isEmpty())
        return;

    // 解析文件内容
    QFile file(ui->lineEdit_3->text());
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        filters.append(line);
        ui->listWidget->addItem( line );
    }
    file.close();
}

void Dialog::on_pushButton_clicked()
{
    // 读取保存的上一次路径
    QString strListPath = settings.value( "/lastPath", "." ).toString();

    //浏览原始文件夹
    QString dir = QFileDialog::getExistingDirectory(this, "打开源文件夹", strListPath,
                                                    QFileDialog::ShowDirsOnly |
                                                    QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
    {
        ui->lineEdit->setText(QDir::toNativeSeparators(dir)); // 将"/"转换为"\\"
        settings.setValue("/lastPath", dir);
    }
}

void Dialog::on_pushButton_2_clicked()
{
    // 读取保存的上一次路径
    QString strListPath = settings.value( "/lastPath", "." ).toString();

    //浏览目标文件夹
    QString dir = QFileDialog::getExistingDirectory(this, "打目标文件夹", strListPath,
                                                    QFileDialog::ShowDirsOnly |
                                                    QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
    {
        ui->lineEdit_2->setText(QDir::toNativeSeparators(dir)); // 将"/"转换为"\\"
        settings.setValue("/lastPath", dir);
    }
}

void Dialog::on_pushButton_3_clicked()
{
    const QString str_lineEdit = QDir::fromNativeSeparators(ui->lineEdit->text());    // 将"\\"转换为"/"
    const QString str_lineEdit2 = QDir::fromNativeSeparators(ui->lineEdit_2->text()); // 将"\\"转换为"/"
    if (str_lineEdit.isEmpty() || str_lineEdit2.isEmpty())
    {
        QMessageBox::warning(this, "错误", "原始文件夹路径与目标文件夹路径不能为空!");
        return;
    }
    QDir dir;
    if (!dir.exists(str_lineEdit) || !dir.exists(str_lineEdit2))
    {
        QMessageBox::warning(this, "错误", "检测到原始文件夹路径或目标文件夹路径为无效路径!\n已终止处理");
        return;
    }

    ProcessDialog* dialog = new ProcessDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->open();

    if (ui->tabWidget->currentIndex()==0)
    {
        // 执行文件夹拷贝
        dialog->processCopyFolder(str_lineEdit,
                                  str_lineEdit2,
                                  filters,
                                  ui->lineEdit_4->text(),
                                  ui->checkBox->isChecked());
    }
    else if (ui->tabWidget->currentIndex()==1)
    {
        // 执行文件拷贝
        dialog->processCopyFlies(str_lineEdit,
                                 str_lineEdit2,
                                 filters,
                                 ui->checkBox->isChecked(),
                                 ui->groupBox_8->isChecked(),
                                 ui->checkBox_6->isChecked(),
                                 ui->checkBox_10->isChecked(), ui->lineEdit_5->text(),
                                 ui->checkBox_11->isChecked(), ui->spinBox->value(), ui->spinBox_2->value(),
                                 ui->checkBox_12->isChecked(), ui->lineEdit_6->text(), ui->lineEdit_7->text());
    }
    else if (ui->tabWidget->currentIndex()==2)
    {
        // 执行文件替换
        dialog->processReplaceFile(str_lineEdit, str_lineEdit2,
                                   ui->checkBox_4->isChecked() ? Qt::CaseInsensitive : Qt::CaseSensitive,
                                   ui->checkBox_2->isChecked());
    }
}

void Dialog::on_pushButton_4_clicked()
{
    // 读取保存的上一次路径
    QString strListPath = settings.value( "/lastPath", "." ).toString();

    QString fileName = QFileDialog::getOpenFileName(this, "打开内容过滤列表文件", strListPath, "文本文件 (*.txt)");
    if (!fileName.isEmpty())
    {
        ui->lineEdit_3->setText(QDir::toNativeSeparators(fileName));     // 将"/"转换为"\\"
        settings.setValue("/lastPath", QFileInfo(fileName).path());
    }
    else
        return;

    resolveFilterFile();
}

void Dialog::on_lineEdit_textChanged(const QString &arg1)
{
    if (ui->tabWidget->currentIndex()==0)
    {
        // 执行文件夹拷贝
        resolveFilterFile();
    }
    else if (ui->tabWidget->currentIndex()==1)
    {
        // 执行文件拷贝
        resolveFilterFile();
    }
}

void Dialog::on_lineEdit_3_textChanged(const QString &arg1)
{
    // 执行文件夹拷贝
    resolveFilterFile();
}

void Dialog::on_checkBox_3_clicked(bool checked)
{
    isShutdown = checked;
}

void Dialog::on_tabWidget_currentChanged(int index)
{
    if (index == 2)
    {
        ui->groupBox_5->setEnabled(false);
    }
    else
    {
        ui->groupBox_5->setEnabled(true);
    }
}

void Dialog::on_checkBox_10_clicked(bool checked)
{
    ui->lineEdit_5->setEnabled(checked);
}

void Dialog::on_checkBox_11_clicked(bool checked)
{
    ui->spinBox->setEnabled(checked);
    ui->spinBox_2->setEnabled(checked);
}

void Dialog::on_checkBox_12_clicked(bool checked)
{
    ui->lineEdit_6->setEnabled(checked);
    ui->lineEdit_7->setEnabled(checked);
}
