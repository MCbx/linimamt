#ifndef IMAGEFILE_H
#define IMAGEFILE_H
#include <QString>
#include <QList>
#include <QTemporaryFile>
#include "errordialog.h"

class ImageFile
{
public:
    struct fileEntry {
        QString name; //name contains folder.
        unsigned long size;
        QString date;
        QString attrib;
    };


    ImageFile(QString imagePath); //open existing
    ImageFile(int imageSize, QString imageInit);   //create new, imagesize doesn't work yet, string is string for formatter
    void disposeFile(); //disposes temp file is present
    int copyFile(QString source, QString destination);
    int moveFile(QString source, QString destination);
    int deleteFile(QString source);
    int makeFolder(QString path);
    QList<fileEntry> getContents(QString home);              //return contents for visualization
                                                             //also used to reload basic attributes like serial or label
    int getFreeSpace();
    int getUsedSpace();
    QString getLabel();
    QString getSerial();
    QString setLabel(QString label);
    void setSerial(QString serial);
    void setAttrbute(QString file, bool recursive, QString attribs);
    bool getModified();
    int prepareForModify();          //this one creates needed copies and reloads  things

    int saveFile(QString fileName);
    QString getCurrentPath();
    void forceModified(bool); //forces modified state


private:

    int execute(QString command, QString parameters, QString &result);
    int errorMessage(QString text, QString console);

    bool modified;
    QString originalPath;   //the path which is used for original file storage
    QString currentPath;    //Path for currently edited file - may be temporary file.
    int freeSpace;
    int usedSpace;
    QString label;
    QString serial;
    QTemporaryFile * tmpF;
};

#endif // IMAGEFILE_H
