#include "filemanager.h"

FileManager::FileManager(QWidget* parent)
    : QWidget(parent)
{

}

bool FileManager::openFile()
{
    QString file_name = QFileDialog::getOpenFileName(this,
                                                     "Choose file",
                                                     QDir::homePath(),
                                                     "(*.pixl)");
    return true;
}
bool FileManager::createFile()
{
    return true;
}

bool FileManager::saveFile()
{
    return true;
}

bool FileManager::saveAsFile()
{
    return true;
}
bool FileManager::printFile()
{
     return true;
}
bool FileManager::closeFile()
{
     return true;
}
