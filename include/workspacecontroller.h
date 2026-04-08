#ifndef WORKSPACECONTROLLER_H
#define WORKSPACECONTROLLER_H

#include <QObject>
#include <QUndoStack>
#include <memory>
#include <unordered_map>

#include "workspacecontext.h"
#include "instrumentpannel.h"
#include "concrete_tools.h"

enum class RasterizeResult {
    Ready,          // Слой готов, можно сразу рисовать
    RasterizedNow,  // Слой растрирован после диалога, рисование нужно прервать (чтобы не было рывка)
    Cancelled       // Пользователь отменил растрирование
};

class WorkspaceController : public QObject {
    Q_OBJECT
public:
    explicit WorkspaceController(const WorkspaceContext& ctx, QObject* parent = nullptr);
    QUndoStack* getUndoStack() const { return m_undo_stack; }
    RasterizeResult prepareRasterLayer();

public slots:
    void setCurrentTool(InstrumentType type);
    void onSelectionChanged();
    void onContextPropertyChanged();
    void onColorTargetChanged(bool isFill);
    void onColorPickedPreview(const QColor& color);
    void onColorPickedCommit(const QColor& color);
    void onMoveObjectLayerRequested(int shift);
    void onActiveLayerChanged(int id);
    void updateTransformBoxScale();
    void clearState();

signals:
    void viewportChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void handleGlobalShortcuts(QKeyEvent* kEvent);
    void handlePaste();
    void handleDrop(QDropEvent* event);

    WorkspaceContext m_context;
    QUndoStack* m_undo_stack;

    std::unordered_map<InstrumentType, std::unique_ptr<Tool>> m_tools;
    Tool* m_active_tool = nullptr;
    InstrumentType m_current_tool_type = InstrumentType::POINTER;
    InstrumentType m_previous_tool_type = InstrumentType::POINTER;

    // Глобальный буфер обмена
    enum class ClipboardType { None, Figure, Text, Image };
    ClipboardType m_clipboard_type = ClipboardType::None;
    FigureState m_clipboard_figure;
    TextState m_clipboard_text;
    ImageState m_clipboard_image;

    bool m_color_target_is_fill = true;
    bool m_is_previewing = false;
    FigureState m_state_before_preview;
    TextState m_text_state_before_preview;
};

#endif // WORKSPACECONTROLLER_H