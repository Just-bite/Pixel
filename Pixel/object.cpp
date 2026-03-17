#include "object.h"

Ellipse::Ellipse(const QRectF& rect, QGraphicsItem* parent)
    : Shape(parent), m_rect(rect)
{
    setFillColor(Qt::red);
}

Ellipse::Ellipse(QGraphicsItem* parent)
    : Shape(parent), m_rect(0,0,0,0)
{
    setFillColor(Qt::red);
}

void Ellipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(QPen(m_strokeColor, m_strokeWidth));
    if (m_filled) painter->setBrush(QBrush(m_fillColor));
    else painter->setBrush(Qt::NoBrush);

    // Рисуем эллипс, вписанный в прямоугольник!
    painter->drawEllipse(m_rect);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
    painter->restore();
}

QRectF Ellipse::boundingRect() const
{
    qreal pen_offset = m_strokeWidth / 2.0;
    // Границы - это наш прямоугольник + толщина линии
    return m_rect.adjusted(-pen_offset, -pen_offset, pen_offset, pen_offset);
}

QPainterPath Ellipse::shape() const
{
    QPainterPath path;
    path.addEllipse(m_rect);
    return path;
}

void Ellipse::setRect(const QRectF& rect)
{
    prepareGeometryChange();
    m_rect = rect;
    update();
}

QRectF Ellipse::getRect() const { return m_rect; }