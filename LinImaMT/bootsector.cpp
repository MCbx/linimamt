#include "bootsector.h"
#include "ui_bootsector.h"
#include <QMessageBox>
#include <QFileDialog>


//This is a completely separate subroutine
//It must get from input a path to image.
//Then it takes its boot sector, visualizes in hex, displays OEM stron in another box.
//It allows to dump these 512 bytes into file or read another 512 bytes from file
//and apply it to the image.

bootSector::bootSector(QWidget *parent, ImageFile * image, int offset, int length) :
    QDialog(parent),
    ui(new Ui::bootSector)
{    
    this->image=image;
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
    ui->leSerial->setInputMask("HH HH HH HH");
    ui->leBIOSEnds->setInputMask("HH");

    this->refreshView();
}

bootSector::~bootSector()
{
    delete ui;
}

void bootSector::on_leOEMString_textEdited(const QString &arg1)
{
    this->modified=1;
    //patch dump

    QString e=arg1.leftJustified(8,' ');
    this->sectorData.replace(3,e.length(),e.toLatin1());

    this->refreshView();
}

void bootSector::on_leSerial_textEdited(const QString &arg1)
{
    this->modified=1;
    int k=ui->leSerial->cursorPosition();
    //patch dump

    //prepare byte array
    int j=0;
    QString arg=arg1;
    arg.replace(" ","");
    for (int i=arg.length()-1;i>0;i=i-2)
    {
        QString w = QString(arg.at(i-1))+QString(arg.at(i));
        char k=(unsigned char)w.toUInt(NULL,16);
        this->sectorData[39+j]=k;
        j++;
    }

    this->refreshView();
    ui->leSerial->setCursorPosition(k);
}

void bootSector::on_leLabel_textEdited(const QString &arg1)
{
    this->modified=1;
    //patch dump

    QString e=arg1.leftJustified(11,' ');
    this->sectorData.replace(43,e.length(),e.toLatin1());

    this->refreshView();
}

//save dump
void bootSector::on_pushButton_2_clicked()
{
    //open save dialog
    QString fname = QFileDialog::getSaveFileName(this,"Save boot sector as","","Binary (*.bin);;All files (*)");
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

//load dump
void bootSector::on_pushButton_clicked()
{
    //open load dialog
    QString fname = QFileDialog::getOpenFileName(this,"Load boot sector","","Binary (*.bin);;All files (*)");

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

    int getBlock=ui->leBIOSEnds->text().toUpper().toInt(NULL,16);
    QByteArray BIOSBlock;
    //check if user entered proper value
    if (ui->cbPreserveBIOS->isChecked())
    {
        if ((getBlock<11)||(this->sectorData.length()<getBlock))
        {
            QMessageBox::critical(this,"Error","Wrong byte count entered into editor. It exceeds the sector size or is smaller than starting address.");
            return;
        }
        BIOSBlock=this->sectorData.mid(11,getBlock-11);
    }

    //load the dump
    this->modified=1;
    file.seek(0); //After calling size() - Qt4 bug on BSD.
    this->sectorData=file.readAll();
    file.close();

    QString label=ui->leLabel->text();
    QString oem = ui->leOEMString->text();
    QString serial=ui->leSerial->text();

    this->refreshView();

    //optionally patch the dump with preserved data
    if (ui->cbPreserveBIOS->isChecked())
    {
        this->sectorData.replace(11,BIOSBlock.length(),BIOSBlock);
        this->refreshView();
    }
    if ((ui->cbPreserveLabel->isEnabled())&&(ui->cbPreserveLabel->isChecked()))
    {
        ui->leLabel->setText(label);
        this->on_leLabel_textEdited(label);
    }
    if (ui->cbPreserveOEM->isChecked())
    {
        ui->leOEMString->setText(oem);
        this->on_leOEMString_textEdited(oem);
    }
    if ((ui->cbPreserveSerial->isEnabled())&&(ui->cbPreserveSerial->isChecked()))
    {
        ui->leSerial->setText(serial);
        this->on_leSerial_textEdited(serial);
    }
}

//re-freshes all view
void bootSector::refreshView()
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

    //2. Extract oem string
    QString str="";
    for (int i=3;i<11;i++)
    {
        str+=this->sectorData.at(i);
    }
    ui->leOEMString->setText(str.trimmed());

    //3. Extract sn string
    str="";
    for (int i=39;i<43;i++)
    {
        str=QString::number((unsigned char)this->sectorData.at(i),16).toUpper().rightJustified(2,'0')+str;
    }
    ui->leSerial->setText(str);


    //4. Extract label
    str="";
    for (int i=43;i<54;i++)
    {
        str+=this->sectorData.at(i);
    }
    ui->leLabel->setText(str.trimmed());

    //5. Extract end of BIOS block
    int jumpTo=this->sectorData.at(1)+2;
    ui->leBIOSEnds->setText(QString::number(jumpTo,16).rightJustified(2,'0').toUpper());
}

void bootSector::on_bootSector_accepted()
{
    if (this->modified==0)
    {
        return;
    }

    //save boot sector
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
    //QMessageBox::critical(this,"Error","File ");
}

void bootSector::on_cbPreserveBIOS_clicked()
{
    if (ui->cbPreserveBIOS->isChecked())
    {
        ui->cbPreserveLabel->setEnabled(0);
        ui->cbPreserveLabel->setChecked(1);
        ui->cbPreserveSerial->setEnabled(0);
        ui->cbPreserveSerial->setChecked(1);
        ui->leBIOSEnds->setEnabled(1);
     } else {
        ui->cbPreserveLabel->setEnabled(1);
        ui->cbPreserveLabel->setChecked(0);
        ui->cbPreserveSerial->setEnabled(1);
        ui->cbPreserveSerial->setChecked(0);
        ui->leBIOSEnds->setEnabled(0);
    }
}
