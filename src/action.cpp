#include "action.h"

AddObjectCommand::AddObjectCommand(QGraphicsItem* pLayer, QGraphicsItem* obj, QUndoCommand *p)
    : QUndoCommand(p), m_parent_layer(pLayer), m_object(obj) { setText("Add Figure"); }

AddObjectCommand::~AddObjectCommand() {
    if (m_object && !m_object->scene() && !m_object->parentItem()) {
        delete m_object;
    }
}

void AddObjectCommand::undo() {
    if (!m_object) return;
    if (m_object->scene()) {
        m_object->scene()->removeItem(m_object);
    }
    m_object->setParentItem(nullptr);
}

void AddObjectCommand::redo() {
    if (!m_object) return;
    m_object->setParentItem(m_parent_layer);
}

DeleteObjectCommand::DeleteObjectCommand(QGraphicsItem* pLayer, QGraphicsItem* obj, QUndoCommand *p)
    : QUndoCommand(p), m_parent_layer(pLayer), m_object(obj) { setText("Delete Figure"); }

DeleteObjectCommand::~DeleteObjectCommand() {
    if (m_object && !m_object->scene() && !m_object->parentItem()) {
        delete m_object;
    }
}

void DeleteObjectCommand::undo() {
    if (!m_object) return;
    m_object->setParentItem(m_parent_layer);
}

void DeleteObjectCommand::redo() {
    if (!m_object) return;
    if (m_object->scene()) {
        m_object->scene()->removeItem(m_object);
    }
    m_object->setParentItem(nullptr);
    if (QGraphicsScene* s = m_object->scene())
        s->removeItem(m_object);
}

ModifyFigureCommand::ModifyFigureCommand(Figure* fig, const FigureState& oState, const FigureState& nState, QUndoCommand *p)
    : QUndoCommand(p),
    m_figure(fig),
    m_old_state(oState),
    m_new_state(nState) {
    setText("Modify Figure");
}
void ModifyFigureCommand::undo() {
    if (m_figure) m_figure->setState(m_old_state);
}
void ModifyFigureCommand::redo() {
    if (m_figure) m_figure->setState(m_new_state);
}

MoveObjectLayerCommand::MoveObjectLayerCommand(Canvas* canvas, Object* obj, int oldLayerId, int newLayerId, QUndoCommand *p)
    : QUndoCommand(p),
    m_canvas(canvas),
    m_object(obj),
    m_old_id(oldLayerId),
    m_new_id(newLayerId) {
    setText("Move Object Layer");
}
void MoveObjectLayerCommand::undo() {
    if (m_canvas && m_object)
        m_canvas->moveObjectToLayer(m_object, m_old_id);
}
void MoveObjectLayerCommand::redo() {
    if (m_canvas && m_object)
        m_canvas->moveObjectToLayer(m_object, m_new_id);
}

ModifyTextCommand::ModifyTextCommand(TextObject* txt, const TextState& oState, const TextState& nState, QUndoCommand *p)
    : QUndoCommand(p), m_text(txt), m_old_state(oState), m_new_state(nState) { setText("Modify Text"); }
void ModifyTextCommand::undo() { if (m_text) m_text->setState(m_old_state); }
void ModifyTextCommand::redo() { if (m_text) m_text->setState(m_new_state); }


ModifyImageCommand::ModifyImageCommand(ImageObject* img, const ImageState& oState, const ImageState& nState, QUndoCommand *p)
    : QUndoCommand(p), m_image(img), m_old_state(oState), m_new_state(nState) { setText("Modify Image"); }
void ModifyImageCommand::undo() { if (m_image) m_image->setState(m_old_state); }
void ModifyImageCommand::redo() { if (m_image) m_image->setState(m_new_state); }

ModifyFilterCommand::ModifyFilterCommand(FilterLayer* filter, const FilterState& oState, const FilterState& nState, QUndoCommand *p)
    : QUndoCommand(p), m_filter(filter), m_old_state(oState), m_new_state(nState) { setText("Modify Filter"); }
void ModifyFilterCommand::undo() { if (m_filter) m_filter->setFilterState(m_old_state); }
void ModifyFilterCommand::redo() { if (m_filter) m_filter->setFilterState(m_new_state); }