#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QStringList>
#include <QLineEdit>
#include <QWidgetAction>
#include <QProgressBar>
#include <QToolBar>


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

    //MANTLE UI
    ui->twFileTree->headerItem()->setText(0,"Name");
    ui->twFileTree->headerItem()->setText(1,"Size");
    ui->twFileTree->headerItem()->setText(2,"Attribute");
    ui->twFileTree->headerItem()->setText(3,"Date");

    ui->twFileTree->header()->setSortIndicatorShown(1);
    ui->twFileTree->header()->setSectionsClickable(1);
    connect(ui->twFileTree->header(),SIGNAL(sectionClicked(int)),this,SLOT(customSortByColumn(int)));
    customSortByColumn(ui->twFileTree->header()->sortIndicatorSection());
    //addres bar
    leAddress = new QLineEdit(ui->tbAddressBar);
    ui->tbAddressBar->addWidget(leAddress);
    leAddress->setReadOnly(1);

    //label editor
    leLabel = new QLineEdit("",ui->menuBar);
    ui->menuBar->setCornerWidget(leLabel);
    connect(leLabel,SIGNAL(editingFinished()),this,SLOT(on_label_edit()));


    //START!
}


MainWindow::~MainWindow()
{
    delete ui;
}

//edit label
void MainWindow::on_label_edit()
{
    this->label=this->leLabel->text();
}

//this thing sorts the doirectories always upwards.
void MainWindow::customSortByColumn(int column)
{
    Qt::SortOrder order=ui->twFileTree->header()->sortIndicatorOrder();
    ui->twFileTree->sortItems(column,order);

    //take directories and push them to the beginning
    for (int i=0;i<ui->twFileTree->topLevelItemCount();i++)
    {
        if (ui->twFileTree->topLevelItem(i)->text(2).at(0)=='D')
        {
            QTreeWidgetItem * item = ui->twFileTree->topLevelItem(i);
            ui->twFileTree->takeTopLevelItem(i);
            ui->twFileTree->insertTopLevelItem(0,item);
        }
    }

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
    if (needsSave)  //to another function
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
    executing.start("mtools -c "+command+" -i \""+this->currentFile+"\" "+parameters);
    executing.waitForFinished();

    QString op(executing.readAllStandardOutput());
   // op=op+"\n\n\n\n"+executing.readAllStandardError();
    result=op;
    return executing.exitCode();
}

//load file
int MainWindow::loadFile(QString fileName)
{
    this->currentFile=fileName;
    this->currentDir="::/";
    //Mantle interface
    this->prepareDirDump(currentDir);
    //Visualize directories
    this->visualize();
    return 0;
}

//throw error message
int MainWindow::errorMessage(QString text, QString console)
{

}

//Prepares directory dump and reads them to internal structure
int MainWindow::prepareDirDump(QString home)
{
    QString op;
    int status=this->execute("mdir","-/ -a \""+home+"\"",op);
    if (status==0)
    {
        QString qsattrs;
        status=this->execute("mattrib","-/ -X \""+home+"\"",qsattrs);
        if (status!=0)
        {
            //Throw error
            //Fuck it
        }
        QStringList attrs = qsattrs.split('\n');

        //QMessageBox::critical(this,"result",op);
        //Parse results
        QStringList lines = op.split('\n');

        //volume label and serial
        QString tmp=lines[0];
        this->label=tmp.mid(tmp.indexOf("is")+2);
        tmp=lines[1];
        this->serial=tmp.mid(tmp.indexOf("is")+2);
        //QMessageBox::critical(this,"result",this->label+"\n"+this->serial);

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
                plik.attrib="-";
                QString l=lines[lineCount];
                plik.name=myHome+l.mid(0,8).trimmed();
                if (l.mid(9,4).trimmed().length()>0)
                {
                    plik.name+="."+l.mid(9,4).trimmed();
                }
                if (l.mid(13,5)=="<DIR>")
                {
                    plik.size=0;
                    plik.attrib="D";
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
                //This is a VERY BAD routine for attribute getting.
                //It should be corrected as it's slow.
                for (int i=0;i<attrs.count();i++)
                {
                    if (attrs[i].endsWith(plik.name,Qt::CaseInsensitive))
                    {
                        //Parse attribute string
                        //append attributes like:
                        //drahs or -----
                        QString attrString=attrs[i].split(':')[0].trimmed();
                        if (attrString.contains('A'))
                            plik.attrib+="a";
                        else
                            plik.attrib+="-";
                        if (attrString.contains('R'))
                            plik.attrib+="r";
                        else
                            plik.attrib+="-";
                        if (attrString.contains('H'))
                            plik.attrib+="h";
                        else
                            plik.attrib+="-";
                        if (attrString.contains('S'))
                            plik.attrib+="s";
                        else
                            plik.attrib+="-";
                       // plik.attrib.endsWith("aaa",Qt::CaseInsensitive)
                    }
                }

                this->dirs.append(plik);
            }
            lineCount++;
        }


    }

//    QString m="";
//    for (int i=0;i<this->dirs.count();i++)
//    {
//        m=m+dirs[i].name+"  "+QString::number(dirs[i].size)+"  "+dirs[i].attrib+"  "+dirs[i].date+"\n";
//    }
//    QMessageBox::critical(this,"aaa",m);
    return status;
}

//visualize tree
void MainWindow::visualize()
{
    //TODO: Save position.

    ui->twDirTree->clear();
    ui->twFileTree->clear();

    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->twDirTree);
    treeItem->setText(0,"::/");
    treeItem->setText(1,"::/");
    treeItem->setIcon(0,QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon));
    //traverse thru the list and find dirs. These dirs will be added
    QTreeWidgetItem * former = treeItem;
    QStringList names;
    for (int i=0;i<this->dirs.count();i++)
    {
        if (this->dirs[i].attrib.at(0)=='D')
        {
            names.append(dirs[i].name);
        }
    }
    qSort(names.begin(),names.end());

    for (int i=1;i<names.count();i++)
    {

        if (names.at(i).contains(names.at(i-1)))
            names[i-1]="";
    }

    for (int i=0;i<names.count();i++)
    {
        former=treeItem;
        QStringList folders=names[i].split('/');
        for (int j=1;j<folders.count();j++)
        {
            QTreeWidgetItem * sub = new QTreeWidgetItem();
            sub->setText(0,folders.at(j));
          //  sub->setText(1,names[i]);
            sub->setIcon(0,QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon));
            if (ui->twDirTree->findItems(names[i],Qt::MatchContains,1).count()==0)
            {
                former->addChild(sub);
                former=sub;
            }
        }
    }
    ui->twDirTree->expandItem(treeItem);
    this->leLabel->setText(this->label);
    //TODO: Restore selected things as were.
}

void MainWindow::on_twDirTree_currentItemChanged(QTreeWidgetItem *current)
{
    ui->twFileTree->clear();
    //determine path
    QString path;
    QTreeWidgetItem *iterate=current;
    while(iterate->parent()!=NULL)
    {
        path=iterate->text(0)+"/"+path;
        iterate=iterate->parent();
    }
    path="::/"+path;
    this->leAddress->setText(path);
    //QMessageBox::critical(this,"sss",path);

    //visualize files
    for (int i=0;i<dirs.count();i++)
    {
        if ((dirs[i].name.startsWith(path))&&
                (!dirs[i].name.mid(path.length()).contains("/")))   //when we have file not in subdir
        {
            QTreeWidgetItem * entry = new QTreeWidgetItem(ui->twFileTree);
            entry->setText(0,dirs[i].name.split('/').last());
            entry->setText(1,QString::number(dirs[i].size));
            entry->setText(2,dirs[i].attrib);
            entry->setText(3,dirs[i].date);
            entry->setText(4,dirs[i].name);
            if (dirs[i].attrib.at(0)=='D')
                entry->setIcon(0,QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon));
            else
                entry->setIcon(0,QApplication::style()->standardIcon(QStyle::SP_FileIcon));

        }
    }
    customSortByColumn(ui->twFileTree->header()->sortIndicatorSection());
    return;
}

void MainWindow::on_twFileTree_itemDoubleClicked(QTreeWidgetItem *item)
{
    if (item->text(2).at(0)=='D')
    {
        //go to tree position
        ui->twDirTree->expandItem(ui->twDirTree->topLevelItem(0));
        QTreeWidgetItem * myItem=ui->twDirTree->topLevelItem(0);
        QStringList parts=item->text(4).split('/');
        for (int i=1;i<parts.count();i++)
        {
            for (int j=0;j<myItem->childCount();j++)
            {
                if (myItem->child(j)->text(0)==parts[i])
                {
                    ui->twDirTree->expandItem(myItem->child(j));
                    myItem=myItem->child(j);
                    break;
                }
            }
        }
        //click it.
        ui->twDirTree->clearSelection();
        myItem->setSelected(1);
        ui->twDirTree->setCurrentItem(myItem);
        this->on_twDirTree_currentItemChanged(myItem);

    }

        //TODO: File double-click
}

//Up button
void MainWindow::on_actionGoUp_triggered()
{
    if (this->leAddress->text()=="::/")
    {
        return;
    }
    ui->twDirTree->setCurrentItem(ui->twDirTree->currentItem()->parent());
}

//toggle address bar visibility
void MainWindow::on_actionAddress_bar_triggered()
{
    if (ui->actionAddress_bar->isChecked())
    {
        ui->tbAddressBar->setVisible(1);
    }
    else
    {
        ui->tbAddressBar->setVisible(0);
    }
}
