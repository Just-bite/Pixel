#ifndef FILTERLAYER_H
#define FILTERLAYER_H

#include "layer.h"
#include <QImage>

enum class FilterType { None, Grayscale, Invert, BrightnessContrast, Blur };

struct FilterState {
    FilterType type = FilterType::Grayscale;
    float param1 = 0.0f;
    float param2 = 0.0f;

    bool operator==(const FilterState& o) const {
        return type == o.type && qAbs(param1 - o.param1) < 0.01f && qAbs(param2 - o.param2) < 0.01f;
    }
    bool operator!=(const FilterState& o) const { return !(*this == o); }
};

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
    QImage m_cached_image;
    QImage m_rendered_image;

    void applyFilter();
};

#endif // FILTERLAYER_H