#ifndef MANIPULATOR_H
#define MANIPULATOR_H

#include <QGraphicsObject>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

class TransformBox : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit TransformBox(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QRectF targetRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // ЭТОТ МЕТОД СПАСЕТ НАС ОТ БЛОКИРОВКИ МЫШИ
    QPainterPath shape() const override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    enum InteractionState { None, ResizeBottomRight, Rotate };
    InteractionState m_state = None;

    QRectF m_target_rect;
    QPointF m_click_pos;
    qreal m_start_scale;
    qreal m_start_rotation;

    QRectF bottomRightHandle() const;
    QRectF topRotateHandle() const;
};

#endif // MANIPULATOR_H