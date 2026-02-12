#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "project.h"
#include "filemanager.h"

class ProjectManager : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectManager(QWidget* parent = nullptr);

public slots:
    bool openFile();
    bool createFile();
    bool saveFile();
    bool saveAsFile();
    bool printFile();
    bool closeFile();

private:
    std::vector<Project*> m_projects;
    Project* m_selected_project;
    FileManager* m_file_manager;
};

#endif // PROJECTMANAGER_H
