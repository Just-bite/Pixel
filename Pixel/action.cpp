#include "action.h"

AddObjectCommand::AddObjectCommand(QGraphicsItem* parentLayer, QGraphicsItem* object, QUndoCommand *parent)
    : QUndoCommand(parent), m_parent_layer(parentLayer), m_object(object) { setText("Add Figure"); }

AddObjectCommand::~AddObjectCommand() { if (!m_object->scene() && !m_object->parentItem()) delete m_object; }

void AddObjectCommand::undo() { m_object->setParentItem(nullptr); if (m_object->scene()) m_object->scene()->removeItem(m_object); }

void AddObjectCommand::redo() { m_object->setParentItem(m_parent_layer); }

DeleteObjectCommand::DeleteObjectCommand(QGraphicsItem* parentLayer, QGraphicsItem* object, QUndoCommand *parent)
    : QUndoCommand(parent), m_parent_layer(parentLayer), m_object(object) { setText("Delete Figure"); }

DeleteObjectCommand::~DeleteObjectCommand() { if (!m_object->scene() && !m_object->parentItem()) delete m_object; }

void DeleteObjectCommand::undo() { m_object->setParentItem(m_parent_layer); }

void DeleteObjectCommand::redo() { m_object->setParentItem(nullptr); if (m_object->scene()) m_object->scene()->removeItem(m_object); }


ModifyFigureCommand::ModifyFigureCommand(Figure* figure, const FigureState& oldState, const FigureState& newState, QUndoCommand *parent)
    : QUndoCommand(parent), m_figure(figure), m_old_state(oldState), m_new_state(newState)
{
    setText("Modify Figure");
}

void ModifyFigureCommand::undo() { if (m_figure) m_figure->setState(m_old_state); }

void ModifyFigureCommand::redo() { if (m_figure) m_figure->setState(m_new_state); }