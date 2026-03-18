#include "layer.h"
#include <QDebug>

Layer::Layer(QGraphicsItem* parent)
    : QGraphicsObject(parent),
    m_visible(true),
    m_locked(false) {}

Layer::Layer(const QString& name, QGraphicsItem* parent)
    : QGraphicsObject(parent),
    m_visible(true),
    m_locked(false),
    m_name(name) {}

Layer::~Layer() {
}

void Layer::addObject(Object* object) {
    if (!object) return;
    object->setParentItem(this);
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

    for (QGraphicsItem* item : childItems()) {
        if (Object* obj = dynamic_cast<Object*>(item)) {
            obj->setFlag(QGraphicsItem::ItemIsMovable, !m_locked);
        }
    }
}

LayerInfo Layer::getInfo() const {
    return LayerInfo{m_name, m_visible, m_locked};
}

QRectF Layer::boundingRect() const {
    return QRectF();
}

void Layer::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}

std::vector<Object*> Layer::getObjects() const {
    std::vector<Object*> res;
    for (QGraphicsItem* item : childItems()) {
        if (Object* obj = dynamic_cast<Object*>(item)) {
            res.push_back(obj);
        }
    }
    return res;
}