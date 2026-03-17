#include "manipulator.h"
#include "action.h"
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

QRectF TransformBox::targetRect() const { return parentItem() ? parentItem()->boundingRect() : QRectF(); }
QRectF TransformBox::boundingRect() const { return targetRect().adjusted(-20, -40, 20, 20); }

// Честное получение масштаба матрицы (чтобы квадратики рамки не плющились при вращении)
void getScaleFromMatrix(const QTransform& t, qreal& sx, qreal& sy) {
    sx = qSqrt(t.m11()*t.m11() + t.m12()*t.m12());
    sy = qSqrt(t.m21()*t.m21() + t.m22()*t.m22());
    if (sx == 0) sx = 1; if (sy == 0) sy = 1;
}

QRectF TransformBox::handleRect(int xPos, int yPos) const {
    QRectF rect = targetRect();
    qreal x = rect.center().x(); qreal y = rect.center().y();
    if (xPos == -1) x = rect.left(); else if (xPos == 1) x = rect.right();
    if (yPos == -1) y = rect.top(); else if (yPos == 1) y = rect.bottom();

    qreal sx, sy; getScaleFromMatrix(parentItem()->sceneTransform(), sx, sy);
    return QRectF(x - (5.0 / sx), y - (5.0 / sy), 10.0 / sx, 10.0 / sy);
}

QRectF TransformBox::rotateHandle() const {
    QRectF rect = targetRect();
    qreal sx, sy; getScaleFromMatrix(parentItem()->sceneTransform(), sx, sy);
    return QRectF(rect.center().x() - (5.0 / sx), rect.top() - (25.0 / sy) - (5.0 / sy), 10.0 / sx, 10.0 / sy);
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

    // Cosmetic Pen (Толщина 0): всегда 1 пиксель на экране, рамка больше не плющится!
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

QPointF TransformBox::getFixedPoint(InteractionState state, QRectF rect) const {
    switch(state) {
    case ResizeTL: return rect.bottomRight();
    case ResizeTC: return QPointF(rect.center().x(), rect.bottom());
    case ResizeTR: return rect.bottomLeft();
    case ResizeML: return QPointF(rect.right(), rect.center().y());
    case ResizeMR: return QPointF(rect.left(), rect.center().y());
    case ResizeBL: return rect.topRight();
    case ResizeBC: return QPointF(rect.center().x(), rect.top());
    case ResizeBR: return rect.topLeft();
    default: return rect.center();
    }
}

void TransformBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!parentItem()) return;

    m_start_pos = parentItem()->pos();
    m_start_rotation = parentItem()->rotation();
    m_start_transform = parentItem()->transform();
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

    // Запоминаем закрепленную точку (противоположную потянутому углу)
    m_fixed_local_point = getFixedPoint(m_state, m_start_rect);
    m_fixed_scene_point = parentItem()->mapToScene(m_fixed_local_point);

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
        // 1. Узнаем, где мышка относительно ИЗНАЧАЛЬНОГО (неискаженного) объекта
        QPointF localMouse = m_initial_scene_transform.inverted().map(event->scenePos());

        qreal sx = 1.0, sy = 1.0;
        qreal w = m_start_rect.width();
        qreal h = m_start_rect.height();
        if (w == 0 || h == 0) return;

        // 2. Считаем масштаб только для нужных осей
        if (m_state == ResizeTR || m_state == ResizeMR || m_state == ResizeBR) sx = (localMouse.x() - m_fixed_local_point.x()) / w;
        if (m_state == ResizeTL || m_state == ResizeML || m_state == ResizeBL) sx = (m_fixed_local_point.x() - localMouse.x()) / w;

        if (m_state == ResizeBL || m_state == ResizeBC || m_state == ResizeBR) sy = (localMouse.y() - m_fixed_local_point.y()) / h;
        if (m_state == ResizeTL || m_state == ResizeTC || m_state == ResizeTR) sy = (m_fixed_local_point.y() - localMouse.y()) / h;

        // Применяем масштаб поверх старого
        QTransform newTransform = m_start_transform;
        newTransform.scale(sx, sy);
        parentItem()->setTransform(newTransform);

        // 3. МАГИЯ КОМПЕНСАЦИИ СДВИГА:
        // Смотрим, куда уехала наша закрепленная точка из-за изменения масштаба...
        QPointF current_fixed_scene = parentItem()->mapToScene(m_fixed_local_point);
        // ...И сдвигаем весь объект обратно, чтобы точка встала на свое законное место!
        QPointF drift = m_fixed_scene_point - current_fixed_scene;
        parentItem()->setPos(parentItem()->pos() + drift);
    }
}

void TransformBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_state != None) {
        // СОХРАНЕНИЕ ТРАНСФОРМАЦИИ В ИСТОРИЮ (Полноценная матрица!)
        m_undo_stack->push(new TransformObjectCommand(
            parentItem(),
            m_start_pos, m_start_rotation, m_start_transform,
            parentItem()->pos(), parentItem()->rotation(), parentItem()->transform()
            ));
        m_state = None;
    } else {
        event->ignore();
    }
}