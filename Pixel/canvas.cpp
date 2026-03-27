#include "canvas.h"
#define ID_IN_BOUNDS(id) (id >= 0 && id < (int)m_layers.size())

Canvas::Canvas(QObject* parent) : QObject(parent), m_parent_sceene(nullptr), m_selected(nullptr), m_selected_index(-1), m_canvas_size(800, 600), m_bg_item(nullptr), m_layer_counter(1) {}

void Canvas::addLayer(Layer* layer) {
    if (!layer) return;
    m_layers.push_back(layer);
    if (m_layer_counter < (int)m_layers.size()) {
        m_layer_counter = m_layers.size();
    }
    if (m_parent_sceene) m_parent_sceene->addItem(layer);
    if (!m_selected) selectLayer(0);
    renderCanvas();
}

void Canvas::newLayer() {
    m_layer_counter++;
    Layer* l = new Layer();
    l->setName(QString("layer%1").arg(m_layer_counter));
    addLayer(l);
}

std::vector<LayerInfo> Canvas::getLayersInfo() const {
    std::vector<LayerInfo> res;
    for (const auto& l : m_layers) res.push_back(l->getInfo());
    return res;
}

void Canvas::deleteLayer(const int id) {
    if (!ID_IN_BOUNDS(id)) return;

    // ЗАЩИТА ОТ КРАША: Снимаем выделение, если удаляем конкретный слой
    if (m_parent_sceene) {
        m_parent_sceene->clearSelection();
    }

    Layer* layer = m_layers[id];
    if (m_parent_sceene) m_parent_sceene->removeItem(layer);

    delete layer;
    m_layers.erase(m_layers.begin() + id);

    if (m_layers.empty()) { m_selected = nullptr; m_selected_index = -1; }
    else selectLayer(std::max(0, m_selected_index - 1));
    renderCanvas();
}

void Canvas::renderCanvas() {
    if (!m_parent_sceene) return;
    if (!m_bg_item) {
        m_bg_item = m_parent_sceene->addRect(0, 0, m_canvas_size.width(), m_canvas_size.height(), QPen(Qt::NoPen), QBrush(Qt::white));
        m_bg_item->setZValue(-1);
    }
    for (size_t i = 0; i < m_layers.size(); ++i) m_layers[i]->setZValue(i);
}

void Canvas::moveLayer(int id, int shift) {
    if (m_layers.empty()) return;
    int other_id = (m_layers.size() + id + shift) % m_layers.size();
    if (!ID_IN_BOUNDS(id) || !ID_IN_BOUNDS(other_id)) return;
    std::swap(m_layers[id], m_layers[other_id]);
    if (m_selected_index == id) m_selected_index = other_id;
    else if (m_selected_index == other_id) m_selected_index = id;
    m_selected = m_layers[m_selected_index];
    renderCanvas();
}

void Canvas::selectLayer(int id) {
    if (!ID_IN_BOUNDS(id)) return;
    m_selected_index = id;
    m_selected = m_layers[id];
}

void Canvas::setLayerVisible(int id, bool visible) {
    if (ID_IN_BOUNDS(id)) m_layers[id]->setVisible(visible);
}

void Canvas::setLayerLocked(int id, bool locked) {
    if (ID_IN_BOUNDS(id)) m_layers[id]->setLocked(locked);
}

void Canvas::addObjectToSelectedLayer(Object* obj) {
    if (m_selected && obj) m_selected->addObject(obj);
}

int Canvas::getLayerIdOfObject(Object* obj) const {
    if (!obj) return -1;
    for (int i = 0; i < (int)m_layers.size(); ++i) {
        if (obj->parentItem() == m_layers[i]) return i;
    }
    return -1;
}

void Canvas::moveObjectToLayer(Object* obj, int new_layer_id) {
    if (!obj || !ID_IN_BOUNDS(new_layer_id)) return;
    int old_id = getLayerIdOfObject(obj);
    if (old_id != -1 && old_id != new_layer_id) {
        m_layers[old_id]->removeObject(obj);
        m_layers[new_layer_id]->addObject(obj);
    }
}

void Canvas::renameLayer(int id, const QString& new_name) {
    if (ID_IN_BOUNDS(id)) m_layers[id]->setName(new_name);
}

void Canvas::clearCanvas() {
    // ЗАЩИТА ОТ КРАША: Снимаем выделение до того, как начнем разрушать объекты
    if (m_parent_sceene) {
        m_parent_sceene->clearSelection();
    }

    while (!m_layers.empty()) {
        deleteLayer(0);
    }
    m_layer_counter = 0;
}

void Canvas::setSize(int w, int h) {
    m_canvas_size = QSize(w, h);
    if (m_bg_item) {
        m_bg_item->setRect(0, 0, w, h);
    }
    renderCanvas();
}