//Open source, GPL, by MCbx 2016

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
#include <QFontDatabase>
#include <QInputDialog>
#include "errordialog.h"
#include <QFileInfo>

//////// MEMENTO ////////
//      TODO LIST      //
// New image in different formats
// Drag-drop
// Boot sector preferences
// Save preferences, window sizes etc.
// Command-line parameters
//Move in image
//Copy in image
//attributes
//Mess with metadata!

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //VERIFY EXISTENCE OF MTOOLS!
    this->process=new QProcess(this);   //TO BE PORTED win
  //  this->process=new QProcess(this);
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
    leLabel->setMaxLength(11);
    leLabel->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    leLabel->setEnabled(0);
    connect(leLabel,SIGNAL(editingFinished()),this,SLOT(on_label_edit()));

    this->img=NULL;
    //START application
}


MainWindow::~MainWindow()
{
    delete ui;
}

//shows asterisk if file is modified
void MainWindow::visualizeModified()
{
    QString wt=this->windowTitle();
    if (this->img->getModified())
    {
        if (wt.at(wt.length()-1)!='*')
        {
            this->setWindowTitle(this->windowTitle()+" *");
            ui->actionSave->setEnabled(1);
        }
    }
    else
    {
        if (wt.at(wt.length()-1)=='*')
        {
            this->setWindowTitle(this->windowTitle().left(this->windowTitle().length()-1));
            ui->actionSave->setEnabled(0);
        }

    }
}

//edit label
void MainWindow::on_label_edit()
{
    this->leLabel->setText(this->leLabel->text().toUpper());
    if (this->img->getLabel()!=this->leLabel->text())
    {
        this->img->setLabel(this->leLabel->text());
        visualizeModified();
    }
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
    if ((this->img!=NULL)&&(this->img->getModified()))
    {
        //REMEMBER ABOUT SAVING QUESTION!

    }
    this->close();
}

void MainWindow::on_actionOpen_triggered()
{
    if ((this->img!=NULL)&&(this->img->getModified()))
    {
        //REMEMBER ABOUT SAVING QUESTION!

    }
    QString fname = QFileDialog::getOpenFileName(this,"Open Image","","Disk Images (*.ima *.dsk *.img);;All files (*)");
    if (fname!="")
    {
        ui->statusBar->showMessage("Loading file ...");
        QApplication::processEvents();
        this->loadFile(fname);
        //we don't need to refresh status bar as it has been done with visualization
    }
}

//load file
int MainWindow::loadFile(QString fileName)
{
    this->currentFile=fileName;
    this->currentDir="::/";
    //Mantle interface
    this->img = new ImageFile(fileName);
    this->dirs=this->img->getContents(currentDir);
    //Visualize directories
    this->visualize();
    this->visualizeModified();
    ui->twDirTree->setCurrentItem(ui->twDirTree->topLevelItem(0));  //select first item. Do not remove this.
    this->leLabel->setEnabled(1);
    ui->actionSave_As->setEnabled(1);
    ui->actionCreate_Directory->setEnabled(1);
    ui->actionVolume_Serial->setEnabled(1);
    ui->actionAdd->setEnabled(1);
    ui->actionAdd_Directories->setEnabled(1);   //TODO: Create "ArmUI" and "DisarmUI" functions with this stuff
    return 0;
}

//visualize trees
void MainWindow::visualize()
{
    //TODO: Save position.
    QString currentDir="";
    QTreeWidgetItem * prev=NULL;
    if (ui->twDirTree->selectedItems().count()>0)
    {
        currentDir=this->leAddress->text();
    }

    ui->twDirTree->clear();
    ui->twFileTree->clear();

    QList<QTreeWidgetItem*> foldery;
    QStringList added;

    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->twDirTree);
    treeItem->setText(0,"::/");
    treeItem->setText(1,"::/");
    treeItem->setIcon(0,QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon));
    //traverse thru the list and find dirs. These dirs will be added
    foldery.append(treeItem);
    added.append("::/");

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
    qSort(names.begin(),names.end());


    for (int i=0;i<names.count();i++)   //create tree from folders
    {
        if (names[i]=="")
            continue;
        QStringList folders=names[i].split('/');
        QString pth="::/";
        for (int j=1;j<folders.count();j++)
        {
            QString pth1=pth+folders.at(j)+"/";
            QTreeWidgetItem * sub = new QTreeWidgetItem();
            sub->setText(0,folders.at(j));
            sub->setIcon(0,QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon));
            if (added.indexOf(pth1)>-1) //if we have such animal, skip it,
            {
                pth=pth1;               //remember only position
                continue;
            }
            added.append(pth1);         //WE WILL ADD. Accoun it in model - name
            foldery.append(sub);        //Account in model - item
            //determine where to connect sub
            int q=added.indexOf(pth);
            foldery.at(q)->addChild(sub);
            pth=pth1;
        }
    }
    ui->twDirTree->expandItem(treeItem);
    this->leLabel->setText(this->img->getLabel());

    //TODO: Restore selected things as were.
    if (prev!=NULL)
    {
        ui->twDirTree->setCurrentItem(prev);
    }

    this->statusBarNormal();
    this->visualizeModified();
}


//makes number in decimal byte form
QString decNumber(int source)
{
 QString a=QString::number(source);
 QString b="";
 for (int i=a.length()-1; i>=0; i--)
 {
     if (b.length()%4==0)
         b=" "+b;
     b=a.at(i)+b;

 }
 return b.trimmed();
}

//change directory
void MainWindow::on_twDirTree_currentItemChanged(QTreeWidgetItem *current)
{
    if (ui->twDirTree->topLevelItemCount()==0)
    {
        return;
    }
    if (current==0)
    {
        return;
    }
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
            entry->setText(1,decNumber(dirs[i].size));
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
    this->statusBarNormal();
    return;
}

//doubleckick item in file list
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

//Displays normal designation on status bar
//Normal designation shows items/selected items count, bytes and free space.
//Contrary to "working designation" when computer is doing sth.
void MainWindow::statusBarNormal()
{
   QString sb="";
   unsigned int si=0;

   //if items selected
   if (ui->twFileTree->selectedItems().count()>0)
   {
        int i;
        ui->actionExtract_selected->setEnabled(1);
        ui->actionDelete_selected->setEnabled(1);
        for (i=0;i<ui->twFileTree->selectedItems().count();i++)
        {
            si+=ui->twFileTree->selectedItems().at(i)->text(1).toInt();
        }
        sb+="Selected "+QString::number(i)+" item/s, occupying "+decNumber(si)+"B";
   }
   else
   {
       QTreeWidgetItemIterator it(ui->twFileTree);
       while (*it)
       {
        si+=(*it)->text(1).toInt();
        ++it;
       }
       sb+=QString::number(ui->twFileTree->topLevelItemCount())+" item/s, occupying "+decNumber(si)+"B";
   }
   sb+=" ("+decNumber(this->img->getFreeSpace())+"B free)";

   if (ui->twFileTree->selectedItems().count()==1)
   {
       ui->actionRename->setEnabled(1);
       ui->actionExtract_selected->setEnabled(1);
   }
   if (ui->twFileTree->selectedItems().count()==0)
   {
       ui->actionRename->setEnabled(0);
       ui->actionExtract_selected->setEnabled(0);
       ui->actionDelete_selected->setEnabled(0);
   }
   ui->statusBar->showMessage(sb);
}

//if we changed selection, refresh status bar.
//This poses an intentional "breaker" for some processes.
void MainWindow::on_twFileTree_itemSelectionChanged()
{
    this->statusBarNormal();
}

//Save file "as" is always accessible. Then the working file
//is changed to saved file.
void MainWindow::on_actionSave_As_triggered()
{
    QString fname = QFileDialog::getSaveFileName(this,"Save Image as","","Disk Images (*.ima *.dsk *.img);;All files (*)");
    if ((!fname.endsWith("img",Qt::CaseInsensitive))&&(!fname.endsWith("ima",Qt::CaseInsensitive))
            &&(!fname.endsWith("dsk",Qt::CaseInsensitive))&&(!fname.endsWith("raw",Qt::CaseInsensitive)))
        fname=fname+".img";

    if (fname!="")
    {
        this->img->saveFile(fname);
    }
    visualizeModified();
}

//Just save. Img will copy all things needed.
void MainWindow::on_actionSave_triggered()
{
    this->img->saveFile("");
    visualizeModified();
}

//rename
void MainWindow::on_actionRename_triggered()
{
    //get path of source
    QString fileName=ui->twFileTree->selectedItems().at(0)->text(0);
    QString source=this->leAddress->text()+fileName;
    bool dialogResult;
    QInputDialog *renameDialog = new QInputDialog();

    //show user some fancy thing
    QString destination = renameDialog->getText(this, "Rename", "New name:", QLineEdit::Normal,
                                           fileName, &dialogResult);
    if ((destination.length()==0)||(!dialogResult)||(destination==fileName))
    {
        this->statusBarNormal();
        return;
    }
    QList<QTreeWidgetItem *> existent= ui->twFileTree->findItems(destination,Qt::MatchFixedString);
    if (existent.count()!=0)
    {
        QMessageBox::critical(this,"Error","Failed to proceed - such item exists.");
        this->statusBarNormal();
        return;
    }

    destination=this->leAddress->text()+destination;
    //rename
    this->img->moveFile(source,destination);

    //refresh
    this->dirs=this->img->getContents(currentDir);
    //Visualize directories
    this->visualize();
    return;
}

//Create directory in current dir
void MainWindow::on_actionCreate_Directory_triggered()
{
        bool dialogResult;
        QInputDialog *mkdirDialog = new QInputDialog();

        //show user some fancy thing
        QString destination = mkdirDialog->getText(this, "Create directory", "Folder name:", QLineEdit::Normal,
                                               "", &dialogResult);
        if ((destination.length()==0)||(!dialogResult))
        {
            this->statusBarNormal();
            return;
        }
        QList<QTreeWidgetItem *> existent= ui->twFileTree->findItems(destination,Qt::MatchFixedString);
        if (existent.count()!=0)
        {
            QMessageBox::critical(this,"Error","Failed to proceed - such item exists.");
            this->statusBarNormal();
            return;
        }

        destination=this->leAddress->text()+destination;
        this->img->makeFolder(destination);

        //refresh
        this->dirs=this->img->getContents(currentDir);
        //Visualize directories
        this->visualize();
        return;
}

//Modify serial number
void MainWindow::on_actionVolume_Serial_triggered()
{
    bool dialogResult;
    QInputDialog *serialDialog = new QInputDialog();

    //show user some fancy thing
    QString g=img->getSerial();
    if (!g.contains("-"))
    {
        g=g.left(4)+"-"+g.right(4);
    }
    QString destination = serialDialog->getText(this, "Volume serial", "Volume serial:", QLineEdit::Normal,
                                           g, &dialogResult);
    if ((destination.length()==0)||(!dialogResult)||(destination==img->getSerial()))
    {
        this->statusBarNormal();
        return;
    }
    destination=destination.toUpper();
    destination=destination.trimmed().replace("-","");
    bool isHex = false;
    destination.toInt(&isHex, 16);
    if ((destination.length()!=8)||(!isHex))
    {
        QMessageBox::critical(this,"Error","Failed to proceed - serial should be 8-digit hex number.");
        this->statusBarNormal();
        return;
    }

    //set
    this->img->setSerial(destination);
    this->visualizeModified();
    ui->statusBar->showMessage("Serial number has been changed");
    return;
}

//Copy selected files into filesystem
void MainWindow::on_actionExtract_selected_triggered()
{
    //propose user destination directory
    QString dir = QFileDialog::getExistingDirectory(this, "Destination Directory", "", QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    if (dir =="")
    {
        return;
    }

    //get paths of selected things and extract them
    for (int i=0;i<ui->twFileTree->selectedItems().count();i++)
    {
         QString fileName=ui->twFileTree->selectedItems().at(i)->text(0);
         fileName=this->leAddress->text()+fileName;
         ui->statusBar->showMessage("Extracting "+ui->twFileTree->selectedItems().at(i)->text(0)+" ("+QString::number(i)+"/"+QString::number(ui->twFileTree->selectedItems().count())+")");
         QApplication::processEvents();
         this->img->copyFile(fileName,dir);
    }


    //refresh
    this->dirs=this->img->getContents(currentDir);
    //Visualize directories
    this->visualize();
    this->statusBarNormal();
    return;
}

//Delete selected items.
void MainWindow::on_actionDelete_selected_triggered()
{
    QMessageBox::StandardButton reply;
     reply = QMessageBox::question(this, "Delete", "Do you want to delete selected files?",
                                   QMessageBox::Yes|QMessageBox::No);
     if (reply == QMessageBox::Yes)
     {
         for (int i=0;i<ui->twFileTree->selectedItems().count();i++)
         {
              QString fileName=ui->twFileTree->selectedItems().at(i)->text(0);
              fileName=this->leAddress->text()+fileName;
              ui->statusBar->showMessage("Deleting "+ui->twFileTree->selectedItems().at(i)->text(0)+" ("+QString::number(i)+"/"+QString::number(ui->twFileTree->selectedItems().count())+")");
              QApplication::processEvents();
              this->img->deleteFile(fileName);
         }
     }

    //refresh
    this->dirs=this->img->getContents(currentDir);
    //Visualize directories
    this->visualize();
    this->statusBarNormal();
    return;

}

//Copy files from filesystem into image. This must be split between dir/file
//as Qt doesn't support selecting Files/dirs in one window (June 2016).
void MainWindow::on_actionAdd_triggered()
{
    //Open file picker
    QFileDialog fileDlg(this,"Add files","","All files (*)");
    fileDlg.setFileMode(QFileDialog::ExistingFiles);
    if (!fileDlg.exec())
        return;
    uint fsize=0;
    ui->statusBar->showMessage("Estimating size...");
    QApplication::processEvents();
    for (int i=0;i<fileDlg.selectedFiles().count();i++)
    {
        QFileInfo fi(fileDlg.selectedFiles().at(i));
        fsize+=fi.size();
    }
    if (fsize>(uint)this->img->getFreeSpace())
    {
        //Marian, to jebnie!
         QMessageBox::StandardButton reply;
         reply = QMessageBox::question(this, "Add", "Files to add are: "+QString::number(fsize)+" Bytes\nBut free space is: "+QString::number(this->img->getFreeSpace())+" Bytes\n Do you want to continue?",
                                       QMessageBox::Yes|QMessageBox::No);
         if (reply == QMessageBox::No)
         {
             return;
             this->statusBarNormal();
         }
    }

    for (int i=0;i<fileDlg.selectedFiles().count();i++)
    {
            QString fileName=fileDlg.selectedFiles().at(i);
            ui->statusBar->showMessage("Adding "+fileName+" ("+QString::number(i)+"/"+QString::number(fileDlg.selectedFiles().count())+")");
            QApplication::processEvents();
            this->img->copyFile(fileName,this->leAddress->text());
    }

    //refresh
    this->dirs=this->img->getContents(currentDir);
    //Visualize directories
    this->visualize();
    this->statusBarNormal();
    return;
}

//helper to estimate size of directory
quint64 dir_size(const QString & str)
{
    quint64 sizex = 0;
    QFileInfo str_info(str);
    if (str_info.isDir())
    {
        QDir dir(str);
        QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs |  QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
            {
                    sizex += dir_size(fileInfo.absoluteFilePath());
            }
            else
                sizex += fileInfo.size();

        }
    }
    return sizex;
}

//Add directory. This must be split between dir/file
//as Qt doesn't support selecting Files/dirs in one window (June 2016).
void MainWindow::on_actionAdd_Directories_triggered()
{
    //Open file picker
    QFileDialog fileDlg(this,"Add Directory","","All files (*)");
    fileDlg.setFileMode(QFileDialog::Directory);
    fileDlg.setOption(QFileDialog::ShowDirsOnly,false);
    if (!fileDlg.exec())
        return;
    uint fsize=0;
    ui->statusBar->showMessage("Estimating size...");
    QApplication::processEvents();

    fsize=dir_size(fileDlg.selectedFiles().at(0));

    if (fsize>(uint)this->img->getFreeSpace())
    {
        //Marian, to jebnie!
         QMessageBox::StandardButton reply;
         reply = QMessageBox::question(this, "Add", "Files to add are: "+QString::number(fsize)+" Bytes\nBut free space is: "+QString::number(this->img->getFreeSpace())+" Bytes\n Do you want to continue?",
                                       QMessageBox::Yes|QMessageBox::No);
         if (reply == QMessageBox::No)
         {
             return;
             this->statusBarNormal();
         }
    }

            QString fileName=fileDlg.selectedFiles().at(0);
            ui->statusBar->showMessage("Adding "+fileName);
            QApplication::processEvents();
            this->img->copyFile(fileName,this->leAddress->text());

    //refresh
    this->dirs=this->img->getContents(currentDir);
    //Visualize directories
    this->visualize();
    this->statusBarNormal();
    return;
}
