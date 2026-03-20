#include "object.h"

Figure::Figure(const QRectF& rect, FigureType type, QGraphicsItem* parent)
    : Object(parent)
{
    m_state.rect = rect;
    m_state.type = type;
    m_state.fill = QColor(Qt::cyan);
    m_state.stroke = QColor(Qt::black);
    m_state.thickness = 2.0f;
    m_state.pos = QPointF(0, 0);
    m_state.rot = 0.0;
}

Figure::Figure(QGraphicsItem* parent) : Figure(QRectF(0,0,0,0), FigureType::Ellipse, parent) {}

void Figure::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    if (m_state.type == FigureType::Image) {
        if (!m_state.image.isNull()) {
            // Кэшируем QPixmap, чтобы не рескейлить тяжелую картинку каждый кадр (при перетаскивании)
            if (m_cached_pixmap.isNull() || m_cached_rect != m_state.rect) {
                QSize targetSize = m_state.rect.size().toSize();
                if (targetSize.width() > 0 && targetSize.height() > 0) {
                    m_cached_pixmap = QPixmap::fromImage(m_state.image).scaled(
                        targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                } else {
                    m_cached_pixmap = QPixmap();
                }
                m_cached_rect = m_state.rect;
            }
            if (!m_cached_pixmap.isNull()) {
                painter->drawPixmap(m_state.rect.topLeft(), m_cached_pixmap);
            }
        }
    } else {
        painter->setPen(QPen(m_state.stroke, m_state.thickness));
        painter->setBrush(m_state.fill.alpha() > 0 ? QBrush(m_state.fill) : Qt::NoBrush);

        if (m_state.type == FigureType::Ellipse)
            painter->drawEllipse(m_state.rect);
        else if (m_state.type == FigureType::Rectangle)
            painter->drawRect(m_state.rect);
    }

    painter->restore();
}

QRectF Figure::boundingRect() const
{
    if (m_state.type == FigureType::Image) {
        return m_state.rect;
    }

    qreal offset = m_state.thickness / 2.0;
    return m_state.rect.adjusted(-offset, -offset, offset, offset);
}

QPainterPath Figure::shape() const
{
    QPainterPath path;
    if (m_state.type == FigureType::Ellipse) path.addEllipse(m_state.rect);
    else path.addRect(m_state.rect); // Изображения тоже прямоугольные
    return path;
}

void Figure::setLocalRect(const QRectF& rect)
{
    prepareGeometryChange();
    m_state.rect = rect;
    update();
}

QRectF Figure::getLocalRect() const { return m_state.rect; }

FigureState Figure::getState() const
{
    FigureState s = m_state;
    s.pos = pos();
    s.rot = rotation();
    return s;
}

void Figure::setState(const FigureState& state)
{
    prepareGeometryChange();
    m_state = state;
    m_cached_pixmap = QPixmap(); // Сбрасываем кэш, чтобы принудительно перерисовать, если изменилось само изображение
    setPos(state.pos);
    setRotation(state.rot);
    update();
}