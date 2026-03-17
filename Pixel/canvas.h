#ifndef CANVAS_H
#define CANVAS_H

#include <QObject>
#include <QPainter>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <unordered_map>

#include "layer.h"
#include "object.h"

class Canvas : public QObject
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
    int getSelectedLayerid() { return m_selected_index; }
    void addObjectToSelectedLayer(Object* obj);

    std::vector<LayerInfo> getLayersInfo() const;

    QSize getSize() const { return m_canvas_size; }

private:
    std::vector<Layer*> m_layers;
    QGraphicsScene* m_parent_sceene;
    Layer* m_selected;
    int m_selected_index;

    QSize m_canvas_size;

    // Рендер-элементы
    QGraphicsRectItem* m_bg_item; // Белый фон холста
    std::unordered_map<Layer*, QGraphicsPixmapItem*> m_layer_items; // Текстуры слоев
};

#endif // CANVAS_H