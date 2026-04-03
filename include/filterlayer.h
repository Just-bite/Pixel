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

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    FilterState m_filter_state;
    std::unique_ptr<BaseFilter> m_filter;
    QImage m_cached_image;
    QImage m_rendered_image;

    void applyFilter();
};

#endif // FILTERLAYER_H