#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QFileDialog>

class FileManager : public QWidget
{
    Q_OBJECT
public:
    explicit FileManager(QWidget* parent = nullptr);
    bool openFile();
    bool createFile();
    bool saveFile();
    bool saveAsFile();
    bool printFile();
    bool closeFile();
};

#endif // FILEMANAGER_H
