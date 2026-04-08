#include "include\canvas.h"
#include "include\manipulator.h"

#define ID_IN_BOUNDS(id) (id >= 0 && id < (int)m_layers.size())

Canvas::Canvas(QObject* parent)
    : QObject(parent),
    m_parent_sceene(nullptr),
    m_selected(nullptr),
    m_selected_index(-1),
    m_canvas_size(800, 600),
    m_bg_item(nullptr),
    m_layer_counter(1) {}

void Canvas::addLayer(Layer* layer) {
    if (!layer)
        return;
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
    for (const auto& l : m_layers)
        res.push_back(l->getInfo());
    return res;
}

void Canvas::deleteLayer(const int id) {
    if (!ID_IN_BOUNDS(id)) return;
    if (m_parent_sceene) m_parent_sceene->clearSelection();

    Layer* layer = m_layers[id];
    if (m_parent_sceene) m_parent_sceene->removeItem(layer);

    delete layer;
    m_layers.erase(m_layers.begin() + id);

    if (m_layers.empty()) {
        m_selected = nullptr;
        m_selected_index = -1;
        emit activeLayerChanged(-1);
    } else {
        selectLayer(std::max(0, m_selected_index - 1));
    }
    renderCanvas();
    updateFilters();
}

void Canvas::renderCanvas() {
    if (!m_parent_sceene)
        return;
    if (!m_bg_item) {
        m_bg_item = m_parent_sceene->addRect(0, 0, m_canvas_size.width(), m_canvas_size.height(), QPen(Qt::NoPen), QBrush(Qt::white));
        m_bg_item->setZValue(-1);
    }
    for (size_t i = 0; i < m_layers.size(); ++i)
        m_layers[i]->setZValue(i);
}

void Canvas::moveLayer(int id, int shift) {
    if (m_layers.empty())
        return;
    int other_id = (m_layers.size() + id + shift) % m_layers.size();
    if (!ID_IN_BOUNDS(id) || !ID_IN_BOUNDS(other_id))
        return;
    std::swap(m_layers[id], m_layers[other_id]);
    if (m_selected_index == id)
        m_selected_index = other_id;
    else if (m_selected_index == other_id)
        m_selected_index = id;
    m_selected = m_layers[m_selected_index];
    renderCanvas();
    updateFilters();
}

void Canvas::selectLayer(int id) {
    if (!ID_IN_BOUNDS(id)) return;

    // ИСПРАВЛЕНИЕ: Выходим из режима маски, если выбрали другой слой
    if (m_mask_edit_layer_id != -1 && m_mask_edit_layer_id != id) {
        setMaskEditingMode(m_mask_edit_layer_id, false);
    }

    m_selected_index = id;
    m_selected = m_layers[id];
    emit activeLayerChanged(id);
}

void Canvas::setLayerVisible(int id, bool visible) {
    if (ID_IN_BOUNDS(id)) {
        m_layers[id]->setVisible(visible);
        updateFilters();
    }
}

void Canvas::setLayerLocked(int id, bool locked) {
    if (ID_IN_BOUNDS(id))
        m_layers[id]->setLocked(locked);
}

void Canvas::addObjectToSelectedLayer(Object* obj) {
    if (m_selected && obj)
        m_selected->addObject(obj);
}

int Canvas::getLayerIdOfObject(Object* obj) const {
    if (!obj) return -1;
    for (int i = 0; i < (int)m_layers.size(); ++i) {
        if (obj->parentItem() == m_layers[i])
            return i;
    }
    return -1;
}

void Canvas::moveObjectToLayer(Object* obj, int new_layer_id) {
    if (!obj || !ID_IN_BOUNDS(new_layer_id))
        return;
    int old_id = getLayerIdOfObject(obj);
    if (old_id != -1 && old_id != new_layer_id) {
        m_layers[old_id]->removeObject(obj);
        m_layers[new_layer_id]->addObject(obj);
    }
}

void Canvas::renameLayer(int id, const QString& new_name) {
    if (ID_IN_BOUNDS(id))
        m_layers[id]->setName(new_name);
}

void Canvas::clearCanvas() {
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

void Canvas::newFilterLayer() {
    m_layer_counter++;
    FilterLayer* l = new FilterLayer(QString("Filter%1").arg(m_layer_counter));
    addLayer(l);
    updateFilters();
}

void Canvas::updateFilters() {
    if (!m_parent_sceene) return;

    if (m_canvas_size.width() <= 0 || m_canvas_size.height() <= 0) return;

    std::vector<QGraphicsItem*> hidden_ui;

    for (QGraphicsItem* item : m_parent_sceene->items()) {
        if (dynamic_cast<TransformBox*>(item) && item->isVisible()) {
            item->setVisible(false);
            hidden_ui.push_back(item);
        }
    }

    for (size_t i = 0; i < m_layers.size(); ++i) {
        if (m_layers[i]->isFilter() && m_layers[i]->isVisible()) {
            FilterLayer* fl = static_cast<FilterLayer*>(m_layers[i]);

            std::vector<bool> visibility(m_layers.size());
            for (size_t j = i; j < m_layers.size(); ++j) {
                visibility[j] = m_layers[j]->isVisible();
                m_layers[j]->setVisible(false);
            }

            QImage buffer(m_canvas_size, QImage::Format_ARGB32_Premultiplied);
            buffer.fill(Qt::transparent);
            QPainter p(&buffer);
            m_parent_sceene->render(&p, QRectF(0,0,m_canvas_size.width(), m_canvas_size.height()), QRectF(0,0,m_canvas_size.width(), m_canvas_size.height()));
            p.end();

            for (size_t j = i; j < m_layers.size(); ++j) {
                m_layers[j]->setVisible(visibility[j]);
            }

            fl->setCachedImage(buffer);
        }
    }

    for (QGraphicsItem* item : hidden_ui) item->setVisible(true);
}

void Canvas::setFiltersInteractionActive(bool active) {
    for (Layer* l : m_layers) {
        if (l->isFilter()) {
            l->setOpacity(active ? 1.0 : 0.0);
            if (active) l->update();
        }
    }
    if (active) {
        updateFilters();
    }
}

QImage Canvas::renderLayerToImage(int id) {
    if (!m_parent_sceene || !ID_IN_BOUNDS(id)) return QImage();
            // Прячем UI (TransformBox)
    std::vector<QGraphicsItem*> hidden_ui;
    for (QGraphicsItem* item : m_parent_sceene->items()) {
        if (dynamic_cast<TransformBox*>(item) && item->isVisible()) {
            item->setVisible(false);
            hidden_ui.push_back(item);
        }
    }

    if (m_bg_item) m_bg_item->setVisible(false);
    // Сохраняем видимость слоев и прячем все кроме нужного
    std::vector<bool> visibility(m_layers.size());
    for (size_t i = 0; i < m_layers.size(); ++i) {
        visibility[i] = m_layers[i]->isVisible();
        m_layers[i]->setVisible((int)i == id);
    }



    // Рендерим
    QImage buffer(m_canvas_size, QImage::Format_ARGB32_Premultiplied);
    buffer.fill(Qt::transparent);
    QPainter p(&buffer);
    p.setRenderHint(QPainter::Antialiasing);

    // ВАЖНО: Мы рендерим сцену, но т.к. видимый только 1 слой, отрендерится только он
    m_parent_sceene->render(&p, QRectF(0, 0, m_canvas_size.width(), m_canvas_size.height()),
                            QRectF(0, 0, m_canvas_size.width(), m_canvas_size.height()));
    p.end();

    if (m_bg_item) m_bg_item->setVisible(true);
    // Восстанавливаем видимость слоев
    for (size_t i = 0; i < m_layers.size(); ++i) {
        m_layers[i]->setVisible(visibility[i]);
    }

    // Восстанавливаем UI
    for (QGraphicsItem* item : hidden_ui) item->setVisible(true);

    return buffer;
}

void Canvas::setMaskEditingMode(int id, bool active) {
    if (!ID_IN_BOUNDS(id)) return;
    Layer* targetLayer = m_layers[id];
    if (!targetLayer->isFilter()) return;

    FilterLayer* fl = static_cast<FilterLayer*>(targetLayer);

    if (active) {
        // Если уже была активна другая маска, отключаем её
        if (m_mask_edit_layer_id != -1 && m_mask_edit_layer_id != id) {
            setMaskEditingMode(m_mask_edit_layer_id, false);
        }

        m_mask_edit_layer_id = id;
        m_pre_mask_visibility.clear();

        // Запоминаем видимость и прячем все слои, кроме текущего
        for (size_t i = 0; i < m_layers.size(); ++i) {
            m_pre_mask_visibility.push_back(m_layers[i]->isVisible());
            if ((int)i != id) m_layers[i]->setVisible(false);
        }

        // Включаем визуальный режим маски
        fl->setVisible(true);
        fl->setMaskVisualMode(true);

        // Автоматически выделяем этот слой
        if (m_selected_index != id) selectLayer(id);

    } else {
        if (m_mask_edit_layer_id == id) {
            fl->setMaskVisualMode(false);
            m_mask_edit_layer_id = -1;

            // Восстанавливаем видимость
            for (size_t i = 0; i < m_layers.size() && i < m_pre_mask_visibility.size(); ++i) {
                m_layers[i]->setVisible(m_pre_mask_visibility[i]);
            }
            m_pre_mask_visibility.clear();

            // Пересчитываем фильтр с новой маской
            fl->applyFilter();
        }
    }
    emit maskEditingChanged(id, active);
}