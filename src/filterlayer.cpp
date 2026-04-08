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

void FilterLayer::setMaskVisualMode(bool active) {
    m_mask_visual_mode = active;
    update();
}

void FilterLayer::applyFilter() {
    if (m_cached_image.isNull() || m_cached_image.width() <= 0 || m_cached_image.height() <= 0) return;

    QImage filtered;
    if (m_filter) {
        filtered = m_filter->apply(m_cached_image, m_filter_state.params);
    } else {
        filtered = m_cached_image;
    }

    QImage mask = getRasterImage();

    // Если маски нет, применяем фильтр на 100%
    if (mask.isNull()) {
        m_rendered_image = filtered;
    } else {
        // СМЕШИВАНИЕ (Блендинг) оригинальной и отфильтрованной картинки по маске
        m_rendered_image = m_cached_image.copy(); // Основа - оригинал
        int w = m_rendered_image.width();
        int h = m_rendered_image.height();
        int mw = mask.width();
        int mh = mask.height();

        for (int y = 0; y < h; ++y) {
            QRgb* orig = reinterpret_cast<QRgb*>(m_cached_image.scanLine(y));
            QRgb* filt = reinterpret_cast<QRgb*>(filtered.scanLine(y));
            QRgb* out = reinterpret_cast<QRgb*>(m_rendered_image.scanLine(y));

            int my = (y < mh) ? y : mh - 1;
            QRgb* msk = reinterpret_cast<QRgb*>(mask.scanLine(my));

            for (int x = 0; x < w; ++x) {
                int mx = (x < mw) ? x : mw - 1;
                int a = qAlpha(msk[mx]);
                int mask_val = (a == 0) ? 0 : qGray(msk[mx]);

                if (mask_val == 255) {
                    out[x] = filt[x];
                } else if (mask_val == 0) {
                    out[x] = orig[x];
                } else {
                    int r = (qRed(orig[x]) * (255 - mask_val) + qRed(filt[x]) * mask_val) / 255;
                    int g = (qGreen(orig[x]) * (255 - mask_val) + qGreen(filt[x]) * mask_val) / 255;
                    int b = (qBlue(orig[x]) * (255 - mask_val) + qBlue(filt[x]) * mask_val) / 255;
                    int alpha = (qAlpha(orig[x]) * (255 - mask_val) + qAlpha(filt[x]) * mask_val) / 255;
                    out[x] = qRgba(r, g, b, alpha);
                }
            }
        }
    }
    update();
}

QRectF FilterLayer::boundingRect() const {
    if (m_mask_visual_mode) {
        return QRectF(0, 0, m_cached_image.width(), m_cached_image.height());
    }
    if (m_rendered_image.isNull()) return QRectF();
    return QRectF(0, 0, m_rendered_image.width(), m_rendered_image.height());
}

void FilterLayer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (m_mask_visual_mode) {
        // В режиме редактирования маски рисуем саму маску
        QImage mask = getRasterImage();
        if (!mask.isNull()) {
            painter->drawImage(0, 0, mask);
        } else {
            painter->fillRect(0, 0, m_cached_image.width(), m_cached_image.height(), Qt::white);
        }
    } else {
        // В обычном режиме рисуем результат фильтрации
        if (!m_rendered_image.isNull() && isVisible()) {
            painter->drawImage(0, 0, m_rendered_image);
        }
    }
}