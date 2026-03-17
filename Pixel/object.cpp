#include "object.h"

Ellipse::Ellipse(const QPointF& center, qreal radius, QGraphicsItem* parent)
    : Shape(parent), m_center(center), m_radius(radius)
{
    setFillColor(Qt::red); // По умолчанию красный для теста
}

Ellipse::Ellipse(QGraphicsItem* parent)
    : Shape(parent), m_center(0,0), m_radius(0)
{
}

void Ellipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();

    // Включаем сглаживание
    painter->setRenderHint(QPainter::Antialiasing);

    // Рисуем сам эллипс
    painter->setPen(QPen(m_strokeColor, m_strokeWidth));
    if (m_filled) painter->setBrush(QBrush(m_fillColor));
    else painter->setBrush(Qt::NoBrush);

    painter->drawEllipse(m_center, m_radius, m_radius);

    // ЕСЛИ ОБЪЕКТ ВЫДЕЛЕН (Кликнули мышкой): Рисуем синюю пунктирную рамку выделения
    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }

    painter->restore();
}

QRectF Ellipse::boundingRect() const
{
    // Qt должен знать точные границы объекта, чтобы правильно его обновлять и выделять.
    // Учитываем толщину обводки, чтобы края не "обрезались".
    qreal pen_offset = m_strokeWidth / 2.0;
    return QRectF(m_center.x() - m_radius - pen_offset,
                  m_center.y() - m_radius - pen_offset,
                  (m_radius + pen_offset) * 2,
                  (m_radius + pen_offset) * 2);
}

QPainterPath Ellipse::shape() const
{
    // shape() определяет, по какой зоне можно кликнуть мышкой.
    // Если этого не сделать, выделяться будет по квадрату (boundingRect).
    QPainterPath path;
    path.addEllipse(m_center, m_radius, m_radius);
    return path;
}

void Ellipse::setCenter(const QPointF& center)
{
    prepareGeometryChange(); // Обязательный вызов перед изменением геометрии в Qt!
    m_center = center;
    update();
}

QPointF Ellipse::getCenter() const { return m_center; }

void Ellipse::setRadius(qreal radius)
{
    prepareGeometryChange(); // Обязательно сообщаем Qt, что размер изменился
    m_radius = radius;
    update();
}

qreal Ellipse::getRadius() const { return m_radius; }