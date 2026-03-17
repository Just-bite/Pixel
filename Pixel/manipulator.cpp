#include "manipulator.h"
#include <QPen>
#include <QBrush>
#include <QLineF>

TransformBox::TransformBox(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setZValue(1000);
    setFlag(ItemIsSelectable, false);
}

QRectF TransformBox::targetRect() const {
    return parentItem() ? parentItem()->boundingRect() : QRectF();
}

QRectF TransformBox::boundingRect() const {
    return targetRect().adjusted(-20, -40, 20, 20);
}

QPainterPath TransformBox::shape() const {
    QPainterPath path;
    path.addRect(bottomRightHandle());
    path.addEllipse(topRotateHandle());
    return path;
}

QRectF TransformBox::bottomRightHandle() const {
    QRectF rect = targetRect();
    return QRectF(rect.right() - 5, rect.bottom() - 5, 10, 10);
}

QRectF TransformBox::topRotateHandle() const {
    QRectF rect = targetRect();
    return QRectF(rect.center().x() - 5, rect.top() - 30, 10, 10);
}

void TransformBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option); Q_UNUSED(widget);
    if (!parentItem()) return;

    QRectF rect = targetRect();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // ЯРКО-КРАСНАЯ РАМКА ДЛЯ ТЕСТА
    painter->setPen(QPen(Qt::red, 2, Qt::DashLine));
    painter->drawRect(rect);

    painter->drawLine(QPointF(rect.center().x(), rect.top()),
                      QPointF(rect.center().x(), rect.top() - 25));

    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(Qt::white);

    painter->drawRect(bottomRightHandle());
    painter->drawEllipse(topRotateHandle());

    painter->restore();
}

void TransformBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (bottomRightHandle().contains(event->pos())) {
        m_state = ResizeBottomRight;
        m_click_pos = event->scenePos();
        m_start_scale = parentItem()->scale();
        event->accept();
    }
    else if (topRotateHandle().contains(event->pos())) {
        m_state = Rotate;
        m_start_rotation = parentItem()->rotation();
        event->accept();
    }
    else {
        event->ignore();
    }
}

void TransformBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!parentItem()) return;

    if (m_state == ResizeBottomRight) {
        QPointF delta = event->scenePos() - m_click_pos;
        qreal scaleFactor = 1.0 + (delta.x() / targetRect().width());
        parentItem()->setScale(m_start_scale * scaleFactor);

        // Важно: заставляем рамку перерисоваться при масштабировании
        update();
    }
    else if (m_state == Rotate) {
        QPointF center = parentItem()->mapToScene(targetRect().center());
        QLineF startLine(center, event->lastScenePos());
        QLineF currentLine(center, event->scenePos());
        qreal angleDelta = startLine.angleTo(currentLine);
        parentItem()->setRotation(parentItem()->rotation() - angleDelta);
        update();
    }
    else {
        event->ignore();
    }
}

void TransformBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_state != None) m_state = None;
    else event->ignore();
}