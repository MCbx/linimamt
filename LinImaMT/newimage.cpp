#include "newimage.h"
#include "ui_newimage.h"
#include <qmath.h>


//A little explanation:
// All these radio buttons set parameters in boxes below.
// Even if they are disabled
// Then the data from these boxes is returned to image formatter.


newImage::newImage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::newImage)
{
    ui->setupUi(this);
    this->dismantleCustom(0);
    this->on_sbSectorSize_valueChanged(ui->sbSectorSize->value());
    this->on_sbClust_valueChanged(ui->sbClust->value());
    this->result="";
}

newImage::~newImage()
{
    delete ui;
}

void newImage::dismantleCustom(bool b)
{
    ui->cb2M->setEnabled(b);
    ui->sb2Msize->setEnabled(b);
    ui->sbClust->setEnabled(b);
    ui->sbCyls->setEnabled(b);
    ui->sbHeads->setEnabled(b);
    ui->sbSectorSize->setEnabled(b);
    ui->sbSPT->setEnabled(b);
}

void newImage::on_sbSectorSize_valueChanged(int arg1)
{
    ui->lbBytesPerSector->setText(QString::number(pow(2,arg1+7))+"B");
    this->on_sbClust_valueChanged(ui->sbClust->value());
}

void newImage::on_sbClust_valueChanged(int arg1)
{
    ui->lbClustBytes->setText(QString::number(arg1*ui->lbBytesPerSector->text().replace("B","").toInt())+"B");
    this->updateImageSize();
}

void newImage::updateImageSize()
{
    int a=ui->sbCyls->value()*ui->sbHeads->value()*ui->sbSPT->value()*ui->lbBytesPerSector->text().replace("B","").toInt();
    ui->lbImaSize->setText("Image size: "+QString::number(a)+"Bytes");
}

void newImage::on_radioButton_13_clicked()
{
    this->dismantleCustom(1);
    this->on_cb2M_clicked(ui->cb2M->isChecked());
}

void newImage::on_cb2M_clicked(bool checked)
{
    if (checked)
        ui->sb2Msize->setEnabled(1);
    else
        ui->sb2Msize->setEnabled(0);
}

void newImage::on_sbSPT_valueChanged(int arg1)
{
    this->updateImageSize();
}

void newImage::on_sbCyls_valueChanged(int arg1)
{
        this->updateImageSize();
}

void newImage::on_sbHeads_valueChanged(int arg1)
{
        this->updateImageSize();
}

void newImage::on_radioButton_2_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(40);
    ui->sbHeads->setValue(1);
    ui->sbSPT->setValue(8);
    ui->sbClust->setValue(1);
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_3_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(40);
    ui->sbHeads->setValue(1);
    ui->sbSPT->setValue(9);
    ui->sbClust->setValue(1);
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_4_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(40);
    ui->sbHeads->setValue(2);
    ui->sbSPT->setValue(8);
    ui->sbClust->setValue(2); //according to http://firmware.altervista.org/Data%20Encoding%20and%20Decoding.htm
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_5_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(40);
    ui->sbHeads->setValue(2);
    ui->sbSPT->setValue(9);
    ui->sbClust->setValue(2); //acording to http://firmware.altervista.org/Data%20Encoding%20and%20Decoding.htm
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_6_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(80);
    ui->sbHeads->setValue(2);
    ui->sbSPT->setValue(9);
    ui->sbClust->setValue(2);
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_7_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(80);
    ui->sbHeads->setValue(2);
    ui->sbSPT->setValue(15);
    ui->sbClust->setValue(1);
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_8_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(80);
    ui->sbHeads->setValue(2);
    ui->sbSPT->setValue(18);
    ui->sbClust->setValue(1);
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_9_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(80);
    ui->sbHeads->setValue(2);
    ui->sbSPT->setValue(36);
    ui->sbClust->setValue(2);
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_12_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(80);
    ui->sbHeads->setValue(2);
    ui->sbSPT->setValue(21);
    ui->sbClust->setValue(1);
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_10_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(80);
    ui->sbHeads->setValue(2);
    ui->sbSPT->setValue(21);
    ui->sbClust->setValue(2);
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_14_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(80);
    ui->sbHeads->setValue(2);
    ui->sbSPT->setValue(21);
    ui->sbClust->setValue(4);
    ui->sbSectorSize->setValue(2);
}

void newImage::on_radioButton_11_clicked()
{
    this->dismantleCustom(0);
    ui->cb2M->setChecked(0);
    ui->sb2Msize->setEnabled(0);
    ui->sbCyls->setValue(82);
    ui->sbHeads->setValue(2);
    ui->sbSPT->setValue(21);
    ui->sbClust->setValue(1);
    ui->sbSectorSize->setValue(2);
}

void newImage::on_newImage_accepted()
{
    //Assemble string to be passed into image formatter
    this->result="-C -t "+QString::number(ui->sbCyls->value())+" -h "+QString::number(ui->sbHeads->value());
    this->result+=" -s "+QString::number(ui->sbSPT->value())+" -c "+QString::number(ui->sbClust->value());
    this->result+=" -S "+QString::number(ui->sbSectorSize->value());
    if ((ui->cb2M->isChecked())&&(ui->cb2M->isEnabled()))
    {
        this->result+=" -2 "+QString::number(ui->sb2Msize->value());
    }
}
