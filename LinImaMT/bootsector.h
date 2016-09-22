#ifndef BOOTSECTOR_H
#define BOOTSECTOR_H

#include <QDialog>
#include "imagefile.h"

namespace Ui {
class bootSector;
}

class bootSector : public QDialog
{
    Q_OBJECT

public:
    explicit bootSector(QWidget *parent = 0, ImageFile * image = NULL, int offset = 0, int length = 512);
    ~bootSector();

private slots:
    void on_leOEMString_textEdited(const QString &arg1);

    void on_leSerial_textEdited(const QString &arg1);

    void on_leLabel_textEdited(const QString &arg1);

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_bootSector_accepted();

    void on_cbPreserveBIOS_clicked();

private:
    Ui::bootSector *ui;
    void refreshView();

    QByteArray sectorData; //this holds the data
    ImageFile * image; //path to image
    int offset; //offset of sector, default 0
    int length; //length of sector, default 512.
    bool modified;
};

#endif // BOOTSECTOR_H
