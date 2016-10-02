#include "harddiskopen.h"
#include "ui_harddiskopen.h"
#include <QTemporaryDir>
#include <QMessageBox>
#include <QProcess>
#include "errordialog.h"

HardDiskOpen::HardDiskOpen(QWidget *parent, QString imageFile) :
    QDialog(parent),
    ui(new Ui::HardDiskOpen)
{
    ui->setupUi(this);
    this->mode=ImageFile::DirectMode;
    this->offset=-1;

    //Fill the list widget with partition info
    //1. delete %tempdir%/backup.log
    //2. launch testdisk in temp dir with:
    //testdisk /cmd c.img partition_i386,analyze,backup
    //3. Analyze the backup.log file:
    // * - boot, primary, P - non-boot primary, L - logical. All other out.
    //offset is Start * 512.
    QTemporaryDir tempDir;
    if (!tempDir.isValid())
    {
        QMessageBox::critical(this,"Error","Cannot make temporary directory. Operation failed.",QMessageBox::Ok);
        return;
    }
    QProcess executing;
    executing.setProcessChannelMode(QProcess::MergedChannels);
    executing.setWorkingDirectory(tempDir.path());
    executing.start("testdisk /cmd \""+imageFile+"\" partition_i386,analyze,backup");
    executing.waitForFinished();
    if (executing.exitCode()!=0)
    {
        QMessageBox::critical(this,"Error","TestDisk returned code "+QString::number(executing.exitCode())+" with:\n"+executing.readAllStandardOutput(),QMessageBox::Ok);
        return;

    }
    QFile plik(tempDir.path()+"/backup.log");
    if (!plik.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,"Error","Cannot open TestDisk's partition information ("+tempDir.path()+"/backup.log"+") file.",QMessageBox::Ok);
        return;
    }
    QString partTable=plik.readAll();
    plik.close();
    QStringList log=partTable.split('\n');
    ui->lbCHS->setText("CHS: "+log.at(0).split(" CHS ").last());
    ui->lwPartitions->clear();
    for (int i=1;i<log.count()-1;i++)
    {
        QString k=QString::number(i-1)+": ";
        QString typ=log.at(i).split(' ').last();
        if (typ=="*")
            k+= "Primary*";
        if (typ=="P")
            k+= "Primary";
        if (typ=="L")
            k+= "Logical";
        if ((typ=="E")||(typ=="X"))
            continue;
        typ=log.at(i).split("start=").last().split(',').first().trimmed();
        k=k+" at: "+typ+ " Size: ";
        typ=log.at(i).split("size=").last().split(',').first().trimmed();
        k=k+typ;
        typ=log.at(i).split("start=").last().split(',').first().trimmed();
        k=k+" Offset: "+QString::number(typ.toInt()*512);
        ui->lwPartitions->addItem(k);
    }
}

HardDiskOpen::~HardDiskOpen()
{
    delete ui;
}

void HardDiskOpen::on_cbCustomOffset_clicked(bool checked)
{
    ui->sbOffset->setEnabled(checked);
    ui->lwPartitions->setEnabled(!checked);
}

void HardDiskOpen::on_HardDiskOpen_accepted()
{
    if (ui->rbDirectMode->isChecked())
        this->mode=ImageFile::DirectMode;
    if (ui->rbReadOnly->isChecked())
        this->mode=ImageFile::ReadOnly;
    if (ui->rbTempMode->isChecked())
        this->mode=ImageFile::DefaultMode;
    this->offset=ui->sbOffset->value();
}

qint64 HardDiskOpen::getOffset()
{
    return this->offset;
}

ImageFile::HandleMode HardDiskOpen::getMode()
{
    return this->mode;
}

void HardDiskOpen::on_lwPartitions_currentRowChanged()
{
    QString a=ui->lwPartitions->currentItem()->text();
    ui->sbOffset->setValue(a.split(' ').last().trimmed().toInt());
}
