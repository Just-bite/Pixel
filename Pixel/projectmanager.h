#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QWidget>
#include <QString>
#include <vector>
#include "project.h"

class ProjectManager : public QWidget {
    Q_OBJECT
public:
    explicit ProjectManager(QWidget* parent = nullptr);
    Canvas* GetCurrentCanvas();
    void createProject();

signals:
    // Сигнал окну, чтобы оно сбросило историю Undo при открытии файла
    void projectLoaded();
    void layersUpdated();

public slots:
    bool createFile();
    bool openFile();
    bool saveFile();
    bool saveAsFile();
    bool exportPng(); // Заменил printFile на exportPng

private:
    void saveToJson(const QString& path);
    void loadFromJson(const QString& path);

    std::vector<Project*> m_projects;
    Project* m_selected_project;
    QString m_current_file_path; // Храним путь текущего проекта
};

#endif // PROJECTMANAGER_H