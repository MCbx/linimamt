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
#include <QSettings>
#include <QTemporaryDir>
#include <QDesktopServices>
#include "attribute.h"
#include "newimage.h"
#include "bootsector.h"
#include "harddiskopen.h"
#include "fileviewer.h"
#include "optionsdialog.h"

//////// MEMENTO ////////
//      TODO LIST      //
// Drag-drop:
//   - implementation of drop-extracting
// Mess with metadata!

//these need mounting to letters
//Convert image between formats by mounting two images and moving files between
// - derivative - defrag image - by converting format on itself

//to open many images at once:
// - configuration file:
// drive 1: file="/path/to/file.img" offset=...
// drive 2: file="/path/to/file2.img" offset=...
// or other offsets.
// partition is for primary ones. offset allows to read logical drives.
// Config file should be pointed by $MTOOLSRC variable.
// e.g. execute this: env MTOOLSRC="/tmp/mtoolsrc" mdir 1:/
// will give hd listing.
// Make unique config file name as user may launch many instances.
//Because of a bug/feature in MTools, the drive letter can be 1: 2: etc., what allows us
//to bypass Windows drive letters. This way we can use multi-image approach to copy files
//without touching the hard disk buffer: mcopy -p -m 2:/command.com 3:/
//better implement it in separate module to throw things between images.

MainWindow::MainWindow(QStringList arguments, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->twFileTree,SIGNAL(sigDragDrop(QStringList,QString)),this,SLOT(on_fileDragDrop(QStringList,QString)));
    connect(ui->twDirTree,SIGNAL(sigDragDrop(QStringList,QString)),this,SLOT(on_fileDragDrop(QStringList,QString)));

    //VERIFY EXISTENCE OF MTOOLS!
    this->process=new QProcess(this);   //TO BE PORTED win
    this->process->start("which mtools");
    this->process->waitForFinished();
    if (this->process->exitCode()!=0)
    {
        QMessageBox::critical(this,"Cannot continue","mtools cannot be found");
        ui->menuBar->setEnabled(0);
        ui->mainToolBar->setEnabled(0);
    }

    //VERIFY EXISTENCE OF TESTDISK
    this->process=new QProcess(this);   //TO BE PORTED win
    this->process->start("which testdisk");
    this->process->waitForFinished();
    if (this->process->exitCode()!=0)
    {
        QMessageBox::critical(this,"Cannot continue","testdisk cannot be found");
        ui->menuBar->setEnabled(0);
        ui->mainToolBar->setEnabled(0);
    }


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
    leLabel->setReadOnly(1);
   // connect(leLabel,SIGNAL(editingFinished()),this,SLOT(on_label_edit()));

    ui->twDirTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    //READ SETTINGS
    this->hdImgSize=3145728;
    this->loadSettings();

    this->img=NULL;
    setAcceptDrops(true);

    ui->twFileTree->setDragDropMode(QTreeWidget::NoDragDrop);
    ui->twDirTree->setDragDropMode(QTreeWidget::NoDragDrop);

    //PARSE PARAMETERS
    if (arguments.count()>=2)
    {
        if (arguments[1]=="-new")
        {
            this->on_actionNew_triggered();
        }

        if (QFile::exists(arguments[1]))
        {
            ImageFile::HandleMode mode=ImageFile::DefaultMode;
            if (arguments.count()>=3)
            {
                if (arguments[2]=="-r")
                    mode=ImageFile::ReadOnly;
                if (arguments[2]=="-d")
                    mode=ImageFile::DirectMode;
            }

            ui->statusBar->showMessage("Loading file ...");
            QApplication::processEvents();
            this->loadFile(arguments[1],mode);
        } else
        {
            QMessageBox::information(0,"Problem","Parameter unknown and file does not exist: "+arguments[1]);
        }
    }

    //START application
}

//pass input data
void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

//file open when dropped on window outside of panels
void MainWindow::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString fileName = url.toLocalFile();

        if ((this->img!=NULL)&&(this->img->getModified()))
        {
            int k=askForSave();
            if (k==-1)
            {
                return;
            }
            if (k==1)
            {
                ui->actionSave->trigger();
            }
        }
        ui->statusBar->showMessage("Loading file ...");
        QApplication::processEvents();
        this->loadFile(fileName, ImageFile::DefaultMode);
        //we don't need to refresh status bar as it has been done with visualization
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


////////////////////////////
///   GENERAL ROUTINES   ///
////////////////////////////
#define FOLDINGSTART {
//shows asterisk if file is modified
void MainWindow::visualizeModified()
{
    //show window title

    QString wt=this->windowTitle();
    if (this->img==NULL)
    {
        setWindowTitle("LinimaMT");
        return;
    }
    else
    {
        this->img->finishProcedure();
        if (this->currentFile=="")
        {
            this->setWindowTitle("LinimaMT - [untitled]");
        }
        else
        {
            this->setWindowTitle("LinimaMT - ["+this->currentFile+"]");
        }
    }
    if (wt.at(wt.length()-1)=='*')
    {
        this->setWindowTitle(this->windowTitle()+" *");
    }

    if(this->img->getHandleMode()==ImageFile::ReadOnly)
    {
        this->setWindowTitle(this->windowTitle()+" [READ ONLY]");
        return;
    }

    if(this->img->getHandleMode()==ImageFile::DirectMode)
    {
        this->setWindowTitle(this->windowTitle()+" [DIRECT MODE]");
        return;
    }

    wt=this->windowTitle();
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

//Display question about saving
int MainWindow::askForSave()
{
    //if we are in direct mode, this has no meaning, even if the file is modified
    if (this->img->getHandleMode()==ImageFile::DirectMode)
    {
        return 0;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "File modified","Do you want to save before closing the file?",QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
    if (reply==QMessageBox::Yes)
        return 1;
    if (reply==QMessageBox::No)
        return 0;
    if (reply==QMessageBox::Cancel)
        return -1;
    return 2;
}

//Load initial settings
void MainWindow::loadSettings()
{
    QString settingsPath = "imarc.ini";
    #ifndef Q_OS_WIN32
        settingsPath = QDir::homePath()+"/.imarc.ini";
    #endif

    QSettings settings(settingsPath,QSettings::IniFormat);
    settings.beginGroup("GUI");
    int winX=MainWindow::width();
    int winY=MainWindow::height();
    winX=settings.value("Width",QString::number(winX)).toInt();
    winY=settings.value("Height",QString::number(winY)).toInt();
    MainWindow::resize(winX,winY);
    winX=20;
    winX=settings.value("Split",winX).toInt();
    winY=20;
    winY=settings.value("Split2",winY).toInt();
    QList<int> a;
    a.append(winX);
    a.append(winY);
    ui->splitter->setSizes(a);

    winX=ui->twFileTree->columnWidth(0);
    winX=settings.value("Col0",QString::number(winX)).toInt();
    ui->twFileTree->setColumnWidth(0,winX);
    winX=ui->twFileTree->columnWidth(1);
    winX=settings.value("Col1",QString::number(winX)).toInt();
    ui->twFileTree->setColumnWidth(1,winX);
    winX=ui->twFileTree->columnWidth(2);
    winX=settings.value("Col2",QString::number(winX)).toInt();
    ui->twFileTree->setColumnWidth(2,winX);
    winX=ui->twFileTree->columnWidth(3);
    winX=settings.value("Col3",QString::number(winX)).toInt();
    ui->twFileTree->setColumnWidth(3,winX);

    ui->tbAddressBar->setVisible(settings.value("AddressBar",true).toBool());
    ui->actionAddress_bar->setChecked(settings.value("AddressBar",true).toBool());
    settings.endGroup();
    settings.beginGroup("Advanced");
    this->hdImgSize=settings.value("HDDImgMinimumSize",this->hdImgSize).toInt();
    this->FileOpenPath=settings.value("FileOpenPath","").toString();
    settings.endGroup();

    settings.beginGroup("Viewing");
    this->defaultViewer=settings.value("DefaultViewer","0").toString();
    int k=settings.value("programsCount",0).toInt();
    this->viewers.clear();
    this->extensions.clear();
    for (int i=0;i<k;i++)
    {
        QString a=settings.value("Extension"+QString::number(i),"").toString();
        this->extensions.append(a);
        a=settings.value("Program"+QString::number(i),"").toString();
        this->viewers.append(a);
    }
    settings.endGroup();

}

//Save settings
void MainWindow::saveSettings()
{
    QString settingsPath = "imarc.ini";
    #ifndef Q_OS_WIN32
    settingsPath = QDir::homePath()+"/.imarc.ini";
    #endif

    QSettings settings(settingsPath,QSettings::IniFormat);
    settings.beginGroup("GUI");
    settings.setValue("Width",MainWindow::width());
    settings.setValue("Height",MainWindow::height());
    QList<int> a=ui->splitter->sizes();
    settings.setValue("Split",MainWindow::width()-ui->splitter->sizes().at(1));
    settings.setValue("Split2",MainWindow::width()-ui->splitter->sizes().at(0));
    settings.setValue("Col0",ui->twFileTree->columnWidth(0));
    settings.setValue("Col1",ui->twFileTree->columnWidth(1));
    settings.setValue("Col2",ui->twFileTree->columnWidth(2));
    settings.setValue("Col3",ui->twFileTree->columnWidth(3));
    settings.setValue("AddressBar",ui->tbAddressBar->isVisible());
    settings.endGroup();

    settings.beginGroup("Advanced");
    settings.setValue("HDDImgMinimumSize",this->hdImgSize);
    settings.setValue("FileOpenPath",this->FileOpenPath);
    settings.endGroup();

    settings.remove("Viewing"); //because user may delete sth, remove a whole group before writing it back

    settings.beginGroup("Viewing");
    settings.setValue("DefaultViewer",this->defaultViewer);
    if (this->extensions.count()!=this->viewers.count())
    {
        settings.endGroup();       //error condition
        return;
    }
    settings.setValue("programsCount",this->extensions.count());
    for (int i=0;i<this->extensions.count();i++)
    {
       settings.setValue("Extension"+QString::number(i),this->extensions.at(i));
       settings.setValue("Program"+QString::number(i),this->viewers.at(i));
    }
    settings.endGroup();
}

//makes number in decimal byte form e.g. 1 457 664 instead of 1457664
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
        ui->actionRename->setEnabled(0);
        ui->actionQuick_Preview->setEnabled(0);
        ui->actionExtract_selected->setEnabled(1);
        ui->actionDelete_selected->setEnabled(1);
        ui->actionAttributes->setEnabled(1);
        for (i=0;i<ui->twFileTree->selectedItems().count();i++)
        { //the thing herer is to get number from "xx xxx" number string.
            si+=ui->twFileTree->selectedItems().at(i)->text(1).replace(" ","").toInt();
        }
        sb+="Selected "+QString::number(i)+" item/s, occupying "+decNumber(si)+"B";
   }
   else
   {
       QTreeWidgetItemIterator it(ui->twFileTree);
       while (*it)
       {
        si+=(*it)->text(1).replace(" ","").toInt();
        ++it;
       }
       sb+=QString::number(ui->twFileTree->topLevelItemCount())+" item/s, occupying "+decNumber(si)+"B";
   }
   sb+=" ("+decNumber(this->img->getFreeSpace())+"B free)";

   if (ui->twFileTree->selectedItems().count()==1)
   {
       ui->actionRename->setEnabled(1);
       ui->actionQuick_Preview->setEnabled(1);
       ui->actionAttributes->setEnabled(1);
       ui->actionExtract_selected->setEnabled(1);
   }
   if (ui->twFileTree->selectedItems().count()==0)
   {
       ui->actionAttributes->setEnabled(0);
       ui->actionRename->setEnabled(0);
       ui->actionQuick_Preview->setEnabled(0);
       ui->actionExtract_selected->setEnabled(0);
       ui->actionDelete_selected->setEnabled(0);
   }
   ui->statusBar->showMessage(sb);

   if (this->img->getHandleMode()==ImageFile::ReadOnly)
   {
       ui->actionAttributes->setEnabled(0);
       ui->actionRename->setEnabled(0);
       ui->actionQuick_Preview->setEnabled(1);
       ui->actionDelete_selected->setEnabled(0);
   }
}

//on window close
void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::closeEvent(QCloseEvent *clsEv)
{
    for (int i=0;i<this->deletion.count();i++) //temporary files cleanup
    {
        QDir d(this->deletion.at(i));
        d.removeRecursively();
    }
    if ((this->img!=NULL)&&(this->img->getModified()))
    {
        int k=askForSave();
        if (k==-1)
        {
            clsEv->ignore();
            return;
        }
        if (k==1)
        {
            ui->actionSave->trigger();
        }
        clsEv->accept();
    }
    if (this->img) this->img->disposeFile();
    this->saveSettings();
    this->close();
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

//enables/disables UI elements coordinated with image modification.
void MainWindow::enableUI(bool state)
{
    this->leLabel->setEnabled(state);
    ui->actionSave_As->setEnabled(state);
    ui->actionCreate_Directory->setEnabled(state);

    ui->actionVolume_Serial->setEnabled(state);

    if ((state==1)&&(this->img->getSerial()=="INVALID")) //invalid DOS disk, old pre-4.0 disk etc.
    {
            ui->actionVolume_Serial->setEnabled(0);
    }


    ui->actionAdd->setEnabled(state);
    //ui->actionAttributes->setEnabled(state);
    ui->actionAdd_Directories->setEnabled(state);
    ui->actionVolume->setEnabled(state);
    ui->actionWipe_free_space->setEnabled(state);
    ui->actionGoUp->setEnabled(state);
    ui->twDirTree->setAcceptDrops(state);
    ui->twFileTree->setAcceptDrops(state);
    ui->actionRun_TestDisk_on_image->setEnabled(state);

    if (this->img==NULL)
        return;

    if (this->img->getHandleMode()==ImageFile::ReadOnly)
    {
        ui->actionSave->setEnabled(0);
        ui->actionCreate_Directory->setEnabled(0);
        ui->actionAdd->setEnabled(0);
        ui->actionAttributes->setEnabled(0);
        ui->actionAdd_Directories->setEnabled(0);
        ui->actionWipe_free_space->setEnabled(0);
        ui->twDirTree->setAcceptDrops(0);
        ui->twFileTree->setAcceptDrops(0);
        ui->actionDelete_selected->setEnabled(0);
        this->leLabel->setEnabled(0);
    }
    if (this->img->getHandleMode()==ImageFile::DirectMode)
    {
        ui->actionSave->setEnabled(0);
        ui->actionCreate_Directory->setEnabled(0);
        ui->actionAttributes->setEnabled(0);
    }
    ui->actionMBR->setEnabled(0);
    if (this->img->getOffset()>0)   //only for partitions
    {
        ui->actionMBR->setEnabled(state);
    }
}

//helper function to check file existence and ask for overwriting.
char MainWindow::askForReplacement(bool &skipAll, bool &overwriteAll, QString from, QString to)
{
    char replaceMode='0';
  //  if (from.endsWith('/'))
  //  {
   //     from=from.left(from.count()-1);   //we cannot.
  //  }
    QString check=from.split('/').last();
    check=check.toUpper();

    bool duplicate=0; //we have to find it in model
    for (int j=0;j<this->dirs.count();j++)
    {
        if (this->dirs.at(j).name.toUpper()==to.toUpper()+check.toUpper())
        {
            duplicate=1;
            break;
        }
    }
    if(duplicate)
    {
        //such item exists
        QMessageBox::StandardButton reply=QMessageBox::Abort;
        if ((!skipAll)&&(!overwriteAll))
        {
            reply = QMessageBox::question(this, "Item exists",
                                              "Warning: The item "+check+" seems to exist. \n"
                                              "Yes to overwrite, No to skip, Cancel to abandon copying.\nIf It's a directory, all files existing inside will be overwritten or skipped.",
                                              QMessageBox::Yes|QMessageBox::No|QMessageBox::YesToAll|QMessageBox::NoToAll|QMessageBox::Cancel);
        }
        if (reply==QMessageBox::Cancel)
         {
            return '-';
         }
        if ((reply==QMessageBox::Yes)||(overwriteAll))
            replaceMode='o';
        if ((reply==QMessageBox::No)||(skipAll))
            replaceMode='s';
        if (reply==QMessageBox::YesToAll)
        {
            replaceMode='o';
            overwriteAll=1;
        }
        if (reply==QMessageBox::NoToAll)
        {
            replaceMode='s';
            skipAll=1;
        }

    }
    return replaceMode;
}

//Fire the options dialog
void MainWindow::on_actionOptions_triggered()
{
    optionsDialog * n = new optionsDialog(&this->hdImgSize, &this->defaultViewer, &this->extensions, &this->viewers, this);
    n->exec();
    this->saveSettings();
}


#define FOLDINGEND }

/////////////////////////////
///   FILE AND DIR VIEW   ///
/////////////////////////////
#define FOLDINGSTART {
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

//visualize trees
void MainWindow::visualize()
{
    this->img->finishProcedure();
    //Save position.
    QString currentDir="::/"; //if you don't know what to do, use root directory. It always exists
    if (ui->twDirTree->selectedItems().count()>0)
    {
        currentDir=this->leAddress->text();
    }

    ui->twDirTree->clear();
    ui->twFileTree->clear(); //clear views

    QList<QTreeWidgetItem*> foldery; //tree widget items list of directories
    QStringList added;  //this is the list of already added directory items

    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->twDirTree); //prepare root entry
    treeItem->setText(0,"::/");
    treeItem->setText(1,"::/"); //text at 1 is the full path, this is not visible at all but is used in drag-drop
    treeItem->setIcon(0,QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon));
    QTreeWidgetItem * prev=treeItem; //previously selected - if nothing assume root.
    //traverse thru the list and find dirs. These dirs will be added
    foldery.append(treeItem); //add root to list
    added.append("::/"); //add root to paths list. This list saves time when searching for duplicates

    QStringList names; //prepare list of directories in a whole image
    for (int i=0;i<this->dirs.count();i++)
    {
        if (this->dirs[i].attrib.at(0)=='D')
        {
            names.append(dirs[i].name+"/");
        }
    }
    qSort(names.begin(),names.end());

    for (int i=1;i<names.count();i++)
    {

        if (names.at(i).contains(names.at(i-1)))
            names[i-1]="";
    }
    qSort(names.begin(),names.end()); //remove duplicates


    for (int i=0;i<names.count();i++)   //create tree from folders
    {
        if (names[i]=="")
            continue;

        if (names[i].at(names[i].length()-1)=='/')
            names[i]=names.at(i).left(names.at(i).length()-1); //trim slashes

        QStringList folders=names[i].split('/');
        QString pth="::/";
        for (int j=1;j<folders.count();j++)
        {
            QString pth1=pth+folders.at(j)+"/";
            QTreeWidgetItem * sub = new QTreeWidgetItem();
            sub->setText(0,folders.at(j)); //add dir name of item
            sub->setText(1,pth1); //add full path of each item
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
            if (pth==currentDir)  //remember selection
            {
                prev=sub;
            }
        }
    }
    ui->twDirTree->expandItem(treeItem);
    this->leLabel->setText(this->img->getLabel().trimmed());

    //Restore selected things as were.
    if (prev!=NULL)
    {
        ui->twDirTree->setCurrentItem(prev);
    }

    this->statusBarNormal();
    this->visualizeModified();
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
    ui->twFileTree->setCurrentDir(path);
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
            entry->setText(4,dirs[i].name); //this is not visible and is used for drag-drop
            if (dirs[i].attrib.at(0)=='D')
            {
                entry->setIcon(0,QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon));
                entry->setText(4,dirs[i].name+"/"); //this is not visible and is used for drag-drop
            }
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
        return;
    }

    //File double-click
    //1. Extract file
    QString fileName=item->text(4);
    QTemporaryDir dir;
    dir.setAutoRemove(0);
    if (!dir.isValid()) {
       QMessageBox::critical(this,"Problem","There was a problem creating temporaty directory.\nPlease try to extract the file manually and view it.");
       return;
    }
    this->img->copyFile(fileName,dir.path()+"/"+item->text(0));
    this->img->forceModified(0);
    fileName=dir.path()+"/"+item->text(0);

    //scan for extensions
    QString currentExtension=item->text(0).split(".").last();
    currentExtension=currentExtension.toUpper();
    QString command=this->defaultViewer;     //default - default viewer configured
    for (int i=0;i<this->extensions.count();i++) //iterate over extensions
    {
        QString exts=this->extensions.at(i).toUpper();
        QStringList a=exts.split("|");
        for (int j=0;j<a.count();j++)
        {
            if (a.at(j)==currentExtension) //we hit an extension
            {
                if (i>this->viewers.count()-1)
                    break;

                command=this->viewers.at(i);
                break;
            }
        }
        if (command!=this->defaultViewer)
            break;
    }


    if (command==this->defaultViewer)
    {
        if (this->defaultViewer=="0")
        {
            //2. Open the file in default application
            QDesktopServices::openUrl(QUrl(fileName));
            this->deletion.append(dir.path());
            return;
        }
        if (this->defaultViewer=="1")
        {
            //2. Create viewer
            QString settingsPath = "imarc.ini";
            #ifndef Q_OS_WIN32
                settingsPath = QDir::homePath()+"/.imarc.ini";
            #endif

            fileViewer * fv = new fileViewer(fileName,settingsPath,item->text(0));
            //3. Launch viewer
            fv->exec();
            dir.setAutoRemove(1); //we can safely remove directory
            return;
        }
    }

    //we have some viewer configured
    if ((command=="")||(command.length()==1))
        return;

    if (command.contains("%f"))
        command=command.replace("%f",fileName);
    else
        command=command+" \""+fileName+"\"";

    //Fire the process up.
    QProcess proc;
    proc.startDetached(command);

}

//if we changed selection, refresh status bar.
//This poses an intentional "breaker" for some processes.
void MainWindow::on_twFileTree_itemSelectionChanged()
{
    this->statusBarNormal();
}

//this executes if valid content is dropped in valid place.
void MainWindow::on_fileDragDrop(QStringList from, QString to)
{
   // QMessageBox::critical(this,"Drag",from+" into "+to);

    //We have the following situations:
    //1. Inside image - we move files
    //2. anything outside image - we copy files.
    if ((from.at(0).startsWith("file://"))||(to.startsWith("file://")))
    {
        if (from.at(0).startsWith("file://"))
        {
            //we copy from outside to inside
            //1. Check outside's size
            //2. If larger than img->freeSpace, ask user what to do.
            int importedSize=0;
            for (int i=0;i<from.count();i++)    //Calculate added files size
            {
                QString k=from.at(i);
                QFile f(k.replace("file://",""));
                QFileInfo fi(f);
                if ((fi.exists())&&(fi.isFile()))
                {
                    importedSize+=fi.size();
                }
                if ((fi.exists())&&(fi.isDir()))
                {
                    importedSize+=dir_size(k.replace("file://",""));
                }

            }
            if (this->img->getFreeSpace()<importedSize) //Ask user what to do if it will not fit.
            {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, "Free space problem",
                                                      "Warning: There is "+QString::number(this->img->getFreeSpace())+" Bytes free on image. \n"
                                                      "Selected items are larger: "+QString::number(importedSize)+" bytes. Not all files may get imported. Do you want to continue?",
                                                      QMessageBox::Yes|QMessageBox::No);
                 if (reply==QMessageBox::No)
                    return;
            }
        }

        //Then we copy. Let's check for existing file.
        bool skipAll=0;
        bool overwriteAll=0;
        for (int i=0;i<from.count();i++)
        {

            QString src=from.at(i);
            src=src.replace("file://","");

            char replaceMode=this->askForReplacement(skipAll,overwriteAll,src,to);
            if (replaceMode=='-')
                return;

            this->img->copyFile(src,to.replace("file://",""),replaceMode);
        }
    }
    else
    {
        //we move files inside
        //generally there is no problem with size, only files existence.
        bool skipAll=0;
        bool overwriteAll=0;
        for (int i=0;i<from.count();i++)
        {
            QString src=from.at(i);
            src=src.replace("file://","");

            char replaceMode=this->askForReplacement(skipAll,overwriteAll,src,to);
            if (replaceMode=='-')
                return;
            this->img->moveFile(src,to.replace("file://",""),replaceMode);
        }
    }

    //refresh
    this->dirs=this->img->getContents("::/");
    //Visualize directories
    this->visualize();
    this->statusBarNormal();
    return;
}

//File listing context menu
void MainWindow::on_twFileTree_customContextMenuRequested(const QPoint &pos)
{
    //Prepare context menu
    QMenu menu(this);
    menu.addAction(ui->actionExtract_selected);
    menu.addAction(ui->actionAdd);
    menu.addAction(ui->actionAdd_Directories);
    menu.addSeparator();
    menu.addAction(ui->actionQuick_Preview);
    menu.addAction(ui->actionCreate_Directory);
    menu.addAction(ui->actionRename);
    menu.addAction(ui->actionDelete_selected);
    menu.addAction(ui->actionAttributes);

    menu.exec(ui->twFileTree->mapToGlobal(pos));
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
#define FOLDINGEND }

/////////////////////////////
///     IMAGE ACTIONS     ///
/////////////////////////////
#define FOLDINGSTART {
//edit label. This may not be useful at all after making it more difficult to stop altering images.
void MainWindow::on_label_edit()
{
//    this->leLabel->setText(this->leLabel->text().trimmed());
//    if (this->img->getLabel()!=this->leLabel->text())
//    {
//        this->img->setLabel(this->leLabel->text());
//        visualizeModified();
//    }
}

//open file action
void MainWindow::on_actionOpen_triggered(ImageFile::HandleMode mode)
{
    if ((this->img!=NULL)&&(this->img->getModified()))
    {
        int k=askForSave();
        if (k==-1)
        {
            return;
        }
        if (k==1)
        {
            ui->actionSave->trigger();
        }
    }
    QString fname = QFileDialog::getOpenFileName(this,"Open Image",this->FileOpenPath,"Disk Images (*.ima *.dsk *.img);;All files (*)");
    if (fname!="")
    {
        ui->statusBar->showMessage("Loading file ...");
        QApplication::processEvents();
        this->loadFile(fname, mode);
        //we don't need to refresh status bar as it has been done with visualization
    }
}

void MainWindow::on_actionOpen_as_Read_only_triggered()
{
    this->on_actionOpen_triggered(ImageFile::ReadOnly);
}

//load file procedure
int MainWindow::loadFile(QString fileName, ImageFile::HandleMode mode)
{
    this->enableUI(0);
    this->currentFile=fileName;
    //Mantle interface
    if (this->img) this->img->disposeFile();

    //hdd image opening
    if (QFile(fileName).size()>this->hdImgSize)
    {
        HardDiskOpen * hd = new HardDiskOpen(this,fileName);
        hd->exec();
        if(hd->getOffset()==-1)
        {
            ui->twFileTree->clear();
            ui->twDirTree->clear();
            this->leAddress->setText("::/");
            this->leLabel->clear();
            this->setWindowTitle("LinImaMT");
            if (this->img)
                this->img->finishProcedure();
            ui->statusBar->showMessage("");
            return 0;
        }
        this->img = new ImageFile(fileName,hd->getMode(),hd->getOffset()); //open partition
    }
    else //fdd image opening
    {
        this->img = new ImageFile(fileName,mode);
    }
    ui->twFileTree->setImageFile(this->img);
    this->dirs=this->img->getContents("::/");
    if ((this->img->getFreeSpace()==0)&&(this->img->getUsedSpace()==0)&&(this->img->getSerial()==""))
    {
        ui->twFileTree->clear();
        ui->twDirTree->clear();
        this->leAddress->setText("::/");
        this->leLabel->clear();
        this->setWindowTitle("LinImaMT");
        if (this->img)
            this->img->finishProcedure();
        ui->statusBar->showMessage("Could not load image. Maybe bad offset?");
        return 0;
    }
    //Visualize directories
    this->visualize();
    this->visualizeModified();
    ui->twDirTree->setCurrentItem(ui->twDirTree->topLevelItem(0));  //select first item. Do not remove this.
        ui->twFileTree->setDragDropMode(QTreeWidget::DragDrop);
    this->enableUI(1);
    this->img->finishProcedure();

    //insert directory to variable
    QFileInfo aa(fileName);
    this->FileOpenPath=aa.absoluteDir().absolutePath();
    if (this->FileExtractPath=="")
    {
        this->FileExtractPath=this->FileOpenPath;
    }

    return 0;
}

//Save file "as" is always accessible. Then the working file is changed to saved file.
void MainWindow::on_actionSave_As_triggered()
{
    QString fname = QFileDialog::getSaveFileName(this,"Save Image as",this->FileOpenPath,"Disk Images (*.ima *.dsk *.img);;All files (*)");
    if ((!fname.endsWith("img",Qt::CaseInsensitive))&&(!fname.endsWith("ima",Qt::CaseInsensitive))
            &&(!fname.endsWith("dsk",Qt::CaseInsensitive))&&(!fname.endsWith("raw",Qt::CaseInsensitive)))
        fname=fname+".img";

    if (fname!=".img")
    {
        this->img->saveFile(fname);
        this->img->finishProcedure();
//        this->currentFile=fname;

        //Reopen file
        ImageFile::HandleMode currentMode=this->img->getHandleMode();
        int imasize=this->img->getFreeSpace()+this->img->getUsedSpace();
        if ((currentMode==ImageFile::ReadOnly)&&(imasize<this->hdImgSize)) //Read only -> Default
        {
            //reopen the file in ordinary mode
            this->loadFile(fname,ImageFile::DefaultMode);
        }
        else        //Default -> default, Direct -> direct
        {
            this->loadFile(fname,currentMode);
        }

        visualizeModified();
    }

}

//Just save. Img will copy all things needed.
void MainWindow::on_actionSave_triggered()
{
    if (this->currentFile=="")
    {
        this->on_actionSave_As_triggered();
        return;
    }
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
    this->dirs=this->img->getContents("::/");
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
        if ((destination.contains('/'))||(destination.contains(':'))||(destination.contains('\\'))||(destination.contains('|'))||(destination==".")||(destination==".."))
        {
            QMessageBox::critical(this,"Error","Directory name should not contain characters: : / \\ | or be . or .. - directory not created");
                this->statusBarNormal();
                return;
        }


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
        this->img->finishProcedure();

        //refresh
        this->dirs=this->img->getContents("::/");
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

    //Read Only is maintained in image.
    this->img->setSerial(destination);
    this->visualizeModified();
    ui->statusBar->showMessage("Serial number has been changed");
    return;
}

void MainWindow::on_actionVolume_label_triggered()
{
    bool dialogResult;
    QInputDialog *labelDialog = new QInputDialog();

    //show user some fancy thing
    QString g=img->getLabel().trimmed();
    QString destination = labelDialog->getText(this, "Volume label", "Volume label:", QLineEdit::Normal,
                                           g, &dialogResult);
    if ((!dialogResult)||(destination==img->getLabel()))
    {
        this->statusBarNormal();
        return;
    }
    destination=destination.trimmed();
    destination.truncate(11);

    //Read Only is maintained in image.
    this->img->setLabel(destination);
    this->img->getContents("::/");
    this->visualizeModified();
    this->leLabel->setText(this->img->getLabel());
    return;
}

//Copy selected files into filesystem
void MainWindow::on_actionExtract_selected_triggered()
{
    //propose user destination directory
    QString dir = QFileDialog::getExistingDirectory(this, "Destination Directory", this->FileExtractPath, QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
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
         this->img->forceModified(0);
    }

    this->FileExtractPath=dir;

    //refresh
    this->dirs=this->img->getContents("::/");
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
    this->dirs=this->img->getContents("::/");
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

    bool skipAll=0;
    bool overwriteAll=0;
    for (int i=0;i<fileDlg.selectedFiles().count();i++)
    {
         QString fileName=fileDlg.selectedFiles().at(i);
         ui->statusBar->showMessage("Adding "+fileName+" ("+QString::number(i)+"/"+QString::number(fileDlg.selectedFiles().count())+")");
         QApplication::processEvents();

         char replaceMode=this->askForReplacement(skipAll,overwriteAll,fileName,this->leAddress->text());
         if (replaceMode=='-')
             return;

         this->img->copyFile(fileName,this->leAddress->text(),replaceMode);
    }

    //refresh
    this->dirs=this->img->getContents("::/");
    //Visualize directories
    this->visualize();
    this->statusBarNormal();
    return;
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

    bool skipAll=0;
    bool overwriteAll=0; //not used in single-shot approach, but prepared if multiple directories will be selected
    if (fsize>(uint)this->img->getFreeSpace()) //directory size too big consequences
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

    char replaceMode=this->askForReplacement(skipAll,overwriteAll,fileName,this->leAddress->text());
    if (replaceMode=='-')
        return;

    this->img->copyFile(fileName,this->leAddress->text(),replaceMode);

    //refresh
    this->dirs=this->img->getContents("::/");
    //Visualize directories
    this->visualize();
    this->statusBarNormal();
    return;
}

//Edit attributes of selected items
void MainWindow::on_actionAttributes_triggered()
{
    QList<QString> attributes;
    QList<QString> files;
    //get attributes of selected things
    for (int i=0;i<ui->twFileTree->selectedItems().count();i++)
    {
         QString fileAttr=ui->twFileTree->selectedItems().at(i)->text(2);
         QString fileName=ui->twFileTree->selectedItems().at(i)->text(0);
         attributes.append(fileAttr);
         files.append(fileName);
    }

    Attribute * a = new Attribute(attributes,this);
    a->exec();
    if (a->result=="")
    {
        return;
    }

    bool recursive=0;
    if (a->result.at(0)!='-')
        recursive=1;
    QString attribs = a->result.right(4);

    for (int i=0;i<files.count();i++)
    {
        this->img->setAttrbute(this->leAddress->text()+files.at(i),recursive,attribs);
    }


    //refresh
    this->dirs=this->img->getContents("::/");
    //Visualize directories
    this->visualize();
    this->statusBarNormal();
    return;
}

//New image
void MainWindow::on_actionNew_triggered()
{
    if ((this->img!=NULL)&&(this->img->getModified()))
    {
        int k=askForSave();
        if (k==-1)
        {
            return;
        }
        if (k==1)
        {
            ui->actionSave->trigger();
        }
    }

    //new image dialog
    newImage * n = new newImage(this);
    n->exec();
    if (n->result=="")
    {
        return;
    }
    enableUI(0);
    ui->twFileTree->clear();
    ui->twDirTree->clear();
    this->leLabel->clear();
    this->leAddress->setText("::/");

    this->currentFile="";

    if (this->img) this->img->disposeFile();
    this->img=new ImageFile (0,n->result);

    if (!this->img->getModified())
    {
        return;
    }

    this->dirs=this->img->getContents("::/");
    //Visualize directories
    this->visualize();
    this->visualizeModified();
    ui->twDirTree->setCurrentItem(ui->twDirTree->topLevelItem(0));  //select first item. Do not remove this.
        ui->twFileTree->setDragDropMode(QTreeWidget::DragDrop);
    enableUI(1);
    return;
}

//Boot sector properties window
void MainWindow::on_actionVolume_triggered()
{
    bool r=0;
    if (this->img->getHandleMode()==ImageFile::ReadOnly)
    {
        r=1;
    }
    bootSector(this,this->img,-1,512,r).exec();

    //refresh drive info
    this->dirs=this->img->getContents("::/");
    this->visualize();
    this->visualizeModified();
}

//wipe free space
void MainWindow::on_actionWipe_free_space_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Wipe free space",
                                  "This will remove all garbage data from free space, making image compress better.\n"
                                  "On the other side, you will loose ability to undelete.\n"
                                  "In some original software disks this may damage the program (copy protection data).\n"
                                  "Do you want to continue?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply==QMessageBox::No)
        return;

    //1. Get free space from image
    uint freeSpace=this->img->getFreeSpace();

    //If you do floppy disk erasing, it's OK. But imagine you have a 8GB HDD image. Temp dir dies quickly.

    //2. Prepare a zero-filled temp file with free space
    ui->statusBar->showMessage("Generating zero-file...");
    QApplication::processEvents();
    QTemporaryFile tmpF(QDir::temp().absoluteFilePath("$XXXXXXX.bin"));
    QTemporaryFile tmpD(QDir::temp().absoluteFilePath("$$XXXXXX.bin"));
    uint sizeThreshold=134217728; //must be multiply of 1024 (cluster requirements).
    //uint sizeThreshold=153600;
    if (freeSpace>sizeThreshold)
    {
        tmpF.open();
        tmpF.write(QByteArray(sizeThreshold,0));
        tmpF.close();
        tmpD.open();
        tmpD.write(QByteArray(freeSpace%sizeThreshold,0));
        tmpD.close();
    }
    else
    {
        tmpF.open();
        tmpF.write(QByteArray(freeSpace,0));
        tmpF.close();
    }

    //3. Copy the file to image
    ui->statusBar->showMessage("Copying to image...");
    QApplication::processEvents();
    for (uint i=0;i<freeSpace/sizeThreshold;i++)
    {
        this->img->copyFile(tmpF.fileName(),"::/$$wip"+QString::number(i)+".bin");
    }
    if (freeSpace>sizeThreshold)
    {
        this->img->copyFile(tmpD.fileName(),"::/$$wipf.bin");
    }
    else
    {
        this->img->copyFile(tmpF.fileName(),"::/$$wipf.bin");
    }

    //4. Remove the file from image
    ui->statusBar->showMessage("Removing from image...");
    QApplication::processEvents();
    this->img->deleteFile("::/$$wip*.bin");

    tmpF.remove();
    if (freeSpace>sizeThreshold)
    {
        tmpD.remove();
    }

    this->dirs=this->img->getContents("::/"); //update structure too to notify about possible failure of file deletion
    this->visualize();
    this->visualizeModified();
}

//Run Testdisk. Ask about working dir before as user may want to recover files
void MainWindow::on_actionRun_TestDisk_on_image_triggered()
{
    QFileDialog fileDlg(this,"Speciwy working directory foe recovered files","","All files (*)");
    fileDlg.setFileMode(QFileDialog::Directory);
    fileDlg.setOption(QFileDialog::ShowDirsOnly);
    if (!fileDlg.exec())
        return;

    QString testDiskPath = "xterm"; //TO BE PORTED win like cmd /k "/path/to/testdisk.exe" param
    QStringList par;
    par.append("-e");
    par.append("testdisk \""+this->img->getCurrentPath()+"\"");

    QProcess::startDetached(testDiskPath,par,fileDlg.selectedFiles()[0]);
}

void MainWindow::on_actionMBR_triggered()
{
    bool r=0;
    if (this->img->getHandleMode()==ImageFile::ReadOnly)
    {
        r=1;
    }
    bootSector(this,this->img,0,512,r).exec();

    //refresh drive info
    this->dirs=this->img->getContents("::/");
    this->visualize();
    this->visualizeModified();
}

void MainWindow::on_actionQuick_Preview_triggered()
{
    if (ui->twFileTree->currentItem()->text(2).at(0)=='D')
        return; //check if it's not a directory

    QString fileName=ui->twFileTree->currentItem()->text(4);
    QTemporaryDir dir;
    dir.setAutoRemove(0);
    if (!dir.isValid()) {
       QMessageBox::critical(this,"Problem","There was a problem creating temporaty directory.\nPlease try to extract the file manually and view it.");
       return;
    }
    this->img->copyFile(fileName,dir.path()+"/"+ui->twFileTree->currentItem()->text(0));
    this->img->forceModified(0);
    fileName=dir.path()+"/"+ui->twFileTree->currentItem()->text(0);
    QString settingsPath = "imarc.ini";
    #ifndef Q_OS_WIN32
        settingsPath = QDir::homePath()+"/.imarc.ini";
    #endif
    fileViewer * fv = new fileViewer(fileName,settingsPath,ui->twFileTree->currentItem()->text(0));
    fv->exec();
    dir.setAutoRemove(1); //we can safely remove directory
    return;
}

#define FOLDINGEND }


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About", "LinIMAMT: Qt DOS floppy image management tool\n and GNU mtools GUI.\n 2017 MCbx, GNU GPL.");
}
