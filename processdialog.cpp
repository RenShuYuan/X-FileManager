#include <dialog.h>
#include "processdialog.h"
#include "ui_processdialog.h"
#include "myLibrary/qsfiles.h"
#include "myLibrary/filecopyspeedcalculates.h"
#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include <windows.h>

ProcessDialog::ProcessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProcessDialog)
{
    ui->setupUi(this);

    // 重置进度条
    ui->progressBar->reset();
    ui->progressBar_2->reset();
    // 进度条对齐方式
    ui->progressBar->setAlignment(Qt::AlignCenter);
    ui->progressBar_2->setAlignment(Qt::AlignCenter);
}

ProcessDialog::~ProcessDialog()
{
    emit stopThread();

    thread.quit();
    thread.wait();

    thread1.quit();
    thread1.wait();

    delete ui;
}

void ProcessDialog::processCopyFolder(const QString &source, const QString &target, const QStringList &filters, const QString &extList, bool ignoreExisted)
{
    // 实例化对象
    fileCopySpeedCalculates *speed = new fileCopySpeedCalculates();
    speed->moveToThread(&thread1);
    connect(&thread1, &QThread::started, speed, &fileCopySpeedCalculates::rateCalculation);
    connect(&thread1, &QThread::finished, &thread1, &QThread::deleteLater);
    connect(this, &ProcessDialog::stopThread, speed, &fileCopySpeedCalculates::stop, Qt::DirectConnection);
    connect(speed, &fileCopySpeedCalculates::setLabelText_2, ui->label_2, &QLabel::setText);

    // 实例化数据处理对象
    QSFiles *mQSFiles = new QSFiles();
    mQSFiles->setSourcePath(source);
    mQSFiles->setTargetPath(target);
    mQSFiles->setFilters(filters);
    mQSFiles->setExtList(extList);
    mQSFiles->setExisted(ignoreExisted);

    mQSFiles->moveToThread(&thread);
    connect(&thread, &QThread::started, mQSFiles, &QSFiles::batchCopyFolder);
    connect(&thread, &QThread::finished, &thread, &QThread::deleteLater);
    connect(this, &ProcessDialog::stopThread, mQSFiles, &QSFiles::stop, Qt::QueuedConnection);

    connect(mQSFiles, &QSFiles::postMessage, this, &ProcessDialog::appendPlainText);
    connect(mQSFiles, &QSFiles::finished, this, &ProcessDialog::shutdown, Qt::QueuedConnection);
    connect(mQSFiles, &QSFiles::finished, this, &ProcessDialog::clearDialogInfo, Qt::QueuedConnection);
    connect(mQSFiles, &QSFiles::setLabelText, ui->label, &QLabel::setText);

    connect(mQSFiles, &QSFiles::setVol, speed, &fileCopySpeedCalculates::setVol, Qt::DirectConnection);
    connect(mQSFiles, &QSFiles::finished, speed, &fileCopySpeedCalculates::stop, Qt::DirectConnection);
    connect(speed, &fileCopySpeedCalculates::bytReset, mQSFiles, &QSFiles::setByteReset, Qt::DirectConnection);

    connect(mQSFiles, &QSFiles::startBusyProgress, this, &ProcessDialog::startBusyProgress);
    connect(mQSFiles, &QSFiles::endBusyProgress, this, &ProcessDialog::endBusyProgress);

    connect(mQSFiles, &QSFiles::setVisiblePd, ui->progressBar, &QProgressBar::setVisible);
    connect(mQSFiles, &QSFiles::setMaximumPd, ui->progressBar, &QProgressBar::setMaximum);
    connect(mQSFiles, &QSFiles::setValuePd, ui->progressBar, &QProgressBar::setValue);

    connect(mQSFiles, &QSFiles::setVisiblePd_2, ui->progressBar_2, &QProgressBar::setVisible);
    connect(mQSFiles, &QSFiles::setMaximumPd_2, ui->progressBar_2, &QProgressBar::setMaximum);
    connect(mQSFiles, &QSFiles::setValuePd_2, ui->progressBar_2, &QProgressBar::setValue);

    thread1.start();
    thread.start();
}

void ProcessDialog::processCopyFlies(const QString &source, const QString &target,
                      const QStringList &filters,
                      const bool ignoreExisted,
                      const bool isCopyFilesToFolder,
                      const bool isAutoCreatFolder,
                      const bool isNotIncludedString, const QString& notIncludedString,
                      const bool isSubStringCreatFolder, const int startIndex, const int endIndex,
                      const bool isAddString, const QString& beforeString, const QString& afterString)
{
    // 实例化对象
    fileCopySpeedCalculates *speed = new fileCopySpeedCalculates();
    speed->moveToThread(&thread1);
    connect(&thread1, &QThread::started, speed, &fileCopySpeedCalculates::rateCalculation);
    connect(&thread1, &QThread::finished, &thread1, &QThread::deleteLater);
    connect(this, &ProcessDialog::stopThread, speed, &fileCopySpeedCalculates::stop, Qt::DirectConnection);
    connect(speed, &fileCopySpeedCalculates::setLabelText_2, ui->label_2, &QLabel::setText);

    // 实例化数据处理对象
    QSFiles *mQSFiles = new QSFiles();
    mQSFiles->setSourcePath(source);
    mQSFiles->setTargetPath(target);
    mQSFiles->setFilters(filters);
    mQSFiles->setExisted(ignoreExisted);
    mQSFiles->setCopyFilesToFolder(isCopyFilesToFolder);
    mQSFiles->setAutoCreatFolder(isAutoCreatFolder);
    mQSFiles->setNotIncludedString(isNotIncludedString, notIncludedString);
    mQSFiles->setSubString(isSubStringCreatFolder, startIndex, endIndex);
    mQSFiles->setAddString(isAddString, beforeString, afterString);

    mQSFiles->moveToThread(&thread);
    connect(&thread, &QThread::started, mQSFiles, &QSFiles::batchCopyFlies);
    connect(&thread, &QThread::finished, &thread, &QThread::deleteLater);
    connect(this, &ProcessDialog::stopThread, mQSFiles, &QSFiles::stop, Qt::QueuedConnection);

    connect(mQSFiles, &QSFiles::postMessage, this, &ProcessDialog::appendPlainText);
    connect(mQSFiles, &QSFiles::finished, this, &ProcessDialog::shutdown, Qt::QueuedConnection);
    connect(mQSFiles, &QSFiles::finished, this, &ProcessDialog::clearDialogInfo, Qt::QueuedConnection);
    connect(mQSFiles, &QSFiles::setLabelText, ui->label, &QLabel::setText);

    connect(mQSFiles, &QSFiles::setVol, speed, &fileCopySpeedCalculates::setVol, Qt::DirectConnection);
    connect(mQSFiles, &QSFiles::finished, speed, &fileCopySpeedCalculates::stop, Qt::DirectConnection);
    connect(speed, &fileCopySpeedCalculates::bytReset, mQSFiles, &QSFiles::setByteReset, Qt::DirectConnection);

    connect(mQSFiles, &QSFiles::startBusyProgress, this, &ProcessDialog::startBusyProgress);
    connect(mQSFiles, &QSFiles::endBusyProgress, this, &ProcessDialog::endBusyProgress);

    connect(mQSFiles, &QSFiles::setVisiblePd, ui->progressBar, &QProgressBar::setVisible);
    connect(mQSFiles, &QSFiles::setMaximumPd, ui->progressBar, &QProgressBar::setMaximum);
    connect(mQSFiles, &QSFiles::setValuePd, ui->progressBar, &QProgressBar::setValue);

    connect(mQSFiles, &QSFiles::setVisiblePd_2, ui->progressBar_2, &QProgressBar::setVisible);
    connect(mQSFiles, &QSFiles::setMaximumPd_2, ui->progressBar_2, &QProgressBar::setMaximum);
    connect(mQSFiles, &QSFiles::setValuePd_2, ui->progressBar_2, &QProgressBar::setValue);

    thread1.start();
    thread.start();
}

void ProcessDialog::processReplaceFile(const QString &source, const QString &target, Qt::CaseSensitivity cs, const bool &isMultiple)
{
    // 实例化对象
    fileCopySpeedCalculates *speed = new fileCopySpeedCalculates();
    speed->moveToThread(&thread1);
    connect(&thread1, &QThread::started, speed, &fileCopySpeedCalculates::rateCalculation);
    connect(&thread1, &QThread::finished, &thread1, &QThread::deleteLater);
    connect(this, &ProcessDialog::stopThread, speed, &fileCopySpeedCalculates::stop, Qt::DirectConnection);
    connect(speed, &fileCopySpeedCalculates::setLabelText_2, ui->label_2, &QLabel::setText);

    // 实例化数据处理对象
    QSFiles *mQSFiles = new QSFiles();
    mQSFiles->setSourcePath(source);
    mQSFiles->setTargetPath(target);
    mQSFiles->setCaseSensitivity(cs);
    mQSFiles->setMultiple(isMultiple);

    mQSFiles->moveToThread(&thread);
    connect(&thread, &QThread::started, mQSFiles, &QSFiles::batchReplaceFile);
    connect(&thread, &QThread::finished, &thread, &QThread::deleteLater);
    connect(this, &ProcessDialog::stopThread, mQSFiles, &QSFiles::stop, Qt::QueuedConnection);

    connect(mQSFiles, &QSFiles::postMessage, this, &ProcessDialog::appendPlainText);
    connect(mQSFiles, &QSFiles::finished, this, &ProcessDialog::shutdown, Qt::QueuedConnection);
    connect(mQSFiles, &QSFiles::finished, this, &ProcessDialog::clearDialogInfo, Qt::QueuedConnection);
    connect(mQSFiles, &QSFiles::setLabelText, ui->label, &QLabel::setText);

    connect(mQSFiles, &QSFiles::setVol, speed, &fileCopySpeedCalculates::setVol, Qt::DirectConnection);
    connect(mQSFiles, &QSFiles::finished, speed, &fileCopySpeedCalculates::stop, Qt::DirectConnection);
    connect(speed, &fileCopySpeedCalculates::bytReset, mQSFiles, &QSFiles::setByteReset, Qt::DirectConnection);

    connect(mQSFiles, &QSFiles::startBusyProgress, this, &ProcessDialog::startBusyProgress);
    connect(mQSFiles, &QSFiles::endBusyProgress, this, &ProcessDialog::endBusyProgress);

    connect(mQSFiles, &QSFiles::setVisiblePd, ui->progressBar, &QProgressBar::setVisible);
    connect(mQSFiles, &QSFiles::setMaximumPd, ui->progressBar, &QProgressBar::setMaximum);
    connect(mQSFiles, &QSFiles::setValuePd, ui->progressBar, &QProgressBar::setValue);

    connect(mQSFiles, &QSFiles::setVisiblePd_2, ui->progressBar_2, &QProgressBar::setVisible);
    connect(mQSFiles, &QSFiles::setMaximumPd_2, ui->progressBar_2, &QProgressBar::setMaximum);
    connect(mQSFiles, &QSFiles::setValuePd_2, ui->progressBar_2, &QProgressBar::setValue);

    thread1.start();
    thread.start();
}

void ProcessDialog::appendPlainText(const QString &text, const QColor &color)
{
    QTextCharFormat modifier;
    modifier.setForeground(QBrush(color));
    ui->plainTextEdit->mergeCurrentCharFormat(modifier);

    ui->plainTextEdit->appendPlainText(text);
}

void ProcessDialog::clearDialogInfo()
{
    ui->label->setText("");
    ui->label_2->setText("");
    ui->progressBar->setValue(0);
    ui->progressBar_2->setValue(0);
}

void ProcessDialog::startBusyProgress()
{
    ui->progressBar_2->reset();
    ui->progressBar_2->setRange(0, 0);
}

void ProcessDialog::endBusyProgress()
{
    ui->progressBar_2->setRange(0, 100);
    ui->progressBar_2->reset();
}

void ProcessDialog::on_pushButton_clicked()
{
    if (thread.isRunning())
        emit stopThread();
}

void ProcessDialog::shutdown()
{
    if (Dialog::instance()->shutdown())
    {
        HANDLE hToken;
        TOKEN_PRIVILEGES tkp;

        //获取进程标志
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
            return;
        }
        //获取关机特权的LUID
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,    &tkp.Privileges[0].Luid);
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        //获取这个进程的关机特权
        AdjustTokenPrivileges(hToken, false, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
        if (GetLastError() != ERROR_SUCCESS)
        {
            return;
        }
        // 强制关闭计算机
        if ( !ExitWindowsEx(EWX_SHUTDOWN , 0))  //关机
        {
            return;
        }
    }
}
