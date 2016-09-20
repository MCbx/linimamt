#ifndef NEWIMAGE_H
#define NEWIMAGE_H

#include <QDialog>

namespace Ui {
class newImage;
}

class newImage : public QDialog
{
    Q_OBJECT

public:
    explicit newImage(QWidget *parent = 0);
    ~newImage();
    QString result;

private slots:
    void on_sbSectorSize_valueChanged(int arg1);

    void on_sbClust_valueChanged(int arg1);

    void on_radioButton_13_clicked();

    void on_cb2M_clicked(bool checked);

    void on_sbSPT_valueChanged(int arg1);

    void on_sbCyls_valueChanged(int arg1);

    void on_sbHeads_valueChanged(int arg1);

    void on_radioButton_2_clicked();

    void on_radioButton_3_clicked();

    void on_radioButton_4_clicked();

    void on_radioButton_5_clicked();

    void on_radioButton_6_clicked();

    void on_radioButton_7_clicked();

    void on_radioButton_8_clicked();

    void on_radioButton_9_clicked();

    void on_radioButton_12_clicked();

    void on_radioButton_10_clicked();

    void on_radioButton_14_clicked();

    void on_radioButton_11_clicked();

    void on_newImage_accepted();

private:
    Ui::newImage *ui;
    void dismantleCustom(bool b);
    void updateImageSize();
};

#endif // NEWIMAGE_H
