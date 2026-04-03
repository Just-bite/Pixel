#ifndef LAYER_H
#define LAYER_H

#include <QGraphicsObject>
#include <QString>
#include <QImage>
#include <QPainter>

#include "object.h"

struct LayerInfo {
    QString name;
    bool visible;
    bool locked;
    bool isFilter;
    bool isRasterized;
};

class Layer : public QGraphicsObject {
    Q_OBJECT
public:
    explicit Layer(QGraphicsItem* parent = nullptr);
    explicit Layer(const QString& name, QGraphicsItem* parent = nullptr);
    ~Layer() override;

    std::vector<Object*> getObjects() const;

    void addObject(Object* object);
    void removeObject(Object* object);

    void setName(const QString& name) { m_name = name; }
    QString getName() const { return m_name; }

    void setVisible(bool visible);
    bool isVisible() const { return m_visible; }

    void setLocked(const bool locked);
    bool isLocked() const { return m_locked; }

    void setRasterized(bool rasterized);
    bool isRasterized() const { return m_is_rasterized; }

    void setRasterImage(const QImage& img);
    QImage getRasterImage() const { return m_raster_image; }

    LayerInfo getInfo() const;

    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    virtual bool isFilter() const { return false; }

private:
    bool m_visible;
    bool m_locked;
    bool m_is_rasterized = false;
    QImage m_raster_image;
    QString m_name;
};

#endif // LAYER_H