#include "attribute.h"
#include "ui_attribute.h"

Attribute::Attribute(QList<QString> situation, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Attribute)
{
    ui->setupUi(this);
    //parse situation - 12345 where
        //1 - if "-" then not a directory
        //2 - if "a" - archive, if "x" - multiple
        //3 - if "r" - readonly, if "x" - multiple
        //4 - if "h" - hidden, if "x" - multiple
        //5 - if "s" - system, if "x" - multiple
    char a=-1;
    char r=-1;
    char h=-1;
    char s=-1;
    for (int i=0;i<situation.count();i++)   //TODO: Simplify
    {
        //parse item
        if (situation.at(i).at(1)=='a')
        {
            if (a==-1)
                a=1;
            if (a==0)
                a=2; //mixed
        }
        if (situation.at(i).at(1)=='-')
        {
            if (a==-1)
                a=0;
            if (a==1)
                a=2; //mixed
        }

        if (situation.at(i).at(2)=='r')
        {
            if (r==-1)
                r=1;
            if (r==0)
                r=2; //mixed
        }
        if (situation.at(i).at(2)=='-')
        {
            if (r==-1)
                r=0;
            if (r==1)
                r=2; //mixed
        }

        if (situation.at(i).at(3)=='h')
        {
            if (h==-1)
                h=1;
            if (h==0)
                h=2; //mixed
        }
        if (situation.at(i).at(3)=='-')
        {
            if (h==-1)
                h=0;
            if (h==1)
                h=2; //mixed
        }

        if (situation.at(i).at(4)=='s')
        {
            if (s==-1)
                s=1;
            if (s==0)
                s=2; //mixed
        }
        if (situation.at(i).at(4)=='-')
        {
            if (s==-1)
                s=0;
            if (s==1)
                s=2; //mixed
        }
        if (situation.at(i).at(0)=='D')
        {
            ui->cbRecursive->setChecked(1);
        }
    }

    if (a==2) ui->cbArchive->setCheckState(Qt::PartiallyChecked);
    if (r==2) ui->cbReadOnly->setCheckState(Qt::PartiallyChecked);
    if (h==2) ui->cbHidden->setCheckState(Qt::PartiallyChecked);
    if (s==2) ui->cbSystem->setCheckState(Qt::PartiallyChecked);
    if (a==1) ui->cbArchive->setChecked(1);
    if (r==1) ui->cbReadOnly->setChecked(1);
    if (h==1) ui->cbHidden->setChecked(1);
    if (s==1) ui->cbSystem->setChecked(1);
    if (ui->cbArchive->checkState()!=Qt::PartiallyChecked) ui->cbArchive->setTristate(0);
    if (ui->cbReadOnly->checkState()!=Qt::PartiallyChecked) ui->cbReadOnly->setTristate(0);
    if (ui->cbHidden->checkState()!=Qt::PartiallyChecked) ui->cbHidden->setTristate(0);
    if (ui->cbSystem->checkState()!=Qt::PartiallyChecked) ui->cbSystem->setTristate(0);
}


Attribute::~Attribute()
{
    delete ui;
}

void Attribute::on_buttonBox_accepted()
{
    if (ui->cbRecursive->checkState()==(Qt::Checked))
        this->result="D";
    else
        this->result="-";

    if (ui->cbArchive->checkState()==Qt::Checked)
        result+="a";
    if (ui->cbArchive->checkState()==Qt::Unchecked)
        result+="-";
    if (ui->cbArchive->checkState()==Qt::PartiallyChecked)
        result+="X";

    if (ui->cbReadOnly->checkState()==Qt::Checked)
        result+="r";
    if (ui->cbReadOnly->checkState()==Qt::Unchecked)
        result+="-";
    if (ui->cbReadOnly->checkState()==Qt::PartiallyChecked)
        result+="X";

    if (ui->cbHidden->checkState()==Qt::Checked)
        result+="h";
    if (ui->cbHidden->checkState()==Qt::Unchecked)
        result+="-";
    if (ui->cbHidden->checkState()==Qt::PartiallyChecked)
        result+="X";

    if (ui->cbSystem->checkState()==Qt::Checked)
        result+="s";
    if (ui->cbSystem->checkState()==Qt::Unchecked)
        result+="-";
    if (ui->cbSystem->checkState()==Qt::PartiallyChecked)
        result+="X";

}

void Attribute::on_cbReadOnly_clicked()
{

}
