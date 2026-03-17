#include "action.h"

AddObjectCommand::AddObjectCommand(QGraphicsItem* parentLayer, QGraphicsItem* object, QUndoCommand *parent)
    : QUndoCommand(parent), m_parent_layer(parentLayer), m_object(object) { setText("Draw Figure"); }

AddObjectCommand::~AddObjectCommand() { if (!m_object->scene() && !m_object->parentItem()) delete m_object; }

void AddObjectCommand::undo() { m_object->setParentItem(nullptr); if (m_object->scene()) m_object->scene()->removeItem(m_object); }

void AddObjectCommand::redo() { m_object->setParentItem(m_parent_layer); }

DeleteObjectCommand::DeleteObjectCommand(QGraphicsItem* parentLayer, QGraphicsItem* object, QUndoCommand *parent)
    : QUndoCommand(parent), m_parent_layer(parentLayer), m_object(object) { setText("Delete Figure"); }

DeleteObjectCommand::~DeleteObjectCommand() { if (!m_object->scene() && !m_object->parentItem()) delete m_object; }

void DeleteObjectCommand::undo() { m_object->setParentItem(m_parent_layer); }

void DeleteObjectCommand::redo() { m_object->setParentItem(nullptr); if (m_object->scene()) m_object->scene()->removeItem(m_object); }

// --- ОБНОВЛЕННАЯ КОМАНДА ТРАНСФОРМАЦИИ ---
TransformObjectCommand::TransformObjectCommand(QGraphicsItem* object,
                                               QPointF oldPos, qreal oldRot, QTransform oldTransform,
                                               QPointF newPos, qreal newRot, QTransform newTransform,
                                               QUndoCommand *parent)
    : QUndoCommand(parent), m_object(object),
    m_old_pos(oldPos), m_new_pos(newPos),
    m_old_rot(oldRot), m_new_rot(newRot),
    m_old_transform(oldTransform), m_new_transform(newTransform)
{
    setText("Transform Figure");
}
void TransformObjectCommand::undo() {
    m_object->setPos(m_old_pos);
    m_object->setRotation(m_old_rot);
    m_object->setTransform(m_old_transform);
}
void TransformObjectCommand::redo() {
    m_object->setPos(m_new_pos);
    m_object->setRotation(m_new_rot);
    m_object->setTransform(m_new_transform);
}