#include "filterlayer.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>

FilterLayer::FilterLayer(const QString& name, QGraphicsItem* parent)
    : Layer(name, parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);

    setAcceptedMouseButtons(Qt::NoButton);
}

void FilterLayer::setFilterState(const FilterState& state) {
    if (m_filter_state != state) {
        m_filter_state = state;
        applyFilter();
    }
}

void FilterLayer::setCachedImage(const QImage& img) {
    m_cached_image = img;
    applyFilter();
}

void FilterLayer::applyFilter() {
    if (m_cached_image.isNull() || m_cached_image.width() <= 0 || m_cached_image.height() <= 0) return;

    m_rendered_image = m_cached_image;

    if (m_filter_state.type == FilterType::Grayscale) {
        for (int y = 0; y < m_rendered_image.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb*>(m_rendered_image.scanLine(y));
            for (int x = 0; x < m_rendered_image.width(); ++x) {
                int g = qGray(line[x]);
                line[x] = qRgba(g, g, g, qAlpha(line[x]));
            }
        }
    }
    else if (m_filter_state.type == FilterType::Invert) {
        m_rendered_image.invertPixels(QImage::InvertRgb);
    }
    else if (m_filter_state.type == FilterType::BrightnessContrast) {
        float b = m_filter_state.param1;
        float c = m_filter_state.param2;
        float factor = (259.0f * (c + 255.0f)) / (255.0f * (259.0f - c));
        for (int y = 0; y < m_rendered_image.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb*>(m_rendered_image.scanLine(y));
            for (int x = 0; x < m_rendered_image.width(); ++x) {
                QRgb p = line[x];
                if (qAlpha(p) == 0) continue;
                int r = qBound(0, (int)(factor * (qRed(p) - 128) + 128 + b), 255);
                int g = qBound(0, (int)(factor * (qGreen(p) - 128) + 128 + b), 255);
                int bl = qBound(0, (int)(factor * (qBlue(p) - 128) + 128 + b), 255);
                line[x] = qRgba(r, g, bl, qAlpha(p));
            }
        }
    }
    else if (m_filter_state.type == FilterType::Blur) {
        if (m_filter_state.param1 > 0) {
            QGraphicsScene scene;
            QGraphicsPixmapItem* item = scene.addPixmap(QPixmap::fromImage(m_cached_image));
            QGraphicsBlurEffect* eff = new QGraphicsBlurEffect();
            eff->setBlurRadius(m_filter_state.param1);
            item->setGraphicsEffect(eff);

            m_rendered_image.fill(Qt::transparent);
            QPainter p(&m_rendered_image);
            scene.render(&p);
        }
    }
    update(); // Запрашиваем перерисовку слоя
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