#include "imagefile.h"
#include <QProcess>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QDir>

ImageFile::ImageFile(QString imagePath)
{
    this->currentPath=imagePath;
    this->originalPath=imagePath;
    this->modified=0;
}

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

QString ImageFile::setLabel(QString label)
{

    this->prepareForModify();
    this->modified=1;
    QString res;
    int code=this->execute("mlabel","::\""+label.left(8)+"\"",res);
    if (code!=0)
    {
        this->errorMessage("Error while setting label. Code: "+QString::number(code),res);
    }
    return "";
}

QList<ImageFile::fileEntry> ImageFile::getContents(QString home)
{
        this->freeSpace=0;
        this->usedSpace=0;
        QList<ImageFile::fileEntry> dirs;
        QString op;
        int status=this->execute("mdir","-/ -a \""+home+"\"",op);
        if (status!=0)
        {
            errorMessage("Failed to acquire listing, code: "+QString::number(status),op);
            return dirs;
        }

        QString qsattrs;
        status=this->execute("mattrib","-/ -X \""+home+"\"",qsattrs);
        if (status!=0)
        {
            errorMessage("Failed to acquire attributes, code: "+QString::number(status),qsattrs);
            return dirs;
        }
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
            if ((lines[lineCount].length()==0)||(lines[lineCount].indexOf("files       ")>0)||
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
            if (lines[lineCount].indexOf("bytes free")>=0)
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

int ImageFile::moveFile(QString source, QString destination)
{
    this->prepareForModify();
    QString op;
    if ((destination.contains("\\..\\"))||(destination.contains("/../")))
    {
        errorMessage("Internal error","Relative path passed");
        return 2;
    }
    int status=this->execute("mren"," \""+source+"\" \""+destination+"\"",op);
    if (status!=0)
    {
        errorMessage("Failed to rename. Code "+QString::number(status),op);
        return 1;
    }
    this->modified=1;
    return 0;
}

int ImageFile::makeFolder(QString path)
{
    this->prepareForModify();
    QString op;
    if ((path.contains("\\..\\"))||(path.contains("/../")))
    {
        errorMessage("Internal error","Relative path passed");
        return 2;
    }
    int status=this->execute("mmd"," \""+path+"\"",op);
    if (status!=0)
    {
        errorMessage("Failed to add folder. Code "+QString::number(status),op);
        return 1;
    }
    this->modified=1;
    return 0;
}

void ImageFile::setSerial(QString serial)
{
    this->prepareForModify();
    QString op;
    int status=this->execute("mlabel","::\""+this->label+"\" -N "+serial,op);
    if (status!=0)
    {
        errorMessage("Failed to modify serial number. Code "+QString::number(status),op);
        return;
    }
    this->modified=1;
    this->serial=serial;
    return;
}

//execute mtools with specific command on currently loaded image.
int ImageFile::execute(QString command, QString parameters, QString &result)       //TO BE PORTED
{
    QProcess executing;
    executing.setProcessChannelMode(QProcess::MergedChannels);
    executing.start("mtools -c "+command+" -i \""+this->currentPath+"\" "+parameters);
    executing.waitForFinished();

    QString op(executing.readAllStandardOutput());
   // op=op+"\n\n\n\n"+executing.readAllStandardError();
    result=op;
    result=result+"\n\nWhile: "+"mtools -c "+command+" -i \""+this->currentPath+"\" "+parameters;
    return executing.exitCode();
}

//throw a nice, GUI-based error message with a console
int ImageFile::errorMessage(QString text, QString console)
{
    ErrorDialog * a = new ErrorDialog(NULL,text,console);
    a->show();
    return 0;
}

//this function prepares file for modification
int ImageFile::prepareForModify()
{
    if (this->currentPath!=this->originalPath)
    {
        return -1;  //already put into modification
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
