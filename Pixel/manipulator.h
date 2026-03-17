#ifndef MANIPULATOR_H
#define MANIPULATOR_H

#include <QGraphicsObject>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QUndoStack>

class TransformBox : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit TransformBox(QGraphicsItem *parent, QUndoStack* undoStack);

    QRectF targetRect() const;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QUndoStack* m_undo_stack;

    enum InteractionState {
        None, Rotate,
        ResizeTL, ResizeTC, ResizeTR,
        ResizeML, ResizeMR,
        ResizeBL, ResizeBC, ResizeBR
    };
    InteractionState m_state = None;

    // Данные для безупречной математики
    QPointF m_start_pos;
    qreal m_start_rotation;
    QTransform m_start_transform;
    QRectF m_start_rect;

    QPointF m_fixed_local_point;
    QPointF m_fixed_scene_point;
    QTransform m_initial_scene_transform;

    QRectF handleRect(int xPos, int yPos) const;
    QRectF rotateHandle() const;
    QPointF getFixedPoint(InteractionState state, QRectF rect) const;
};

#endif // MANIPULATOR_H