#include "layer.h"

Layer::Layer(QObject* parent) : QObject(parent), m_visible(true), m_locked(false)
{
}

void Layer::addObject(Object* object)
{
    object->setParent(this);
    m_objects.push_back(object);
}

void Layer::draw(QPainter* painter) const
{
    if (!m_visible) return;

    for (const auto& obj : m_objects) {
        obj->draw(painter);
    }
}
