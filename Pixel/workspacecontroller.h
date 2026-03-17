#ifndef WORKSPACECONTROLLER_H
#define WORKSPACECONTROLLER_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QEvent>
#include <QUndoStack>

#include "projectmanager.h"
#include "object.h"
#include "instrumentpannel.h"
#include "contextpannel.h"
#include "palettepannel.h"
#include "layerspannel.h"
#include "manipulator.h"

class WorkspaceController : public QObject {
    Q_OBJECT
public:
    explicit WorkspaceController(QGraphicsView* view, QGraphicsScene* scene, ProjectManager* projectManager, ContextPannel* contextPannel, PalettePannel* palettePannel, LayersPannel* layersPannel, QObject *parent = nullptr);

public slots:
    void setCurrentTool(InstrumentType type);
    void onSelectionChanged();
    void onContextPropertyChanged();
    void onColorTargetChanged(bool isFill);
    void onColorPickedPreview(const QColor& color);
    void onColorPickedCommit(const QColor& color);
    void onMoveObjectLayerRequested(int shift);

signals:
    void viewportChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void clearTransformBox();
    QString getToolName(InstrumentType type);

    QGraphicsView* m_view;
    ProjectManager* m_project_manager;
    ContextPannel* m_context_pannel;
    PalettePannel* m_palette_pannel;
    LayersPannel* m_layers_pannel;

    TransformBox* m_transform_box = nullptr;
    QUndoStack* m_undo_stack;

    Figure* m_selected_figure = nullptr;
    InstrumentType m_current_tool = InstrumentType::POINTER;

    bool m_space_pressed = false, m_is_panning = false, m_is_drawing = false;
    QPoint m_last_pan_pos;
    QPointF m_draw_start_pos;
    Figure* m_temp_figure = nullptr;

    bool m_color_target_is_fill = true;
    FigureState m_state_before_preview;
    bool m_is_previewing = false;

    FigureState m_clipboard_state;
    bool m_has_clipboard = false;
};

#endif // WORKSPACECONTROLLER_H