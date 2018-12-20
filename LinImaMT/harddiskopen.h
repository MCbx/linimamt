#ifndef HARDDISKOPEN_H
#define HARDDISKOPEN_H

#include <QDialog>
#include <imagefile.h>
#include <QListWidgetItem>

namespace Ui {
class HardDiskOpen;
}

class HardDiskOpen : public QDialog
{
    Q_OBJECT

public:
    explicit HardDiskOpen(QWidget *parent = 0, QString imageFile="");
    ~HardDiskOpen();
    qint64 getOffset();
    ImageFile::HandleMode getMode();

private slots:
    void on_cbCustomOffset_clicked(bool checked);

    void on_HardDiskOpen_accepted();

    void on_lwPartitions_currentRowChanged();

    void on_lwPartitions_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::HardDiskOpen *ui;
    ImageFile::HandleMode mode;
    qint64 offset;
};

#endif // HARDDISKOPEN_H
