#ifndef FILEVIEWER_H
#define FILEVIEWER_H

#include <QDialog>

namespace Ui {
class fileViewer;
}

class fileViewer : public QDialog
{
    Q_OBJECT

public:
    explicit fileViewer(QWidget *parent = 0);
    ~fileViewer();

private:
    Ui::fileViewer *ui;
};

#endif // FILEVIEWER_H
