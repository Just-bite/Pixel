#include "projectmanager.h"

ProjectManager::ProjectManager(QWidget* parent)
    : QWidget(parent)
{
    m_file_manager = new FileManager(this);
}

bool ProjectManager::openFile()
{
    return m_file_manager->openFile();
}

bool ProjectManager::createFile()
{
    return m_file_manager->createFile();
}

bool ProjectManager::saveFile()
{
    return m_file_manager->saveFile();
}

bool ProjectManager::saveAsFile()
{
    return m_file_manager->saveAsFile();
}

bool ProjectManager::printFile()
{
    return m_file_manager->printFile();
}

bool ProjectManager::closeFile()
{
    return m_file_manager->closeFile();
}
