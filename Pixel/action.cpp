#include "action.h"

// --- ADD COMMAND ---
AddObjectCommand::AddObjectCommand(QGraphicsItem* parentLayer, QGraphicsItem* object, QUndoCommand *parent)
    : QUndoCommand(parent), m_parent_layer(parentLayer), m_object(object)
{
    setText("Draw Figure");
}

AddObjectCommand::~AddObjectCommand() {
    if (!m_object->scene() && !m_object->parentItem()) delete m_object;
}

void AddObjectCommand::undo() {
    // Убираем со слоя
    m_object->setParentItem(nullptr);
    if (m_object->scene()) m_object->scene()->removeItem(m_object);
}

void AddObjectCommand::redo() {
    // Добавляем на слой
    m_object->setParentItem(m_parent_layer);
}

// --- DELETE COMMAND ---
DeleteObjectCommand::DeleteObjectCommand(QGraphicsItem* parentLayer, QGraphicsItem* object, QUndoCommand *parent)
    : QUndoCommand(parent), m_parent_layer(parentLayer), m_object(object)
{
    setText("Delete Figure");
}

DeleteObjectCommand::~DeleteObjectCommand() {
    if (!m_object->scene() && !m_object->parentItem()) delete m_object;
}

void DeleteObjectCommand::undo() {
    m_object->setParentItem(m_parent_layer); // Возвращаем
}

void DeleteObjectCommand::redo() {
    m_object->setParentItem(nullptr);
    if (m_object->scene()) m_object->scene()->removeItem(m_object); // Удаляем
}