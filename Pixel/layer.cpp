#include "layer.h"
#include <QDebug>

Layer::Layer(QGraphicsItem* parent) : QGraphicsObject(parent), m_visible(true), m_locked(false) {}

Layer::Layer(const QString& name, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_visible(true), m_locked(false), m_name(name) {}

Layer::~Layer() {
    // УДАЛЕНО: for (Object* i : m_objects) delete i;
    // Никакого ручного удаления! QGraphicsObject сам корректно удалит всех своих детей.
    // А QUndoStack сам очистит объекты, удаленные через Ctrl+Z.
}

void Layer::addObject(Object* object) {
    if (!object) return;
    object->setParentItem(this);
    // Привязываем статус мобильности к блокировке слоя
    object->setFlag(QGraphicsItem::ItemIsMovable, !m_locked);
}

void Layer::removeObject(Object* object) {
    if (object && object->parentItem() == this) {
        object->setParentItem(nullptr);
    }
}

void Layer::setVisible(bool visible) {
    m_visible = visible;
    QGraphicsObject::setVisible(visible);
}

void Layer::setLocked(const bool locked) {
    m_locked = locked;

    // Вместо нашего вектора используем встроенный в Qt список детей слоя
    for (QGraphicsItem* item : childItems()) {
        if (Object* obj = dynamic_cast<Object*>(item)) {
            obj->setFlag(QGraphicsItem::ItemIsMovable, !m_locked);
        }
    }
}

LayerInfo Layer::getInfo() const { return LayerInfo{m_name, m_visible, m_locked}; }

QRectF Layer::boundingRect() const { return QRectF(); }
void Layer::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}