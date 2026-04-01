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
    explicit TransformBox(QGraphicsItem *target, QUndoStack* undoStack);

    QRectF targetRect() const;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    bool isInteracting() const { return m_state != None; }
    void setViewScale(qreal scale);
    void syncPosition();

signals:
    void interactionStarted();
    void interactionEnded();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QUndoStack* m_undo_stack;
    QGraphicsItem* m_target; // Теперь ссылаемся на цель, а не на parentItem

    enum InteractionState {
        None, Rotate,
        ResizeTL, ResizeTC, ResizeTR,
        ResizeML, ResizeMR,
        ResizeBL, ResizeBC, ResizeBR
    };
    InteractionState m_state = None;

    QPointF m_start_pos;
    qreal m_start_rotation;
    QRectF m_start_rect;
    QTransform m_initial_scene_transform;
    qreal m_view_scale = 1.0;

    QRectF handleRect(int xPos, int yPos) const;
    QRectF rotateHandle() const;
};

#endif // MANIPULATOR_H