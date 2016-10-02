#include "imagefile.h"
#include <QProcess>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QDir>

//open image
ImageFile::ImageFile(QString imagePath, HandleMode a, qint64 offset)
{
    this->procedureError=NULL;
    this->tmpF=NULL;
    this->currentPath=imagePath;
    this->originalPath=imagePath;
    this->modified=0;
    this->operationMode=a;
    this->offset=offset;
}

//create new
ImageFile::ImageFile(int imageSize, QString imageInit, HandleMode a)
{
    this->modified=0;
    this->procedureError=NULL;
    if (imageSize<0)
            return;
    //create new tmp file
    this->offset=0;
    this->tmpF = new QTemporaryFile(QDir::temp().absoluteFilePath("imaXXXXXXXX.img"));
    this->tmpF->open();
    this->tmpF->close();
    //change current path to temporary file
    this->currentPath=this->tmpF->fileName();
    this->originalPath=this->currentPath; //temporary hack


    //execute mtools formatter on tmp file
    QString op;
    int status=this->execute("mformat",imageInit,op);
    if ((status!=0)||(op.contains("\nTrouble ")))
    {
        errorMessage(status,"Failed to format the image. Code "+QString::number(status),op);
        return;
    }
    this->operationMode=a;
    this->modified=1;
    return;
}

//Terminates the image removing its temp file
void ImageFile::disposeFile()
{
    if (this->tmpF!=NULL)
    {
      //  if (this->currentPath==this->tmpF->fileName())
        if (this->tmpF->exists())
        {
            this->tmpF->remove();
        }
    }
}

///////////////////////////
/// GENERAL GET/SETTERS ///
///////////////////////////

int ImageFile::getFreeSpace()
{
    return this->freeSpace;
}
int ImageFile::getUsedSpace()
{
    return this->usedSpace;
}
QString ImageFile::getLabel()
{
    return this->label;
}
QString ImageFile::getSerial()
{
    return this->serial;
}
bool ImageFile::getModified()
{
    return this->modified;
}
QString ImageFile::getCurrentPath()
{
    return this->currentPath;
}
ImageFile::HandleMode ImageFile::getHandleMode()
{
    return this->operationMode;
}
qint64 ImageFile::getOffset()
{
    return this->offset;
}

//////////////////////////////
/// IMAGE FILE MAINTENANCE ///
//////////////////////////////

//execute mtools with specific command on currently loaded image.
int ImageFile::execute(QString command, QString parameters, QString &result)       //TO BE PORTED
{
    QProcess executing;
    executing.setProcessChannelMode(QProcess::MergedChannels);
    QString partOffset="";
    if (this->offset>-1)
    {
        partOffset="@@"+QString::number(this->offset);
    }
    executing.start("mtools -c "+command+" -i \""+this->currentPath+"\""+partOffset+" "+parameters);
    executing.waitForFinished();

    QString op(executing.readAllStandardOutput());
   // op=op+"\n\n\n\n"+executing.readAllStandardError();
    result=op;
    result=result+"\n\nWhile: "+"mtools -c "+command+" -i \""+this->currentPath+"\" "+parameters;
    return executing.exitCode();
}

//throw a nice, GUI-based error message with a console
int ImageFile::errorMessage(int code, QString text, QString console)
{
    if (this->procedureError!=NULL)
    {
        this->procedureError->append(code,text,console);
    }
    else
    {
        this->procedureError=new ErrorDialog(0,"","");
        this->procedureError->append(code,text,console);
    }
}

//this function prepares file for modification, it does NOT set modified flag yet.
int ImageFile::prepareForModify()
{
    if (this->operationMode==ImageFile::ReadOnly)
        return -2; // file is read only. This should never happen
    if (this->operationMode==ImageFile::DirectMode)
        return 0;  //file is directly modified on disk

    if (this->currentPath!=this->originalPath)
    {
        return -1;  //already put into modification
    }
    if (this->tmpF)
    {
        if (this->originalPath==this->tmpF->fileName())
            return -1;
    }
    //1. create temporary file
    this->tmpF = new QTemporaryFile(QDir::temp().absoluteFilePath("imaXXXXXXXX.img"));
    //2. copy source image to temporary file
    this->tmpF->open();
    this->tmpF->close();
    QFile destination(this->tmpF->fileName());
    QFile source(this->currentPath);
    source.open(QIODevice::ReadOnly);
    destination.open(QIODevice::ReadWrite);
    QByteArray buffer;
    int chunksize = 256;
    while(!(buffer = source.read(chunksize)).isEmpty()){
        destination.write(buffer);
    }
    tmpF->close();
    source.close();
    destination.close();
    //3. change current path to temporary file
    this->currentPath=this->tmpF->fileName();
    //this->modified=1;
    return 0;
}

int ImageFile::saveFile(QString fileName)
{
    //if fileName="", then save to originalPath
    if (fileName=="")
    {
        if (!this->modified)
            return 2;
        QFile source(this->currentPath);
        QFile destination(this->originalPath);
        source.open(QIODevice::ReadOnly);
        destination.open(QIODevice::ReadWrite);
        QByteArray buffer;
        int chunksize = 256;
        while(!(buffer = source.read(chunksize)).isEmpty()){
            destination.write(buffer);
        }
        source.close();
        destination.close();
        this->modified=0;
        return 0;
    }
    else    //save as
    {
        QFile source(this->currentPath);
        QFile destination(fileName);
        source.open(QIODevice::ReadOnly);
        destination.open(QIODevice::ReadWrite);
        QByteArray buffer;
        int chunksize = 256;
        while(!(buffer = source.read(chunksize)).isEmpty()){
            destination.write(buffer);
        }
        source.close();
        destination.close();
        this->modified=0;
        this->currentPath=fileName;
        this->originalPath=fileName;
        return 0;
    }
    //if name is specified, just copy the file.
}

//Forces modification state. Use with care. Use only if the file has been prepared for modify
//and the modification was performed outside of this class.
void ImageFile::forceModified(bool mod)
{
    if (this->operationMode==ImageFile::ReadOnly)
        return;

    this->modified=mod;
}

//shuts a series of operations down giving a log if error happened
//Dear GUI programmer, please do it after series of ImageFile procedure calls
void ImageFile::finishProcedure()
{
    if (this->procedureError!=NULL)
    {
        this->procedureError->showIt(0);
        this->procedureError=NULL;
    }
}

///////////////////////////
///    IMAGE EDITING    ///
///////////////////////////

QList<ImageFile::fileEntry> ImageFile::getContents(QString home)
{
        this->freeSpace=0;
        this->usedSpace=0;
        QList<ImageFile::fileEntry> dirs;
        QString op="";

        //When image is blank, mtools returns error. First, we will make a dumb try to read root directory.
        int status=this->execute("mdir","-a \""+home+"\"",op);
        if (op.contains("Directory for ::/")&&op.contains("No files"))
        {
            //we are in this situation
            //the only thing we will get: Label, serial, bytes free. Nothing more.
            QStringList lines = op.split('\n');
            QString tmp=lines[0];
            this->label=tmp.mid(tmp.indexOf("is")+3);
            if (this->label.contains("has no label",Qt::CaseInsensitive))
            {
                this->label="";
            }
            int abbrPos=this->label.indexOf(" (abbr=");
            if (abbrPos>0)
            {
                this->label=this->label.left(abbrPos);
            }
            tmp=lines[1];
            this->serial=tmp.mid(tmp.indexOf("is")+3);

            if (lines[5].indexOf("bytes free")>=0)
            {
                QString a=lines[5];
                a=a.replace("bytes free","").trimmed();
                a=a.replace(" ","");
                this->freeSpace=a.toInt();
            }
            return dirs;
        }

        status=this->execute("mdir","-/ -a \""+home+"\"",op);
        if (status!=0)
        {
            errorMessage(status,"Failed to acquire listing, code: "+QString::number(status),op);
            return dirs;
        }

        QString qsattrs;
        status=this->execute("mattrib","-/ -X \""+home+"\"",qsattrs);
       // if (status!=0)
       // {
       //     errorMessage(status,"Failed to acquire attributes, code: "+QString::number(status),qsattrs);
       //     return dirs;
       // } //mtools reports presence of directories only as error condition.
        QStringList attrs = qsattrs.split('\n');

        //Parse results
        QStringList lines = op.split('\n');

        //volume label and serial
        QString tmp=lines[0];
        this->label=tmp.mid(tmp.indexOf("is")+3);
        if (this->label.contains("has no label",Qt::CaseInsensitive))
        {
            this->label="";
        }
        tmp=lines[1];
        this->serial=tmp.mid(tmp.indexOf("is")+3);
        //QMessageBox::critical(this,"result",this->label+"\n"+this->serial);

        int lineCount=2;
        QString myHome;
        while (lineCount<lines.count())
        {
            if ((lines[lineCount].length()==0)||(lines[lineCount].indexOf("files       ")>0)||(lines[lineCount].indexOf("file        ")>0) ||
                    (lines[lineCount].indexOf(".            <DIR>")>=0)||
                       (lines[lineCount].indexOf("..           <DIR>")>=0))
            {
                lineCount++;
                continue;
            }
            if (lines[lineCount].indexOf("Total files listed:")>=0)
            {

                QString a=lines[lineCount+2];
                a=a.replace("bytes free","").trimmed();
                a=a.replace(" ","");
                this->freeSpace=a.toInt();
                break;
            }
            if ((lines[lineCount].indexOf("bytes free")>=0)&&(lines[lineCount].startsWith("      ")))
            {
                QString a=lines[lineCount];
                a=a.replace("bytes free","").trimmed();
                a=a.replace(" ","");
                this->freeSpace=a.toInt();
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

                dirs.append(plik);
            }
            lineCount++;
        }

        for (int i=0;i<dirs.count();i++)
        {
            this->usedSpace+=dirs[i].size;
        }
    //    QString m="";
    //    for (int i=0;i<dirs.count();i++)
    //    {
    //        m=m+dirs[i].name+"  "+QString::number(dirs[i].size)+"  "+dirs[i].attrib+"  "+dirs[i].date+"\n";
    //    }
    //    QMessageBox::critical(this,"aaa",m);
        return dirs;
}

QString ImageFile::setLabel(QString label)
{
    if (this->operationMode==ImageFile::ReadOnly)
        return "";
    this->prepareForModify();
    this->modified=1;
    QString res;
    int code=this->execute("mlabel","::\""+label.left(11)+"\"",res);
    if (code!=0)
    {
        this->errorMessage(code,"Error while setting label. Code: "+QString::number(code),res);
    }
    return "";
}

int ImageFile::moveFile(QString source, QString destination, char defaultAction)
{
    if (this->operationMode==ImageFile::ReadOnly)
        return -1;
    this->prepareForModify();
    QString op;
    if ((destination.contains("\\..\\"))||(destination.contains("/../")))
    {
        errorMessage(200,"Internal error","Relative path passed");
        return 2;
    }
    QString replaceAction="";
    if (defaultAction!='0')
        replaceAction="-D "+QString(defaultAction);

    int status=this->execute("mren",replaceAction+" \""+source+"\" \""+destination+"\"",op);
    if (status!=0)
    {
        errorMessage(status,"Failed to rename. Code "+QString::number(status),op);
        return 1;
    }
    this->modified=1;
    return 0;
}

int ImageFile::copyFile(QString source, QString destination, char defaultAction)
{
    if (this->operationMode==ImageFile::ReadOnly)
        return -1;
    this->prepareForModify();
    QString op;
    if ((destination.contains("\\..\\"))||(destination.contains("/../")))
    {
        errorMessage(200,"Internal error","Relative path passed");
        return 2;
    }
    QString replaceAction="";
    if (defaultAction!='0')
        replaceAction="-D "+QString(defaultAction)+" ";

    int status=this->execute("mcopy","-s -p -n -m -v -Q "+replaceAction+"\""+source+"\" \""+destination+"\"",op);
    if (status!=0)
    {
        errorMessage(status,"Failed to copy. Code "+QString::number(status),op);
        this->modified=1; //Filed copying may left the image in unknown state - we will mark it as modified.
        return 1;
    }
    this->modified=1;
    return 0;
}

int ImageFile::makeFolder(QString path)
{
    if (this->operationMode==ImageFile::ReadOnly)
        return -1;
    this->prepareForModify();
    QString op;
    if ((path.contains("\\..\\"))||(path.contains("/../")))
    {
        errorMessage(200,"Internal error","Relative path passed");
        return 2;
    }
    int status=this->execute("mmd"," \""+path+"\"",op);
    if (status!=0)
    {
        errorMessage(status,"Failed to add folder. Code "+QString::number(status),op);
        return 1;
    }
    this->modified=1;
    return 0;
}

int ImageFile::deleteFile(QString source)
{
    if (this->operationMode==ImageFile::ReadOnly)
        return -1;
    this->prepareForModify();
    QString op="";
    if ((source.contains("\\..\\"))||(source.contains("/../")))
    {
        errorMessage(200,"Internal error","Relative path passed");
        return 2;
    }
    int status=this->execute("mdeltree","-v \""+source+"\"",op);
    if (status!=0)
    {
        errorMessage(status,"Failed to delete. Code "+QString::number(status),op);
        return 1;
    }
    this->modified=1;
    return 0;
}

void ImageFile::setSerial(QString serial)
{
    if (this->operationMode==ImageFile::ReadOnly)
        return;
    this->prepareForModify();
    QString op;
    int status=this->execute("mlabel","::\""+this->label+"\" -N "+serial,op);
    if (status!=0)
    {
        errorMessage(status,"Failed to modify serial number. Code "+QString::number(status),op);
        return;
    }
    this->modified=1;
    this->serial=serial;
    return;
}

void ImageFile::setAttrbute(QString file, bool recursive, QString attribs)
{
    if (this->operationMode==ImageFile::ReadOnly)
        return;
    //set attributes. Letter - is, dash - remove, X - doesn't matter.
    this->prepareForModify();

    QString attributes="";
    if (attribs.at(0)=='-')
        attributes+=" -a";
    if (attribs.at(0)=='a')
        attributes+=" +a";
    if (attribs.at(1)=='-')
        attributes+=" -r";
    if (attribs.at(1)=='r')
        attributes+=" +r";
    if (attribs.at(2)=='-')
        attributes+=" -h";
    if (attribs.at(2)=='h')
        attributes+=" +h";
    if (attribs.at(3)=='-')
        attributes+=" -s";
    if (attribs.at(3)=='s')
        attributes+=" +s";

    if (recursive)
        attributes+=" -/";
    attributes=attributes.trimmed();

    QString op="";
    int status=this->execute("mattrib", attributes+" \""+file+"\"",op);

    if (status!=0)
    {
        errorMessage(status,"Failed to set attribute. Code "+QString::number(status),op);
        return;
    }

    this->modified=1;
    return;
}

