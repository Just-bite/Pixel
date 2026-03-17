#include "layer.h"
#include <QDebug>

Layer::Layer(QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_visible(true)
    , m_locked(false)
{
}

Layer::Layer(const QString& name, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_visible(true)
    , m_locked(false)
    , m_name(name)
{
}

Layer::~Layer()
{
    qDebug() << "deleting layer";
    for (Object* i : m_objects)
        delete i; // При удалении слоя удаляем и объекты
}

void Layer::addObject(Object* object)
{
    m_objects.push_back(object);
    // Магия Qt: привязываем графический объект к этому слою-контейнеру
    object->setParentItem(this);
}

void Layer::setVisible(bool visible)
{
    m_visible = visible;
    QGraphicsObject::setVisible(visible); // Встроенная функция Qt скроет все дочерние объекты!
}

LayerInfo Layer::getInfo() const
{
    return LayerInfo{m_name, m_visible, m_locked};
}

// Слой сам по себе невидимый, поэтому методы пустые
QRectF Layer::boundingRect() const { return QRectF(); }
void Layer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) { Q_UNUSED(painter); Q_UNUSED(option); Q_UNUSED(widget); }