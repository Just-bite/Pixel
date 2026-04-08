#ifndef CONCRETE_TOOLS_H
#define CONCRETE_TOOLS_H

#include "tool.h"
#include "object.h"
#include "manipulator.h"
#include <QPoint>
#include "layer.h"

class PointerTool : public Tool {
    Q_OBJECT
public:
    explicit PointerTool(QObject* parent = nullptr);

    void onActivate(const WorkspaceContext& ctx) override;
    void onDeactivate(const WorkspaceContext& ctx) override;

    bool mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseDoubleClickEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool keyPressEvent(QKeyEvent* event, const WorkspaceContext& ctx) override;

    void onSelectionChanged(const WorkspaceContext& ctx) override;
    void onViewScaleChanged(const WorkspaceContext& ctx) override;
    void onObjectModified(const WorkspaceContext& ctx) override;

private:
    void clearTransformBox(const WorkspaceContext& ctx);
    void updateContextPanel(const WorkspaceContext& ctx);

    TransformBox* m_transform_box = nullptr;
    bool m_updating_selection = false;

    Figure* m_drag_target = nullptr;
    FigureState m_drag_start_state;

    TextObject* m_drag_target_text = nullptr;
    TextState m_drag_start_text_state;

    ImageObject* m_drag_target_image = nullptr;
    ImageState m_drag_start_image_state;
};


class HandTool : public Tool {
    Q_OBJECT
public:
    explicit HandTool(QObject* parent = nullptr) : Tool(parent) {}

    void onActivate(const WorkspaceContext& ctx) override;
    void onDeactivate(const WorkspaceContext& ctx) override;

    bool mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;

private:
    bool m_is_panning = false;
    QPoint m_last_pan_pos;
};


class FigureTool : public Tool {
    Q_OBJECT
public:
    explicit FigureTool(QObject* parent = nullptr) : Tool(parent) {}

    void onActivate(const WorkspaceContext& ctx) override;
    bool mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;

private:
    bool m_is_drawing = false;
    QPointF m_draw_start_pos;
    Figure* m_temp_figure = nullptr;
};


class TextTool : public Tool {
    Q_OBJECT
public:
    explicit TextTool(QObject* parent = nullptr) : Tool(parent) {}

    void onActivate(const WorkspaceContext& ctx) override;
    bool mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;

private:
    bool m_is_drawing = false;
    QPointF m_draw_start_pos;
    TextObject* m_temp_text = nullptr;
};


class PencilTool : public Tool {
    Q_OBJECT
public:
    explicit PencilTool(QObject* parent = nullptr) : Tool(parent) {}

    void onActivate(const WorkspaceContext& ctx) override;
    bool mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;

    void onRasterSettingsChanged(const WorkspaceContext& ctx) override;

private:
    void drawStroke(QPainter& p, const QPoint& p1, const QPoint& p2, int radius, int density, const QColor& color);

    bool m_is_drawing = false;
    Layer* m_active_layer = nullptr;
    QPoint m_last_pos;
    QImage m_image_before_stroke;
    int m_radius = 10;
    int m_density = 100;
    int m_hardness = 100;
    QRect m_dirty_rect;
    QColor m_current_stroke_color = Qt::black; // Кэш цвета для текущего мазка
};


class EraserTool : public Tool {
    Q_OBJECT
public:
    explicit EraserTool(QObject* parent = nullptr) : Tool(parent) {}

    void onActivate(const WorkspaceContext& ctx) override;
    bool mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;

    void onRasterSettingsChanged(const WorkspaceContext& ctx) override;

private:
    void drawStroke(QPainter& p, const QPoint& p1, const QPoint& p2, int radius, int density);

    bool m_is_drawing = false;
    Layer* m_active_layer = nullptr;
    QPoint m_last_pos;
    QImage m_image_before_stroke;
    int m_radius = 20;
    int m_density = 100;
    int m_hardness = 100;
    QRect m_dirty_rect;
};


class FillTool : public Tool {
    Q_OBJECT
public:
    explicit FillTool(QObject* parent = nullptr) : Tool(parent) {}

    void onActivate(const WorkspaceContext& ctx) override;
    bool mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
};

class PipetteTool : public Tool {
    Q_OBJECT
public:
    explicit PipetteTool(QObject* parent = nullptr) : Tool(parent) {}

    void onActivate(const WorkspaceContext& ctx) override;
    bool mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;

private:
    void pickColor(const QPoint& mousePos, const WorkspaceContext& ctx, bool commit);
    bool m_is_picking = false;
};

#endif // CONCRETE_TOOLS_H