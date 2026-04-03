#ifndef CONCRETE_TOOLS_H
#define CONCRETE_TOOLS_H

#include "tool.h"
#include "object.h"
#include "manipulator.h"
#include <QPoint>

// --- ИНСТРУМЕНТ "УКАЗАТЕЛЬ" (POINTER) ---
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

// --- ИНСТРУМЕНТ "РУКА" (HAND) ---
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

// --- ИНСТРУМЕНТ "ФИГУРА" (FIGURE) ---
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

// --- ИНСТРУМЕНТ "ТЕКСТ" (TEXT) ---
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

// --- ИНСТРУМЕНТ "КАРАНДАШ" (PENCIL) ---
class PencilTool : public Tool {
    Q_OBJECT
public:
    explicit PencilTool(QObject* parent = nullptr) : Tool(parent) {}

    void onActivate(const WorkspaceContext& ctx) override;
    bool mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;
    bool mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) override;

private:
    bool m_is_drawing = false;
};

#endif // CONCRETE_TOOLS_H