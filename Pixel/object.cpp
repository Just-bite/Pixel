#include "object.h"

void Object::setPosition(const QPointF& pos)
{
    m_position = pos;
}
QPointF Object::position() const { return m_position; }

Ellipse::Ellipse(const QPointF& center, qreal radius, QObject* parent)
    : Shape(parent), m_center(center), m_radius(radius)
{
}

void Ellipse::draw(QPainter* painter) const
{
    painter->save();

    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(Qt::red));

    painter->drawEllipse(m_center, m_radius, m_radius);

    painter->restore();
}

bool Ellipse::contains(const QPointF& point) const { return false; /* TODO */ }
QRectF Ellipse::boundingRect() const { return QRectF(); /* TODO */ }
