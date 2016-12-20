#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include <QSettings>

optionsDialog::optionsDialog(qint64 *hdImgSize, QString *defViever, QList<QString> *exts, QList<QString> *viewers, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::optionsDialog)
{
    ui->setupUi(this);
    this->hdImgSize=hdImgSize;
    this->defViever=defViever;
    this->exts=exts;
    this->viewers=viewers;
    ui->sbHddSize->setValue(*((int*)(this->hdImgSize)));

    if (*this->defViever=="0")
    {
        ui->rbDefault->setChecked(1);
        this->on_rbDefault_clicked();
    }
    if (*this->defViever=="1")
    {
        ui->rbInternal->setChecked(1);
        this->on_rbInternal_clicked();
    }
    if ((*this->defViever!="0")&&(*this->defViever!="1"))
    {
        ui->rbApplication->setChecked(1);
        ui->leApplication->setText(*this->defViever);
    }
    this->repaintList();
}

optionsDialog::~optionsDialog()
{
    delete ui;
}

void optionsDialog::on_rbInternal_clicked()
{
    ui->leApplication->setEnabled(0);
    ui->leApplication->setText("1");
}

void optionsDialog::on_rbDefault_clicked()
{
    ui->leApplication->setEnabled(0);
    ui->leApplication->setText("0");
}

void optionsDialog::on_rbApplication_clicked()
{
    ui->leApplication->setEnabled(1);
    ui->leApplication->setText("");
}

void optionsDialog::on_optionsDialog_accepted()
{
    //save
    *this->hdImgSize=ui->sbHddSize->value();
    *this->defViever=ui->leApplication->text();

}

void optionsDialog::repaintList()
{
    int k=ui->lwExtensons->currentRow();
    ui->lwExtensons->clear();
    ui->leCommand->clear();
    ui->leExtensioins->clear();
    for (int i=0;i<this->exts->count();i++)
    {
        QString itm=QString::number(i)+" "+this->exts->at(i)+" "+this->viewers->at(i);
        ui->lwExtensons->addItem(itm);
    }
    if (k<ui->lwExtensons->count())
    {
        ui->lwExtensons->setCurrentRow(k);
        this->on_lwExtensons_currentRowChanged(k);
    }
}

void optionsDialog::on_lwExtensons_currentRowChanged(int currentRow)
{
    if (currentRow>-1)
    {
        ui->leCommand->setText(this->viewers->at(currentRow));
        ui->leExtensioins->setText(this->exts->at(currentRow));
    }
}

void optionsDialog::on_btnAdd_clicked()
{
    this->exts->append("EXT");
    this->viewers->append("command");
    this->repaintList();
}

void optionsDialog::on_btnRemove_clicked()
{
    if (ui->lwExtensons->currentRow()>-1)
    {
        this->exts->removeAt(ui->lwExtensons->currentRow());
        this->viewers->removeAt(ui->lwExtensons->currentRow());
        this->repaintList();
    }
}

void optionsDialog::on_leExtensioins_editingFinished()
{
    if (ui->lwExtensons->currentRow()==-1)
        return;

    this->exts->replace(ui->lwExtensons->currentRow(),ui->leExtensioins->text());
    this->repaintList();
}

void optionsDialog::on_leCommand_editingFinished()
{
    if (ui->lwExtensons->currentRow()==-1)
        return;
    this->viewers->replace(ui->lwExtensons->currentRow(),ui->leCommand->text());
    this->repaintList();
}
