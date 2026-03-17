#ifndef ACTION_H
#define ACTION_H

#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QRectF>

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

// ТРАНСФОРМАЦИЯ: Теперь сохраняет параметрическую геометрию (QRectF)
class TransformObjectCommand : public QUndoCommand {
public:
    TransformObjectCommand(QGraphicsItem* object,
                           QPointF oldPos, qreal oldRot, QRectF oldRect,
                           QPointF newPos, qreal newRot, QRectF newRect,
                           QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    QGraphicsItem* m_object;
    QPointF m_old_pos, m_new_pos;
    qreal m_old_rot, m_new_rot;
    QRectF m_old_rect, m_new_rect;
};

#endif // ACTION_H