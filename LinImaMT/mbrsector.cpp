#include "mbrsector.h"
#include "ui_mbrsector.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStyle>

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
    this->modified=0;
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
    {
        ui->leSignature->setStyleSheet("background-color:red;");
        ui->leSignature->setToolTip("WARNING: Should be usually 55 AA");
    }
    else
    {
        ui->leSignature->setStyleSheet("background-color:none;");
        ui->leSignature->setToolTip("");
    }

    //4. Extract partition table

    //PART1
    ui->cbActive1->setTristate(0);
    if ((unsigned char)this->sectorData.at(446)==0)
    {
        ui->cbActive1->setChecked(0);
    }
    if ((unsigned char)this->sectorData.at(446)==128)
    {
        ui->cbActive1->setChecked(1);
    }
    if (((unsigned char)this->sectorData.at(446)!=128) && ((unsigned char)this->sectorData.at(446)!=0))
    {
        ui->cbActive1->setTristate(1);
        ui->cbActive1->setToolTip("0x"+QString::number((unsigned char)this->sectorData.at(446),16).rightJustified(2,'0'));
    }
    ui->sbStartH1->setValue((unsigned char)this->sectorData.at(447));

    ui->sbStartS1->setValue((unsigned char)this->sectorData.at(448)%64);
    ui->sbStartC1->setValue(((unsigned char)this->sectorData.at(448) - ((unsigned char)this->sectorData.at(448)%64))*4 + (unsigned char)this->sectorData.at(449) );

    ui->sbType1->setValue((unsigned char)this->sectorData.at(450));

    ui->sbEndH1->setValue((unsigned char)this->sectorData.at(451));

    ui->sbEndS1->setValue((unsigned char)this->sectorData.at(452)%64);
    ui->sbEndC1->setValue(((unsigned char)this->sectorData.at(452) - ((unsigned char)this->sectorData.at(452)%64))*4 + (unsigned char)this->sectorData.at(453) );

    unsigned int aa = (unsigned int)((unsigned char)sectorData.at(457) << 24 | (unsigned char)sectorData.at(456) << 16 | (unsigned char)sectorData.at(455) << 8 | (unsigned char)sectorData.at(454));
    ui->sbStartLBA1->setText(QString::number(aa));
    aa = (unsigned int)((unsigned char)sectorData.at(461) << 24 | (unsigned char)sectorData.at(460) << 16 | (unsigned char)sectorData.at(459) << 8 | (unsigned char)sectorData.at(458));
    ui->sbLength1->setText(QString::number(aa));

    //PART2
    ui->cbActive2->setTristate(0);
    if ((unsigned char)this->sectorData.at(462)==0)
    {
        ui->cbActive2->setChecked(0);
    }
    if ((unsigned char)this->sectorData.at(462)==128)
    {
        ui->cbActive2->setChecked(1);
    }
    if (((unsigned char)this->sectorData.at(462)!=128) && ((unsigned char)this->sectorData.at(462)!=0))
    {
        ui->cbActive2->setTristate(1);
        ui->cbActive2->setToolTip("0x"+QString::number((unsigned char)this->sectorData.at(462),16).rightJustified(2,'0'));
    }
    ui->sbStartH2->setValue((unsigned char)this->sectorData.at(463));

    ui->sbStartS2->setValue((unsigned char)this->sectorData.at(464)%64);
    ui->sbStartC2->setValue(((unsigned char)this->sectorData.at(464) - ((unsigned char)this->sectorData.at(464)%64))*4 + (unsigned char)this->sectorData.at(465) );

    ui->sbType2->setValue((unsigned char)this->sectorData.at(466));

    ui->sbEndH2->setValue((unsigned char)this->sectorData.at(467));

    ui->sbEndS2->setValue((unsigned char)this->sectorData.at(468)%64);
    ui->sbEndC2->setValue(((unsigned char)this->sectorData.at(468) - ((unsigned char)this->sectorData.at(468)%64))*4 + (unsigned char)this->sectorData.at(469) );

    aa = (unsigned int)((unsigned char)sectorData.at(473) << 24 | (unsigned char)sectorData.at(472) << 16 | (unsigned char)sectorData.at(471) << 8 | (unsigned char)sectorData.at(470));
    ui->sbStartLBA2->setText(QString::number(aa));
    aa = (unsigned int)((unsigned char)sectorData.at(477) << 24 | (unsigned char)sectorData.at(476) << 16 | (unsigned char)sectorData.at(475) << 8 | (unsigned char)sectorData.at(474));
    ui->sbLength2->setText(QString::number(aa));

    //PART3
    ui->cbActive3->setTristate(0);
    if ((unsigned char)this->sectorData.at(478)==0)
    {
        ui->cbActive3->setChecked(0);
    }
    if ((unsigned char)this->sectorData.at(478)==128)
    {
        ui->cbActive3->setChecked(1);
    }
    if (((unsigned char)this->sectorData.at(478)!=128) && ((unsigned char)this->sectorData.at(478)!=0))
    {
        ui->cbActive3->setTristate(1);
        ui->cbActive3->setToolTip("0x"+QString::number((unsigned char)this->sectorData.at(478),16).rightJustified(2,'0'));
    }
    ui->sbStartH3->setValue((unsigned char)this->sectorData.at(479));

    ui->sbStartS3->setValue((unsigned char)this->sectorData.at(480)%64);
    ui->sbStartC3->setValue(((unsigned char)this->sectorData.at(480) - ((unsigned char)this->sectorData.at(480)%64))*4 + (unsigned char)this->sectorData.at(481) );

    ui->sbType3->setValue((unsigned char)this->sectorData.at(482));

    ui->sbEndH3->setValue((unsigned char)this->sectorData.at(483));

    ui->sbEndS3->setValue((unsigned char)this->sectorData.at(484)%64);
    ui->sbEndC3->setValue(((unsigned char)this->sectorData.at(484) - ((unsigned char)this->sectorData.at(484)%64))*4 + (unsigned char)this->sectorData.at(485) );

    aa = (unsigned int)((unsigned char)sectorData.at(489) << 24 | (unsigned char)sectorData.at(488) << 16 | (unsigned char)sectorData.at(487) << 8 | (unsigned char)sectorData.at(486));
    ui->sbStartLBA3->setText(QString::number(aa));
    aa = (unsigned int)((unsigned char)sectorData.at(493) << 24 | (unsigned char)sectorData.at(492) << 16 | (unsigned char)sectorData.at(491) << 8 | (unsigned char)sectorData.at(490));
    ui->sbLength3->setText(QString::number(aa));

    //PART4
    ui->cbActive4->setTristate(0);
    if ((unsigned char)this->sectorData.at(494)==0)
    {
        ui->cbActive4->setChecked(0);
    }
    if ((unsigned char)this->sectorData.at(494)==128)
    {
        ui->cbActive4->setChecked(1);
    }
    if (((unsigned char)this->sectorData.at(494)!=128) && ((unsigned char)this->sectorData.at(494)!=0))
    {
        ui->cbActive4->setTristate(1);
        ui->cbActive4->setToolTip("0x"+QString::number((unsigned char)this->sectorData.at(494),16).rightJustified(2,'0'));
    }
    ui->sbStartH4->setValue((unsigned char)this->sectorData.at(495));

    ui->sbStartS4->setValue((unsigned char)this->sectorData.at(496)%64);
    ui->sbStartC4->setValue(((unsigned char)this->sectorData.at(496) - ((unsigned char)this->sectorData.at(496)%64))*4 + (unsigned char)this->sectorData.at(497) );

    ui->sbType4->setValue((unsigned char)this->sectorData.at(498));

    ui->sbEndH4->setValue((unsigned char)this->sectorData.at(499));

    ui->sbEndS4->setValue((unsigned char)this->sectorData.at(500)%64);
    ui->sbEndC4->setValue(((unsigned char)this->sectorData.at(500) - ((unsigned char)this->sectorData.at(500)%64))*4 + (unsigned char)this->sectorData.at(501) );

    aa = (unsigned int)((unsigned char)sectorData.at(505) << 24 | (unsigned char)sectorData.at(504) << 16 | (unsigned char)sectorData.at(503) << 8 | (unsigned char)sectorData.at(502));
    ui->sbStartLBA4->setText(QString::number(aa));
    aa = (unsigned int)((unsigned char)sectorData.at(509) << 24 | (unsigned char)sectorData.at(508) << 16 | (unsigned char)sectorData.at(507) << 8 | (unsigned char)sectorData.at(506));
    ui->sbLength4->setText(QString::number(aa));
}


void MBRSector::on_cbActive1_clicked()
{
    if (ui->cbActive1->isTristate())
        ui->cbActive1->setTristate(0);
    if (ui->cbActive1->isChecked())
        this->sectorData[446]=128;
    else
        this->sectorData[446]=0;
    this->modified=1;
    this->refreshView();
}

void MBRSector::on_cbActive2_clicked()
{
    if (ui->cbActive2->isTristate())
        ui->cbActive2->setTristate(0);
    if (ui->cbActive2->isChecked())
        this->sectorData[462]=128;
    else
        this->sectorData[462]=0;
    this->modified=1;
    this->refreshView();
}

void MBRSector::on_cbActive3_clicked()
{
    if (ui->cbActive3->isTristate())
        ui->cbActive3->setTristate(0);
    if (ui->cbActive3->isChecked())
        this->sectorData[478]=128;
    else
        this->sectorData[478]=0;
    this->modified=1;
    this->refreshView();
}

void MBRSector::on_cbActive4_clicked()
{
    if (ui->cbActive4->isTristate())
        ui->cbActive4->setTristate(0);
    if (ui->cbActive4->isChecked())
        this->sectorData[494]=128;
    else
        this->sectorData[494]=0;
    this->modified=1;
    this->refreshView();
}

void MBRSector::on_sbType1_valueChanged(int arg1)
{
    this->modified=1;
    this->sectorData[450]=arg1;
    this->refreshView();
}

void MBRSector::on_sbType2_valueChanged(int arg1)
{
    this->modified=1;
    this->sectorData[466]=arg1;
    this->refreshView();
}

void MBRSector::on_sbType3_valueChanged(int arg1)
{
    this->modified=1;
    this->sectorData[482]=arg1;
    this->refreshView();
}

void MBRSector::on_sbType4_valueChanged(int arg1)
{
    this->modified=1;
    this->sectorData[498]=arg1;
    this->refreshView();
}

void MBRSector::on_leSignature_editingFinished()
{
    this->modified=1;
    QString a=ui->leSignature->text().mid(0,2);
    unsigned char x = a.toInt(NULL,16);
    this->sectorData[510]=x;
    a=ui->leSignature->text().mid(3,2);
    x = a.toInt(NULL,16);
    this->sectorData[511]=x;
    this->refreshView();
}

void MBRSector::on_buttonBox_accepted()
{
    if (this->modified==0)
    {
        return;
    }

    //ask QMessageBox to make sure that user knows what's doing
    QMessageBox::StandardButton reply;
     reply = QMessageBox::question(this, "WARNING!", "The sector has been modified. Do you REALLY want to save it?\n (and know what you are doing)",
                                   QMessageBox::Yes|QMessageBox::No);
     if (reply == QMessageBox::No)
     {
        return;
     }

    //save sector
    this->image->prepareForModify();
    //Now the image supplies us modified path to temp file.

    //patch the image file with a new sector held in this->sectorData.
    QFile plik(this->image->getCurrentPath());
    if (!plik.open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(this,"Error","Image data cannot be opened for writing!");
    }
    plik.seek(this->offset);
    plik.write(this->sectorData);
    plik.close();
    this->image->forceModified(1);
}
