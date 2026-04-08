#ifndef CANVAS_H
#define CANVAS_H

#include <QObject>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <vector>

#include "layer.h"
#include "object.h"
#include "filterlayer.h"

class Canvas : public QObject {
    Q_OBJECT
public:
    explicit Canvas(QObject* parent = nullptr);

    void addLayer(Layer* layer);
    void newLayer();
    void deleteLayer(const int id);
    void renderCanvas();
    void setScene(QGraphicsScene* scene) { m_parent_sceene = scene; }
    void moveLayer(int id, int shift);
    void setSize(int w, int h);
    QImage renderLayerToImage(int id);

    const std::vector<Layer*>& getLayers() const { return m_layers; }
    QGraphicsScene* getScene() const { return m_parent_sceene; }
    void clearCanvas();

    void selectLayer(int id);
    int getSelectedLayerid() const { return m_selected_index; }

    void setLayerVisible(int id, bool visible);
    void setLayerLocked(int id, bool locked);

    void addObjectToSelectedLayer(Object* obj);
    void moveObjectToLayer(Object* obj, int new_layer_id);
    int getLayerIdOfObject(Object* obj) const;

    std::vector<LayerInfo> getLayersInfo() const;

    void newFilterLayer();
    void updateFilters();
    void setFiltersInteractionActive(bool active);

    // Поддержка режима масок
    int getMaskEditLayerId() const { return m_mask_edit_layer_id; }
    void setMaskEditingMode(int id, bool active);

    QSize getSize() const { return m_canvas_size; }
    void renameLayer(int id, const QString& new_name);
    QGraphicsRectItem* getBgItem() const { return m_bg_item; }

private:
    std::vector<Layer*> m_layers;
    QGraphicsScene* m_parent_sceene;
    Layer* m_selected;
    int m_selected_index;
    QSize m_canvas_size;
    QGraphicsRectItem* m_bg_item;
    int m_layer_counter;

    // Режим маски
    int m_mask_edit_layer_id = -1;
    std::vector<bool> m_pre_mask_visibility;

signals:
    void activeLayerChanged(int id);
    void maskEditingChanged(int id, bool active); // Сигнал для UI
};

#endif // CANVAS_H