#ifndef WORKSPACECONTROLLER_H
#define WORKSPACECONTROLLER_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene> // Добавили
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QDebug>
#include <QUndoStack>

#include "projectmanager.h"
#include "object.h"
#include "instrumentpannel.h"
#include "manipulator.h"

class WorkspaceController : public QObject
{
    Q_OBJECT
public:
    // ТЕПЕРЬ МЫ ЯВНО ПЕРЕДАЕМ СЦЕНУ
    explicit WorkspaceController(QGraphicsView* view, QGraphicsScene* scene, ProjectManager* projectManager, QObject *parent = nullptr);

public slots:
    void setCurrentTool(InstrumentType type);
    void onSelectionChanged();

signals:
    void viewportChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QGraphicsView* m_view;
    ProjectManager* m_project_manager;
    TransformBox* m_transform_box = nullptr;
    QUndoStack* m_undo_stack;

    InstrumentType m_current_tool = InstrumentType::POINTER;

    bool m_space_pressed = false;
    bool m_is_panning = false;
    QPoint m_last_pan_pos;

    bool m_is_drawing = false;
    QPointF m_draw_start_pos;
    Ellipse* m_temp_ellipse = nullptr;
};

#endif // WORKSPACECONTROLLER_H