#include "include/layer.h"

Layer::Layer(QGraphicsItem* parent)
    : QGraphicsObject(parent), m_visible(true), m_locked(false), m_is_rasterized(false) {}

Layer::Layer(const QString& name, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_visible(true), m_locked(false), m_is_rasterized(false), m_name(name) {}

Layer::~Layer() {}

void Layer::addObject(Object* object) {
    if (!object) return;
    object->setParentItem(this);
    object->setFlag(QGraphicsItem::ItemIsMovable, !m_locked);
    object->setFlag(QGraphicsItem::ItemIsSelectable, !m_locked);
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
            obj->setFlag(QGraphicsItem::ItemIsSelectable, !m_locked);
        }
    }
}

void Layer::setRasterized(bool rasterized) {
    prepareGeometryChange();
    m_is_rasterized = rasterized;
    update();
}

void Layer::setRasterImage(const QImage& img) {
    prepareGeometryChange();
    m_raster_image = img;
    update();
}

LayerInfo Layer::getInfo() const {
    return LayerInfo{m_name, m_visible, m_locked, isFilter(), m_is_rasterized};
}

QRectF Layer::boundingRect() const {
    if (m_is_rasterized && !m_raster_image.isNull()) {
        return QRectF(0, 0, m_raster_image.width(), m_raster_image.height());
    }
    return QRectF();
}

void Layer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (m_is_rasterized && !m_raster_image.isNull()) {
        painter->drawImage(0, 0, m_raster_image);
    }
}

std::vector<Object*> Layer::getObjects() const {
    std::vector<Object*> res;
    for (QGraphicsItem* item : childItems()) {
        if (Object* obj = dynamic_cast<Object*>(item)) {
            res.push_back(obj);
        }
    }
    return res;
}