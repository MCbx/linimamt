#include "mbrsector.h"
#include "ui_mbrsector.h"
#include <QMessageBox>
#include <QFileDialog>

MBRSector::MBRSector(QWidget *parent, ImageFile * image, qint64 offset, int length, bool ReadOnly) :
    QDialog(parent),
    ui(new Ui::MBRSector)
{
    this->image=image;
    if (offset==-1)
        this->offset=image->getOffset();
    else
        this->offset=offset;
    this->length=length;
    ui->setupUi(this);
    this->modified=0;

    //1. Read first length bytes from img->getcurrentpath starting from offset into data array:
    QFile img(this->image->getCurrentPath());
    if (!img.open(QIODevice::ReadOnly))
    {
        ui->teHexView->clear();
        ui->teHexView->appendPlainText("ERROR: File cannot be read: "+this->image->getCurrentPath());
        return;
    }
    img.seek(this->offset);
    char* temp=new char[this->length];
    int a = img.read(temp,this->length);
    this->sectorData=QByteArray(temp,a);
    img.close();

    if (ReadOnly==1)
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    }
    this->refreshView();
}

MBRSector::~MBRSector()
{
    delete ui;
}

//Save to file
void MBRSector::on_pushButton_2_clicked()
{
    //open save dialog
    QString fname = QFileDialog::getSaveFileName(this,"Save MBR sector as","","Binary (*.bin);;All files (*)");
    if (!fname.endsWith(".bin",Qt::CaseInsensitive))
    {
        fname=fname+".bin";
    }

    //save the dump to file.
    if (fname!=".bin")
    {
        QFile file(fname);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(this,"Error","File "+fname+" cannot be opened for writing.");
            return;
        }
        file.write(this->sectorData);
        file.close();
    }
}

//Load from file
void MBRSector::on_pushButton_clicked()
{
    //open load dialog
    QString fname = QFileDialog::getOpenFileName(this,"Load MBR sector","","Binary (*.bin);;All files (*)");

    if (fname=="")
        return;

    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,"Error","File "+fname+" cannot be opened for reading.");
        return;
    }

    //check length
    int size=file.size();
    if (size!=this->length)
    {
        QMessageBox::critical(this,"Error","File "+fname+" has size "+QString::number(size)+"B while sector has "+QString::number(this->length)+"B. File should be the same length as sector.");
        return;
    }

    //load the dump
    this->modified=1;
    file.seek(0);
    QByteArray newSector = file.readAll();
    file.close();

    //Copy the data
    for (int i=0;i<this->sectorData.length();i++)
    {
        //Perform all copy exceptions
        if ((ui->cbPreserveCode->isChecked()) && (i>=0) && (i<436) )
            continue;
        if ((ui->cbPreserveIdentifier->isChecked()) && (i>=436) && (i<446) )
            continue;
        if ((ui->cbPreservePart1->isChecked()) && (i>=446) && (i<462) )
            continue;
        if ((ui->cbPreservePart2->isChecked()) && (i>=462) && (i<478) )
            continue;
        if ((ui->cbPreservePart3->isChecked()) && (i>=478) && (i<494) )
            continue;
        if ((ui->cbPreservePart4->isChecked()) && (i>=494) && (i<510) )
            continue;
        if ((ui->cbPreserveSignature->isChecked()) && (i>=510) && (i<512) )
            continue;

        this->sectorData[i]=newSector.at(i);
    }

    this->refreshView();
}

void MBRSector::refreshView()
{
    //1. Paint data to text viewer:
    ui->teHexView->clear();
    ui->teHexView->appendPlainText("       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    QString linia;
    for (int i=0;i<this->sectorData.size();i=i+16)
    {
        linia=QString::number(i,16).rightJustified(5,'0');
        linia+=":";
        for (int k=0;k<16;k++)
        {
            linia=linia+" "+QString::number((unsigned char)this->sectorData[k+i],16).rightJustified(2,'0');
        }
        linia+=" | ";
        for (int k=0;k<16;k++)
        {
            if ((this->sectorData[k+i]<char(32))||(this->sectorData[k+i]>char(126)))
                linia=linia+".";
            else
                linia=linia+this->sectorData[k+i];
        }

        linia+"\n";
        ui->teHexView->appendPlainText(linia);
    }
    ui->teHexView->moveCursor(QTextCursor::Start);

    //2. Extract id
    QString str="";
    QString str2="";
    for (int i=436;i<446;i++)
    {
        str2+=this->sectorData.at(i);
        str+=QString::number((unsigned char)this->sectorData.at(i),16).rightJustified(2,'0')+" ";
    }
    ui->leIdentifier->setText(str.trimmed());
    ui->lbIdentText->setText(str2);

    //3. Extract signature
    str=QString::number((unsigned char)this->sectorData.at(510),16).rightJustified(2,'0')+" "+QString::number((unsigned char)this->sectorData.at(511),16).rightJustified(2,'0');
    ui->leSignature->setText(str);
    if (str.toUpper()!="55 AA")
        ui->leSignature->setStyleSheet("background-color:red;");
    else
        ui->leSignature->setStyleSheet("background-color:none;");


    //4. Extract partition table
}

