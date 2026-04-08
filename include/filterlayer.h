#ifndef FILTERLAYER_H
#define FILTERLAYER_H

#include "layer.h"
#include "filter.h"
#include <QImage>

class FilterLayer : public Layer {
    Q_OBJECT
public:
    explicit FilterLayer(const QString& name, QGraphicsItem* parent = nullptr);

    bool isFilter() const override { return true; }

    FilterState getFilterState() const { return m_filter_state; }
    void setFilterState(const FilterState& state);

    void setCachedImage(const QImage& img);
    void applyFilter();

    void setMaskVisualMode(bool active);
    bool isMaskVisualMode() const { return m_mask_visual_mode; }

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    FilterState m_filter_state;
    std::unique_ptr<BaseFilter> m_filter;
    QImage m_cached_image;
    QImage m_rendered_image;
    bool m_mask_visual_mode = false;
};

#endif // FILTERLAYER_H