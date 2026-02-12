#ifndef CANVAS_H
#define CANVAS_H

#include <QObject>
#include <QPainter>
#include <QWidget>
#include <QPaintEvent>

#include "layer.h"

class Canvas : public QWidget
{
    Q_OBJECT

public:
    explicit Canvas(QObject* parent = nullptr);
    void addLayer(Layer* layer);
    void newLayer();
    void draw(QPainter* painter) const;

    std::vector<LayerInfo> getLayersInfo() const;

private:
    std::vector<Layer*> m_layers;
    Layer* m_selected;

public slots:
    void paintEvent(QPaintEvent* event) const {}
};

#endif // CANVAS_H
