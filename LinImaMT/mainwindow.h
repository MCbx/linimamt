#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTreeWidgetItem>
#include "imagefile.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_twDirTree_currentItemChanged(QTreeWidgetItem *current);

    void customSortByColumn(int column);

    void on_twFileTree_itemDoubleClicked(QTreeWidgetItem *item);

    void on_actionGoUp_triggered();

    void on_actionAddress_bar_triggered();
    void on_label_edit();

    void on_twFileTree_itemSelectionChanged();

    void on_actionSave_As_triggered();

    void on_actionSave_triggered();

    void on_actionRename_triggered();

    void on_actionCreate_Directory_triggered();

    void on_actionVolume_Serial_triggered();

    void on_actionExtract_selected_triggered();

    void on_actionDelete_selected_triggered();

    void on_actionAdd_triggered();

    void on_actionAdd_Directories_triggered();

    void on_fileDragDrop(QString from, QString to);
private:
    Ui::MainWindow *ui;
    QProcess *process;
    QString currentFile;
    ImageFile * img;

    //structure
    QList<ImageFile::fileEntry>dirs;

    //functions
    void closeEvent(QCloseEvent *clsEv);
    int execute(QString command, QString parameters, QString & result);
    int loadFile(QString fileName);
    int prepareDirDump(QString home);
    int errorMessage(QString text, QString console);
    void visualize();
    void statusBarNormal();
    void visualizeModified();
    void saveSettings();
    void loadSettings();

    //ui components
    QLineEdit * leAddress;
    QLineEdit * leLabel;
};

#endif // MAINWINDOW_H
