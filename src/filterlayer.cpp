#include "include\filterlayer.h"
#include <QPainter>

FilterLayer::FilterLayer(const QString& name, QGraphicsItem* parent)
    : Layer(name, parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setAcceptedMouseButtons(Qt::NoButton);
    m_filter = FilterFactory::createFilter(FilterType::None);
}

void FilterLayer::setFilterState(const FilterState& state) {
    if (m_filter_state != state) {
        bool typeChanged = (m_filter_state.type != state.type);
        m_filter_state = state;

        if (typeChanged) {
            m_filter = FilterFactory::createFilter(state.type);
        }
        applyFilter();
    }
}

void FilterLayer::setCachedImage(const QImage& img) {
    m_cached_image = img;
    applyFilter();
}

void FilterLayer::applyFilter() {
    if (m_cached_image.isNull() || m_cached_image.width() <= 0 || m_cached_image.height() <= 0) return;

    if (m_filter) {
        m_rendered_image = m_filter->apply(m_cached_image, m_filter_state.params);
    } else {
        m_rendered_image = m_cached_image;
    }
    update();
}

QRectF FilterLayer::boundingRect() const {
    if (m_rendered_image.isNull()) return QRectF();
    return QRectF(0, 0, m_rendered_image.width(), m_rendered_image.height());
}

void FilterLayer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (!m_rendered_image.isNull() && isVisible()) {
        painter->drawImage(0, 0, m_rendered_image);
    }
}