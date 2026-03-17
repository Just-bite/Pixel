#ifndef WORKSPACECONTROLLER_H
#define WORKSPACECONTROLLER_H

#include <QObject>
#include <QGraphicsView>
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QDebug>

#include "projectmanager.h"
#include "object.h"
#include "instrumentpannel.h" // Для InstrumentType

class WorkspaceController : public QObject
{
    Q_OBJECT
public:
    explicit WorkspaceController(QGraphicsView* view, ProjectManager* projectManager, QObject *parent = nullptr);

public slots:
    void setCurrentTool(InstrumentType type); // Слот для смены инструмента

signals:
    void viewportChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QGraphicsView* m_view;
    ProjectManager* m_project_manager;

    InstrumentType m_current_tool = InstrumentType::POINTER; // По умолчанию Указатель

    bool m_space_pressed = false;
    bool m_is_panning = false;
    QPoint m_last_pan_pos;

    bool m_is_drawing = false;
    QPointF m_draw_start_pos;
    Ellipse* m_temp_ellipse = nullptr;
};

#endif // WORKSPACECONTROLLER_H