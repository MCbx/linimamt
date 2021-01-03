#include "fileviewer.h"
#include "ui_fileviewer.h"
#include <QSettings>
#include <QFileDialog>
#include <QProcess>
#include <QSizePolicy>

fileViewer::fileViewer(QString source, QString settingsPath, QString fileName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fileViewer)
{
    this->name=fileName;
    this->path=source;
    this->configu=settingsPath;

    ui->setupUi(this);

    //1. Read and apply configuration
    QSettings settings(this->configu,QSettings::IniFormat);
    settings.beginGroup("Viewer");

    //restore window dimensions from settings, the first thing we do
    int k=settings.value("W",0).toInt();
    if (k>0)
    {
        this->resize(k,this->height());
    }
    k=settings.value("H",0).toInt();
    if (k>0)
    {
        this->resize(this->width(),k);
    }

    ui->cbWordWrap->setChecked(settings.value("Wrap",1).toBool());

    if (ui->cbWordWrap->isChecked())
        ui->textContents->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    else
        ui->textContents->setLineWrapMode(QPlainTextEdit::NoWrap);

    int indx = settings.value("DType",0).toInt();
    ui->cbDispType->setCurrentIndex(indx);



    for (int i=0;i<10;i++)
    {
        QString prog=settings.value("Recent"+QString::number(i),"").toString();
        this->programs.append(prog);
        if (this->programs.at(i)!="")
        {
            ui->cbPrograms->addItem(this->programs.at(i));
        }
    }
    settings.endGroup();


    //2. Read file.
    QFile plik(this->path);
    if (!plik.open(QIODevice::ReadOnly))
    {
        ui->textContents->appendHtml("<font color=#FF0000 size=5><b>ERROR OPENING FILE</b></font>");
        return;
    }
    this->data = plik.readAll();
    plik.close();

    //3. Show titlebar text
    this->setWindowTitle(this->name+" - Preview");

    //4. Show file
    this->showFile();

    //ui->lbType->setWordWrap(1);

    //5. Fill the type
    QProcess fileproc;
    fileproc.setProgram("file");
    fileproc.setArguments(QStringList(source));
    fileproc.start();
    fileproc.waitForFinished(1000);
    QString w = fileproc.readAllStandardOutput();
    w=w.trimmed();
  ui->lbType->setText("Type"+w.mid(w.indexOf(":"),w.length()));
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
    QString command=ui->cbPrograms->currentText();
    //Check validity of combo box
    if (command=="")
        return;

    if (command.contains("%f"))
        command=command.replace("%f",this->path);
    else
        command=command+" \""+this->path+"\"";

    //Fire the process up.
    QProcess proc;
    proc.startDetached(command);

    int q=-1;
    for (int i=0;i<this->programs.count();i++)
    {
        if (this->programs.at(i)==ui->cbPrograms->currentText())
        {
            q=i;
        }
    }
    if (q>-1)
        this->programs.removeAt(q);
    else
        this->programs.removeLast();

    this->programs.push_front(ui->cbPrograms->currentText());
    ui->cbPrograms->clear();
    for (int i=0;i<10;i++)
    {
        if (this->programs.at(i)!="")
        {
            ui->cbPrograms->addItem(this->programs.at(i));
        }
    }

    this->saveSettings();
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
    settings.setValue("W",this->width());
    settings.setValue("H",this->height()); //remember width and height of viewer.
    for (int i=0;i<10;i++) //save last used programs
    {
       if ((this->programs.count()<i)||(this->programs.count()==0))
       {
           break; //sanity check
       }
       settings.setValue("Recent"+QString::number(i),this->programs.at(i));
    }

    settings.endGroup();
}
