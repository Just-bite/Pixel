#include "action.h"

AddObjectCommand::AddObjectCommand(QGraphicsItem* pLayer, QGraphicsItem* obj, QUndoCommand *p) : QUndoCommand(p), m_parent_layer(pLayer), m_object(obj) { setText("Add Figure"); }
AddObjectCommand::~AddObjectCommand() { if (!m_object->scene() && !m_object->parentItem()) delete m_object; }
void AddObjectCommand::undo() { m_object->setParentItem(nullptr); if (m_object->scene()) m_object->scene()->removeItem(m_object); }
void AddObjectCommand::redo() { m_object->setParentItem(m_parent_layer); }

DeleteObjectCommand::DeleteObjectCommand(QGraphicsItem* pLayer, QGraphicsItem* obj, QUndoCommand *p) : QUndoCommand(p), m_parent_layer(pLayer), m_object(obj) { setText("Delete Figure"); }
DeleteObjectCommand::~DeleteObjectCommand() { if (!m_object->scene() && !m_object->parentItem()) delete m_object; }
void DeleteObjectCommand::undo() { m_object->setParentItem(m_parent_layer); }
void DeleteObjectCommand::redo() { m_object->setParentItem(nullptr); if (m_object->scene()) m_object->scene()->removeItem(m_object); }

ModifyFigureCommand::ModifyFigureCommand(Figure* fig, const FigureState& oState, const FigureState& nState, QUndoCommand *p) : QUndoCommand(p), m_figure(fig), m_old_state(oState), m_new_state(nState) { setText("Modify Figure"); }
void ModifyFigureCommand::undo() { if (m_figure) m_figure->setState(m_old_state); }
void ModifyFigureCommand::redo() { if (m_figure) m_figure->setState(m_new_state); }

MoveObjectLayerCommand::MoveObjectLayerCommand(Canvas* canvas, Object* obj, int oldLayerId, int newLayerId, QUndoCommand *p) : QUndoCommand(p), m_canvas(canvas), m_object(obj), m_old_id(oldLayerId), m_new_id(newLayerId) { setText("Move Object Layer"); }
void MoveObjectLayerCommand::undo() { if (m_canvas && m_object) m_canvas->moveObjectToLayer(m_object, m_old_id); }
void MoveObjectLayerCommand::redo() { if (m_canvas && m_object) m_canvas->moveObjectToLayer(m_object, m_new_id); }