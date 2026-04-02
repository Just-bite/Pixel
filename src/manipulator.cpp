#include "include\manipulator.h"
#include "include\action.h"
#include "include\object.h"
#include <QPen>
#include <QBrush>
#include <QLineF>
#include <qmath.h>

TransformBox::TransformBox(QGraphicsItem *target, QUndoStack* undoStack)
    : QGraphicsObject(nullptr), m_undo_stack(undoStack), m_target(target)
{
    setZValue(10000);
    setFlag(ItemIsSelectable, false);
    syncPosition();
}

void TransformBox::syncPosition() {
    if (m_target) {
        // Обязательно предупреждаем графический движок, что рамка могла поменять размер или позицию
        prepareGeometryChange();
        setPos(m_target->scenePos());
        setRotation(m_target->rotation());
        update(); // Принудительная перерисовка
    }
}

void TransformBox::setViewScale(qreal scale) {
    prepareGeometryChange();
    m_view_scale = scale > 0.0 ? scale : 1.0;
    update();
}

QRectF TransformBox::targetRect() const {
    Object* obj = dynamic_cast<Object*>(m_target);
    if (obj) return obj->getLocalRect();
    return m_target ? m_target->boundingRect() : QRectF();
}

void getScaleFromMatrix(const QTransform& t, qreal& sx, qreal& sy) {
    sx = qSqrt(t.m11()*t.m11() + t.m12()*t.m12());
    sy = qSqrt(t.m21()*t.m21() + t.m22()*t.m22());
    if (sx == 0) sx = 1; if (sy == 0) sy = 1;
}

QRectF TransformBox::boundingRect() const {
    if (!m_target) return QRectF();
    qreal sx, sy; getScaleFromMatrix(m_target->sceneTransform(), sx, sy);
    qreal actualSx = sx * m_view_scale;
    qreal actualSy = sy * m_view_scale;

    qreal padX = 100.0 / actualSx;
    qreal padY = 100.0 / actualSy;
    return targetRect().adjusted(-padX, -padY, padX, padY);
}

QRectF TransformBox::handleRect(int xPos, int yPos) const {
    QRectF rect = targetRect();
    qreal x = rect.center().x(); qreal y = rect.center().y();
    if (xPos == -1) x = rect.left(); else if (xPos == 1) x = rect.right();
    if (yPos == -1) y = rect.top(); else if (yPos == 1) y = rect.bottom();

    qreal sx, sy; getScaleFromMatrix(m_target->sceneTransform(), sx, sy);
    qreal actualSx = sx * m_view_scale;
    qreal actualSy = sy * m_view_scale;
    return QRectF(x - (5.0 / actualSx), y - (5.0 / actualSy), 10.0 / actualSx, 10.0 / actualSy);
}

QRectF TransformBox::rotateHandle() const {
    QRectF rect = targetRect();
    qreal sx, sy; getScaleFromMatrix(m_target->sceneTransform(), sx, sy);
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
    if (!m_target) return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

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
    if (!m_target) return;

    m_start_pos = m_target->pos();
    m_start_rotation = m_target->rotation();
    m_start_rect = targetRect();
    m_initial_scene_transform = m_target->sceneTransform();

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
    emit interactionStarted();
}

void TransformBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_target || m_state == None) { event->ignore(); return; }

    if (m_state == Rotate) {
        QPointF center = m_target->mapToScene(m_start_rect.center());
        QLineF startLine(center, m_initial_scene_transform.map(rotateHandle().center()));
        QLineF currentLine(center, event->scenePos());

        qreal newRot = m_start_rotation - startLine.angleTo(currentLine);
        while (newRot < -360.0) newRot += 360.0;
        while (newRot > 360.0) newRot -= 360.0;

        m_target->setRotation(newRot);
        syncPosition();
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

        Object* obj = dynamic_cast<Object*>(m_target);
        if (obj) {
            obj->setLocalRect(centeredRect);
            obj->setPos(newPos);
            syncPosition();
        }
    }
}

void TransformBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_state != None) {
        if (Figure* fig = dynamic_cast<Figure*>(m_target)) {
            FigureState startState = fig->getState();
            startState.pos = m_start_pos;
            startState.rot = m_start_rotation;
            startState.rect = m_start_rect;
            m_undo_stack->push(new ModifyFigureCommand(fig, startState, fig->getState()));
        }
        else if (TextObject* txt = dynamic_cast<TextObject*>(m_target)) {
            TextState startState = txt->getState();
            startState.pos = m_start_pos;
            startState.rot = m_start_rotation;
            startState.rect = m_start_rect;
            m_undo_stack->push(new ModifyTextCommand(txt, startState, txt->getState()));
        }
        else if (ImageObject* img = dynamic_cast<ImageObject*>(m_target)) {
            ImageState startState = img->getState();
            startState.pos = m_start_pos;
            startState.rot = m_start_rotation;
            startState.rect = m_start_rect;
            m_undo_stack->push(new ModifyImageCommand(img, startState, img->getState()));
        }
        m_state = None;
        emit interactionEnded();
    } else {
        event->ignore();
    }
}