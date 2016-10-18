#include <dialog.h>
#include "processdialog.h"
#include "ui_processdialog.h"
#include "myLibrary/qsfiles.h"
#include "myLibrary/filecopyspeedcalculates.h"
#include <QProcess>
#include <QFileInfo>
#include <QDebug>

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

void ProcessDialog::processCopyFolder(const QString &source, const QString &target, const QStringList &filters, bool ignoreExisted)
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

void ProcessDialog::processReplaceFile(const QString &source, const QString &target, Qt::CaseSensitivity cs, const bool &isMultiple)
{

}

void ProcessDialog::appendPlainText(const QString &text)
{
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
        QProcess myProcess;
        myProcess.start("shutdown.exe", QStringList()<<"/s"<<"/t"<<"60");
    }
}
