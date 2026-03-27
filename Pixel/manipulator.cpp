#include "manipulator.h"
#include "action.h"
#include "object.h"
#include <QPen>
#include <QBrush>
#include <QLineF>
#include <qmath.h>

TransformBox::TransformBox(QGraphicsItem *parent, QUndoStack* undoStack)
    : QGraphicsObject(parent), m_undo_stack(undoStack)
{
    setZValue(1000);
    setFlag(ItemIsSelectable, false);
}

void TransformBox::setViewScale(qreal scale) {
    prepareGeometryChange();
    m_view_scale = scale > 0.0 ? scale : 1.0;
    update();
}

QRectF TransformBox::targetRect() const {
    Object* obj = dynamic_cast<Object*>(parentItem());
    if (obj) return obj->getLocalRect();
    return parentItem() ? parentItem()->boundingRect() : QRectF();
}

void getScaleFromMatrix(const QTransform& t, qreal& sx, qreal& sy) {
    sx = qSqrt(t.m11()*t.m11() + t.m12()*t.m12());
    sy = qSqrt(t.m21()*t.m21() + t.m22()*t.m22());
    if (sx == 0) sx = 1; if (sy == 0) sy = 1;
}

QRectF TransformBox::boundingRect() const {
    qreal sx, sy; getScaleFromMatrix(parentItem()->sceneTransform(), sx, sy);
    qreal actualSx = sx * m_view_scale;
    qreal actualSy = sy * m_view_scale;

    // Рассчитываем динамический отступ, чтобы ручки не обрезались при сильном отдалении
    qreal padX = 100.0 / actualSx;
    qreal padY = 100.0 / actualSy;
    return targetRect().adjusted(-padX, -padY, padX, padY);
}

QRectF TransformBox::handleRect(int xPos, int yPos) const {
    QRectF rect = targetRect();
    qreal x = rect.center().x(); qreal y = rect.center().y();
    if (xPos == -1) x = rect.left(); else if (xPos == 1) x = rect.right();
    if (yPos == -1) y = rect.top(); else if (yPos == 1) y = rect.bottom();

    qreal sx, sy; getScaleFromMatrix(parentItem()->sceneTransform(), sx, sy);
    qreal actualSx = sx * m_view_scale;
    qreal actualSy = sy * m_view_scale;
    return QRectF(x - (5.0 / actualSx), y - (5.0 / actualSy), 10.0 / actualSx, 10.0 / actualSy);
}

QRectF TransformBox::rotateHandle() const {
    QRectF rect = targetRect();
    qreal sx, sy; getScaleFromMatrix(parentItem()->sceneTransform(), sx, sy);
    qreal actualSx = sx * m_view_scale;
    qreal actualSy = sy * m_view_scale;
    return QRectF(rect.center().x() - (5.0 / actualSx), rect.top() - (25.0 / actualSy) - (5.0 / actualSy), 10.0 / actualSx, 10.0 / actualSy);
}

QPainterPath TransformBox::shape() const {
    QPainterPath path;
    path.addEllipse(rotateHandle());
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue;
            path.addRect(handleRect(i, j));
        }
    }
    return path;
}

void TransformBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option); Q_UNUSED(widget);
    if (!parentItem()) return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Рисуем рамку и соединительную линию. Толщина = 0 означает толщину 1px визуально
    painter->setPen(QPen(Qt::blue, 0, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(targetRect());
    painter->drawLine(QPointF(targetRect().center().x(), targetRect().top()), rotateHandle().center());

    painter->setPen(QPen(Qt::black, 0, Qt::SolidLine));
    painter->setBrush(Qt::white);
    painter->drawEllipse(rotateHandle());
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue;
            painter->drawRect(handleRect(i, j));
        }
    }
    painter->restore();
}

void TransformBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!parentItem()) return;

    m_start_pos = parentItem()->pos();
    m_start_rotation = parentItem()->rotation();
    m_start_rect = targetRect();
    m_initial_scene_transform = parentItem()->sceneTransform();

    QPointF p = event->pos();

    if (rotateHandle().contains(p)) m_state = Rotate;
    else if (handleRect(-1, -1).contains(p)) m_state = ResizeTL;
    else if (handleRect(0, -1).contains(p)) m_state = ResizeTC;
    else if (handleRect(1, -1).contains(p)) m_state = ResizeTR;
    else if (handleRect(-1, 0).contains(p)) m_state = ResizeML;
    else if (handleRect(1, 0).contains(p)) m_state = ResizeMR;
    else if (handleRect(-1, 1).contains(p)) m_state = ResizeBL;
    else if (handleRect(0, 1).contains(p)) m_state = ResizeBC;
    else if (handleRect(1, 1).contains(p)) m_state = ResizeBR;
    else { m_state = None; event->ignore(); return; }

    event->accept();
}

void TransformBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!parentItem() || m_state == None) { event->ignore(); return; }

    if (m_state == Rotate) {
        QPointF center = parentItem()->mapToScene(m_start_rect.center());
        QLineF startLine(center, m_initial_scene_transform.map(rotateHandle().center()));
        QLineF currentLine(center, event->scenePos());
        parentItem()->setRotation(m_start_rotation - startLine.angleTo(currentLine));
    }
    else {
        QPointF localMouse = m_initial_scene_transform.inverted().map(event->scenePos());

        qreal left = m_start_rect.left();
        qreal right = m_start_rect.right();
        qreal top = m_start_rect.top();
        qreal bottom = m_start_rect.bottom();

        if (m_state == ResizeTL) { left = localMouse.x(); top = localMouse.y(); }
        else if (m_state == ResizeTC) { top = localMouse.y(); }
        else if (m_state == ResizeTR) { right = localMouse.x(); top = localMouse.y(); }
        else if (m_state == ResizeML) { left = localMouse.x(); }
        else if (m_state == ResizeMR) { right = localMouse.x(); }
        else if (m_state == ResizeBL) { left = localMouse.x(); bottom = localMouse.y(); }
        else if (m_state == ResizeBC) { bottom = localMouse.y(); }
        else if (m_state == ResizeBR) { right = localMouse.x(); bottom = localMouse.y(); }

        QRectF rawNewRect(QPointF(left, top), QPointF(right, bottom));
        rawNewRect = rawNewRect.normalized();

        if (rawNewRect.width() < 1.0) rawNewRect.setWidth(1.0);
        if (rawNewRect.height() < 1.0) rawNewRect.setHeight(1.0);

        QPointF center = rawNewRect.center();
        QRectF centeredRect = rawNewRect.translated(-center);
        QPointF newPos = m_initial_scene_transform.map(center);

        Object* obj = dynamic_cast<Object*>(parentItem());
        if (obj) {
            prepareGeometryChange();

            obj->setLocalRect(centeredRect);
            obj->setPos(newPos);
        }
    }
}

void TransformBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_state != None) {
        Figure* fig = dynamic_cast<Figure*>(parentItem());
        if (fig) {
            FigureState startState = fig->getState();
            startState.pos = m_start_pos;
            startState.rot = m_start_rotation;
            startState.rect = m_start_rect;

            m_undo_stack->push(new ModifyFigureCommand(fig, startState, fig->getState()));
        }
        m_state = None;
    } else {
        event->ignore();
    }
}