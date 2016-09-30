#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent = 0, QString text="", QString cons="");
    ~ErrorDialog();
    void append(int code, QString text, QString console);
    void showIt(bool always);

private:
    Ui::ErrorDialog *ui;
    int code;
    QString console;
    QString text;
};

#endif // ERRORDIALOG_H
