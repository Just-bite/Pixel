#ifndef WORKSPACECONTEXT_H
#define WORKSPACECONTEXT_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QUndoStack>

class ProjectManager;
class ContextPannel;
class PalettePannel;
class LayersPannel;
class WorkspaceController;

struct WorkspaceContext {
    QGraphicsView* view = nullptr;
    QGraphicsScene* scene = nullptr;
    ProjectManager* projectManager = nullptr;
    ContextPannel* contextPannel = nullptr;
    PalettePannel* palettePannel = nullptr;
    LayersPannel* layersPannel = nullptr;
    QUndoStack* undoStack = nullptr;
    WorkspaceController* controller = nullptr;
};

#endif // WORKSPACECONTEXT_H