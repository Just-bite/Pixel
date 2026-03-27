#ifndef ACTION_H
#define ACTION_H

#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include "object.h"
#include "canvas.h"

class AddObjectCommand : public QUndoCommand {
public:
    AddObjectCommand(QGraphicsItem* parentLayer, QGraphicsItem* object, QUndoCommand *parent = nullptr);
    ~AddObjectCommand();
    void undo() override;
    void redo() override;
private:
    QGraphicsItem* m_parent_layer;
    QGraphicsItem* m_object;
};

class DeleteObjectCommand : public QUndoCommand {
public:
    DeleteObjectCommand(QGraphicsItem* parentLayer, QGraphicsItem* object, QUndoCommand *parent = nullptr);
    ~DeleteObjectCommand();
    void undo() override;
    void redo() override;
private:
    QGraphicsItem* m_parent_layer;
    QGraphicsItem* m_object;
};

class ModifyFigureCommand : public QUndoCommand {
public:
    ModifyFigureCommand(Figure* figure, const FigureState& oldState, const FigureState& newState, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Figure* m_figure;
    FigureState m_old_state;
    FigureState m_new_state;
};

class MoveObjectLayerCommand : public QUndoCommand {
public:
    MoveObjectLayerCommand(Canvas* canvas, Object* obj, int oldLayerId, int newLayerId, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Canvas* m_canvas;
    Object* m_object;
    int m_old_id;
    int m_new_id;
};

class ModifyTextCommand : public QUndoCommand {
public:
    ModifyTextCommand(TextObject* textObj, const TextState& oldState, const TextState& newState, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    TextObject* m_text;
    TextState m_old_state;
    TextState m_new_state;
};

#endif // ACTION_H
