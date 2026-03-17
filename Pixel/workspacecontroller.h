#ifndef WORKSPACECONTROLLER_H
#define WORKSPACECONTROLLER_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
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
#include "contextpannel.h"
#include "palettepannel.h"
#include "manipulator.h"

class WorkspaceController : public QObject
{
    Q_OBJECT
public:
    explicit WorkspaceController(QGraphicsView* view, QGraphicsScene* scene,
                                 ProjectManager* projectManager, ContextPannel* contextPannel,
                                 PalettePannel* palettePannel, QObject *parent = nullptr);

public slots:
    void setCurrentTool(InstrumentType type);
    void onSelectionChanged();
    void onContextPropertyChanged();
    void onColorTargetChanged(bool isFill);
    void onColorPicked(const QColor& color);

signals:
    void viewportChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void clearTransformBox();

    QGraphicsView* m_view;
    ProjectManager* m_project_manager;
    ContextPannel* m_context_pannel;

    TransformBox* m_transform_box = nullptr;
    QUndoStack* m_undo_stack;

    Figure* m_selected_figure = nullptr;
    InstrumentType m_current_tool = InstrumentType::POINTER;

    bool m_space_pressed = false;
    bool m_is_panning = false;
    QPoint m_last_pan_pos;

    bool m_is_drawing = false;
    QPointF m_draw_start_pos;
    Figure* m_temp_figure = nullptr;

    bool m_color_target_is_fill = true;
};

#endif // WORKSPACECONTROLLER_H