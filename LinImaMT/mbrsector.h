#ifndef MBRSECTOR_H
#define MBRSECTOR_H

#include <QDialog>
#include "imagefile.h"

namespace Ui {
class MBRSector;
}

class MBRSector : public QDialog
{
    Q_OBJECT

public:
    explicit MBRSector(QWidget *parent = 0, ImageFile * image = NULL, qint64 offset = -1, int length = 512, bool ReadOnly = 0);
    ~MBRSector();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_cbActive1_clicked();

    void on_cbActive2_clicked();

    void on_cbActive3_clicked();

    void on_cbActive4_clicked();

    void on_sbType1_valueChanged(int arg1);

    void on_sbType2_valueChanged(int arg1);

    void on_sbType3_valueChanged(int arg1);

    void on_sbType4_valueChanged(int arg1);

    void on_leSignature_editingFinished();

    void on_buttonBox_accepted();

private:
    Ui::MBRSector *ui;
    void refreshView();

    QByteArray sectorData; //this holds the data
    ImageFile * image; //path to image
    int offset; //offset of sector, default 0
    int length; //length of sector, default 512.
    bool modified;
};

#endif // MBRSECTOR_H
