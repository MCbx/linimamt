#include "fileviewer.h"
#include "ui_fileviewer.h"
#include <QSettings>
#include <QFileDialog>

fileViewer::fileViewer(QString source, QString settingsPath, QString fileName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fileViewer)
{
    this->name=fileName;
    this->configu=settingsPath;

    ui->setupUi(this);

    //1. Read and apply configuration
    QSettings settings(settingsPath,QSettings::IniFormat);
    settings.beginGroup("Viewer");

    ui->cbWordWrap->setChecked(settings.value("Wrap",1).toBool());

    if (ui->cbWordWrap->isChecked())
        ui->textContents->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    else
        ui->textContents->setLineWrapMode(QPlainTextEdit::NoWrap);

    int indx = settings.value("DType",0).toInt();
    ui->cbDispType->setCurrentIndex(indx);

    settings.endGroup();


    //2. Read file.
    QFile plik(source);
    if (!plik.open(QIODevice::ReadOnly))
    {
        return;
    }
    this->data = plik.readAll();
    plik.close();

    //3. Show titlebar text
    this->setWindowTitle(fileName+" - Preview");

    //4. Show file
    this->showFile();
}

fileViewer::~fileViewer()
{
    delete ui;
}

void fileViewer::on_cbWordWrap_clicked(bool checked)
{
    if (checked)
        ui->textContents->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    else
        ui->textContents->setLineWrapMode(QPlainTextEdit::NoWrap);
    this->saveSettings();
}

void fileViewer::on_cbDispType_currentIndexChanged(int index)
{
    this->showFile();
    this->saveSettings();
}

void fileViewer::on_btnOpen_clicked()
{
    //Check validity of combo box
    //If bad - fail.
    //If ok - save CBox state
    //Fire the process up.
}

void fileViewer::on_btnExport_clicked()
{

    QString fname = QFileDialog::getSaveFileName(this,"Export file",this->name,"All files (*)");
    if (fname!="")
    {
        QFile file(fname);
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();
    }
}

void fileViewer::showFile()
{
    ui->textContents->clear();
    if (ui->cbDispType->currentIndex()==0)
    {
        ui->textContents->appendPlainText(QString::fromLatin1(this->data));
        ui->textContents->moveCursor(QTextCursor::Start);
    }

    if (ui->cbDispType->currentIndex()==1)
    {
        QString linia;
        ui->textContents->appendPlainText("        00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
          for (int i=0;i<this->data.size();i=i+16)
          {
              linia=QString::number(i,16).rightJustified(6,'0');
              linia+=":";
              for (int k=0;k<16;k++)
              {
                  linia=linia+" "+QString::number((unsigned char)this->data[k+i],16).rightJustified(2,'0');
              }
              linia+=" | ";
              for (int k=0;k<16;k++)
              {
                  if ((this->data[k+i]<char(32))||(this->data[k+i]>char(126)))
                      linia=linia+".";
                  else
                      linia=linia+this->data[k+i];
              }

              linia+"\n";
              ui->textContents->appendPlainText(linia);
          }
          ui->textContents->moveCursor(QTextCursor::Start);
    }

}

void fileViewer::saveSettings()
{
    //save settings back to ini file
    QSettings settings(this->configu,QSettings::IniFormat);
    settings.beginGroup("Viewer");
    settings.setValue("DType",ui->cbDispType->currentIndex());
    settings.setValue("Wrap",ui->cbWordWrap->isChecked());
    settings.endGroup();
}
