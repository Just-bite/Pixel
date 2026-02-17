#include "projectmanager.h"

#include <QDebug>

ProjectManager::ProjectManager()
    : m_selected_project(nullptr)
{
}

void ProjectManager::createProject()
{
    Project* project = new Project();
    m_projects.push_back(project);
    if(m_projects.size() == 1)
    {
        m_selected_project = project;
    }
}

Canvas* ProjectManager::GetCurrentCanvas()
{
    if(!m_selected_project) {
        qDebug() << "err: trying to use NULL project";
        return nullptr;
    }
    return m_selected_project->GetCanvas();
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
