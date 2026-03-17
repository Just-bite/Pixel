#ifndef ACTION_H
#define ACTION_H

#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTransform>

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

// ТРАНСФОРМАЦИЯ: Теперь хранит полноценную матрицу (QTransform) вместо одного qreal
class TransformObjectCommand : public QUndoCommand {
public:
    TransformObjectCommand(QGraphicsItem* object,
                           QPointF oldPos, qreal oldRot, QTransform oldTransform,
                           QPointF newPos, qreal newRot, QTransform newTransform,
                           QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    QGraphicsItem* m_object;
    QPointF m_old_pos, m_new_pos;
    qreal m_old_rot, m_new_rot;
    QTransform m_old_transform, m_new_transform; // Матрицы вместо чисел!
};

#endif // ACTION_H