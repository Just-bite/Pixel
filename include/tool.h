#ifndef TOOL_H
#define TOOL_H

#include <QObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include "workspacecontext.h"

class Tool : public QObject {
    Q_OBJECT
public:
    explicit Tool(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Tool() = default;

    // Жизненный цикл инструмента
    virtual void onActivate(const WorkspaceContext& ctx) {}
    virtual void onDeactivate(const WorkspaceContext& ctx) {}

    // События мыши и клавиатуры
    virtual bool mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) { return false; }
    virtual bool mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) { return false; }
    virtual bool mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) { return false; }
    virtual bool mouseDoubleClickEvent(QMouseEvent* event, const WorkspaceContext& ctx) { return false; }

    virtual bool keyPressEvent(QKeyEvent* event, const WorkspaceContext& ctx) { return false; }
    virtual bool keyReleaseEvent(QKeyEvent* event, const WorkspaceContext& ctx) { return false; }

    // События от других частей системы
    virtual void onSelectionChanged(const WorkspaceContext& ctx) {}
    virtual void onViewScaleChanged(const WorkspaceContext& ctx) {}
    virtual void onObjectModified(const WorkspaceContext& ctx) {}
};

#endif // TOOL_H