#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class optionsDialog;
}

class optionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit optionsDialog(qint64 *hdImgSize, QString *defViever, QList<QString> *exts, QList<QString> *viewers, QWidget *parent = 0);
    ~optionsDialog();

private slots:
    void on_rbInternal_clicked();

    void on_rbDefault_clicked();

    void on_rbApplication_clicked();

    void on_optionsDialog_accepted();

    void on_lwExtensons_currentRowChanged(int currentRow);

    void on_btnAdd_clicked();

    void on_btnRemove_clicked();

    void on_leExtensioins_editingFinished();

    void on_leCommand_editingFinished();

private:
    qint64 *hdImgSize;
    QString *defViever;
    QList<QString> *exts;
    QList<QString> *viewers;
    Ui::optionsDialog *ui;

    void repaintList();
};

#endif // OPTIONSDIALOG_H
