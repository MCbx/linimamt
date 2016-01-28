#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->needsSave=0;
    //VERIFY EXISTENCE OF MTOOLS!
    this->process=new QProcess(this);   //TO BE PORTED
    this->process=new QProcess(this);
    this->process->start("which mtools");
    this->process->waitForFinished();
    if (this->process->exitCode()!=0)
    {
        QMessageBox::critical(this,"Cannot continue","mtools cannot be found");
        ui->menuBar->setEnabled(0);
        ui->mainToolBar->setEnabled(0);
    }
    //READ SETTINGS!

    //PARSE PARAMETERS!!

    //OTHER INITIALIZATION

    //START!
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    if (needsSave)
    {
        //REMEMBER ABOUT SAVING QUESTION!

    }
    this->close();
}

void MainWindow::on_actionOpen_triggered()
{
    if (needsSave)
    {
        //REMEMBER ABOUT SAVING QUESTION!

    }
    QString fname = QFileDialog::getOpenFileName(this,"Open Image","","Disk Images (*.ima *.dsk *.img);;All files (*.*)");
    if (fname!="")  //To be pushed to new function
    {
        this->loadFile(fname);
    }
}

int MainWindow::execute(QString command, QString parameters, QString &result)       //TO BE PORTED
{
    QProcess executing;
    executing.setProcessChannelMode(QProcess::MergedChannels);
    executing.start("mtools -c "+command+" -i "+this->currentFile+" "+parameters);
    executing.waitForFinished();

    QString op(executing.readAllStandardOutput());
   // op=op+"\n\n\n\n"+executing.readAllStandardError();
    result=op;
    return executing.exitCode();
}

int MainWindow::loadFile(QString fileName)
{
    this->currentFile=fileName;
    this->currentDir="::/";
    //Mantle interface
    this->prepareDirDump(currentDir);
    //Get information

    //Parse home directory

    //Visualize directories
}

int MainWindow::errorMessage(QString text, QString console)
{

}

//Prepares directory dump and reads them to internal structure
int MainWindow::prepareDirDump(QString home)
{
    QString op;
    int status=this->execute("mdir","-/ -a "+home,op);
    if (status==0)
    {
        //QMessageBox::critical(this,"result",op);
        //Parse results
        QStringList lines = op.split('\n');

        //volume label and serial
        QString tmp=lines[0];
        this->label=tmp.mid(tmp.indexOf("is")+2);
        tmp=lines[1];
        this->serial=tmp.mid(tmp.indexOf("is")+2);
        QMessageBox::critical(this,"result",this->label+"\n"+this->serial);

        int lineCount=2;
        QString myHome;
        this->dirs.clear();
        while (lineCount<lines.count())
        {
            if ((lines[lineCount].length()==0)||(lines[lineCount].indexOf("files       ")>0)||
                    (lines[lineCount].indexOf(".            <DIR>")>=0)||
                       (lines[lineCount].indexOf("..           <DIR>")>=0))
            {
                lineCount++;
                continue;
            }
            if (lines[lineCount].indexOf("Total files listed:")>=0)
            {
                break;
            }
            if (lines[lineCount].indexOf("Directory for")>=0)
            {
                myHome=lines[lineCount].mid(lines[lineCount].indexOf("Directory for")+14);
                if (myHome.at(myHome.length()-1)!='/')
                {
                    myHome+='/';
                }
            }
            else
            {
                fileEntry plik;
                plik.dir=0;
                QString l=lines[lineCount];
                plik.name=myHome+l.mid(0,8).trimmed();
                if (l.mid(9,4).trimmed().length()>0)
                {
                    plik.name+="."+l.mid(9,4).trimmed();
                }
                if (l.mid(13,5)=="<DIR>")
                {
                    plik.size=0;
                    plik.dir=1;
                }
                else
                {
                    plik.size=l.mid(13,9).trimmed().toInt();
                }
                plik.date=l.mid(23,17);
                if (l.length()>41)  //use lfn
                {
                    plik.name=myHome+l.mid(42);
                }

                this->dirs.append(plik);
            }
            lineCount++;
        }

    }
//    QString m="";
//    for (int i=0;i<this->dirs.count();i++)
//    {
//        m=m+dirs[i].name+"  "+QString::number(dirs[i].size)+"  "+dirs[i].dir+"  "+dirs[i].date+"\n";
//    }
//    QMessageBox::critical(this,"aaa",m);
    return status;
}
