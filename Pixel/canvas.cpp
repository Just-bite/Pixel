#include "canvas.h"
#include <QDebug>
#include "layer.h"
#include "object.h"

#define ID_IN_BOUNDS(id) (id >= 0 || id < m_layers.size())

Canvas::Canvas(QObject* parent)
    : QObject(parent)
    , m_parent_sceene(nullptr)
    , m_selected(nullptr)
    , m_canvas_size(800, 600)
    , m_bg_item(nullptr)
{
}

void Canvas::addLayer(Layer* layer)
{
    if (!layer) { qDebug() << "err: adding null layer"; return; }
    layer->setParent(this);
    m_layers.push_back(layer);
    if (!m_selected) { selectLayer(0); }
}

void Canvas::newLayer()
{
    Layer* l = new Layer(this);
    l->setName(QString("layer%1").arg(m_layers.size() + 1));
    addLayer(l);
}

void Canvas::draw(QPainter* painter) const
{
    for (const auto& layer : m_layers) {
        layer->draw(painter);
    }
}

std::vector<LayerInfo> Canvas::getLayersInfo() const
{
    std::vector<LayerInfo> res;
    for (const auto& l : m_layers) { res.push_back(l->getInfo()); }
    return res;
}

void Canvas::deleteLayer(const int id)
{
    if (!ID_IN_BOUNDS(id)) { qDebug() << "err: trying to remove layer " << id; return; }

    Layer* layer = m_layers[id];

    // Удаляем графический элемент со сцены
    if (m_layer_items.count(layer)) {
        if (m_parent_sceene) m_parent_sceene->removeItem(m_layer_items[layer]);
        delete m_layer_items[layer];
        m_layer_items.erase(layer);
    }

    delete layer;
    m_layers.erase(m_layers.begin() + id);
}

void Canvas::renderCanvas()
{
    if (!m_parent_sceene) { qDebug() << "err: canvas does not have scene"; return; }

    // 1. Рисуем белый фон холста (только один раз), на Z-индексе -1
    if (!m_bg_item) {
        m_bg_item = m_parent_sceene->addRect(0, 0, m_canvas_size.width(), m_canvas_size.height(),
                                             QPen(Qt::NoPen), QBrush(Qt::white));
        m_bg_item->setZValue(-1);
    }

    // 2. Обновляем каждый слой отдельно! Никакого m_parent_scene->clear()
    for (size_t i = 0; i < m_layers.size(); ++i) {
        Layer* layer = m_layers[i];

        // Создаем итем слоя, если его еще нет
        if (m_layer_items.find(layer) == m_layer_items.end()) {
            QGraphicsPixmapItem* item = new QGraphicsPixmapItem();
            m_parent_sceene->addItem(item);
            m_layer_items[layer] = item;
        }

        QGraphicsPixmapItem* item = m_layer_items[layer];

        // Порядок в векторе = Z-index на экране (нижние рисуются раньше, Z меньше)
        item->setZValue(i);

        // Если слой видим — перерисовываем его пиксельмап
        if (layer->isVisible()) {
            QPixmap pixmap(m_canvas_size);
            pixmap.fill(Qt::transparent); // Слой изначально прозрачный!

            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            layer->draw(&painter);
            painter.end();

            item->setPixmap(pixmap);
            item->show();
        } else {
            item->hide();
        }
    }
}

void Canvas::moveLayer(int id, int shift)
{
    int other_id = (m_layers.size() + id + shift) % m_layers.size();
    if (!ID_IN_BOUNDS(id) || !ID_IN_BOUNDS(other_id) || !m_layers[id] || !m_layers[other_id]) {
        qDebug() << "err: wrong layer id or null on moveLayer"; return;
    }
    auto temp = m_layers[id];
    m_layers[id] = m_layers[other_id];
    m_layers[other_id] = temp;
    m_selected_index = (m_selected_index + shift) % m_layers.size();
    m_selected = m_layers[m_selected_index];

    // Просто вызываем renderCanvas() — он сам расставит новые Z-индексы
}

void Canvas::selectLayer(int id)
{
    if (!ID_IN_BOUNDS(id)) { qDebug() << "err: trying to select layer " << id; return; }
    m_selected_index = id;
    m_selected = m_layers[id];
}

void Canvas::addObjectToSelectedLayer(Object* obj)
{
    if(!m_selected) { qDebug() << "err: trying to add obj to NULL layer"; return; }
    if(!obj) { qDebug() << "err: trying to add NULL obj"; return; }
    m_selected->addObject(obj);
}