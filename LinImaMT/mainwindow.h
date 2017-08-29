#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTreeWidgetItem>
#include <QTemporaryDir>
#include "imagefile.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QStringList arguments, QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();
    void on_actionOpen_triggered(ImageFile::HandleMode mode=ImageFile::DefaultMode);
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
    void on_fileDragDrop(QStringList from, QString to);
    void on_actionAttributes_triggered();
    void on_actionNew_triggered();
    void on_twFileTree_customContextMenuRequested(const QPoint &pos);
    void on_actionVolume_triggered();
    void on_actionWipe_free_space_triggered();
    void on_actionOpen_as_Read_only_triggered();
    void on_actionRun_TestDisk_on_image_triggered();

    void on_actionMBR_triggered();

    void on_actionOptions_triggered();

    void on_actionQuick_Preview_triggered();

    void on_actionVolume_label_triggered();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    QProcess *process;
    QString currentFile;
    ImageFile * img;

    QString defaultViewer;
    QList <QString> viewers;
    QList <QString> extensions;
    QList <QString> deletion; //used to specify temporary files which will be deleted on exit

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

    //Paths to be saved.
    QString FileOpenPath;   //Initialized to read from settings
    QString FileExtractPath; //initialized to FileOpenPath

    //structure
    QList<ImageFile::fileEntry>dirs;

    //functions
    void closeEvent(QCloseEvent *clsEv);
    int execute(QString command, QString parameters, QString & result);
    int loadFile(QString fileName, ImageFile::HandleMode mode = ImageFile::DefaultMode);
    int prepareDirDump(QString home);
    int errorMessage(QString text, QString console);
    void visualize();
    void statusBarNormal();
    void visualizeModified();
    void saveSettings();
    void loadSettings();
    void enableUI(bool state);
    int askForSave();
    qint64 hdImgSize;

    char askForReplacement(bool &SkipAll, bool &overwriteAll, QString from, QString to);

    //ui components
    QLineEdit * leAddress;
    QLineEdit * leLabel;
};

#endif // MAINWINDOW_H
