#ifndef CANVAS_H
#define CANVAS_H

#include <QObject>
#include <QPainter>
#include <QWidget>
#include <QPaintEvent>
#include <QGraphicsScene>

#include "layer.h"

class Canvas : public QWidget
{
    Q_OBJECT

public:
    explicit Canvas(QObject* parent = nullptr);
    void addLayer(Layer* layer);
    void newLayer();
    void draw(QPainter* painter) const;
    void deleteLayer(const int id);
    void renderCanvas();
    void setScene(QGraphicsScene* scene) { m_parent_sceene = scene; }
    void moveLayer(int id, int shift);
    void selectLayer(int id);

    std::vector<LayerInfo> getLayersInfo() const;

private:
    std::vector<Layer*> m_layers;
    QGraphicsScene* m_parent_sceene;
    Layer* m_selected;


public slots:
    void paintEvent(QPaintEvent* event) const {}
};

#endif // CANVAS_H
