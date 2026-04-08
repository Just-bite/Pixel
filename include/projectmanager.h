#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QWidget>
#include <QString>
#include <vector>
#include <QMessageBox>
#include "project.h"

class ProjectManager : public QWidget {
    Q_OBJECT
public:
    explicit ProjectManager(QWidget* parent = nullptr);
    Canvas* GetCurrentCanvas();
    Project* getCurrentProject() { return m_selected_project; }
    void createProject();

    // Новые методы контроля состояния проекта
    bool promptSaveIfModified();
    void setModified(bool modified) { m_is_modified = modified; }
    QString getCurrentFilePath() const { return m_current_file_path; }

signals:
    void projectAboutToClose();
    void projectLoaded();
    void layersUpdated();

    // Новые сигналы для UI и синхронизации
    void statusMessage(const QString& msg);
    void projectSaved();

public slots:
    bool createFile();
    bool openFile();
    bool saveFile();
    bool saveAsFile();
    bool exportPng();

private:
    void saveToJson(const QString& path);
    void loadFromJson(const QString& path);

    std::vector<Project*> m_projects;
    Project* m_selected_project;
    QString m_current_file_path;
    bool m_is_modified = false; // Флаг измененного проекта
};

#endif // PROJECTMANAGER_H