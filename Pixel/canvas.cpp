#include "canvas.h"
#include <QDebug>

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
    m_layers.push_back(layer);

    // Если есть сцена, сразу добавляем слой на нее
    if (m_parent_sceene) {
        m_parent_sceene->addItem(layer);
    }

    if (!m_selected) { selectLayer(0); }
}

void Canvas::newLayer()
{
    Layer* l = new Layer();
    l->setName(QString("layer%1").arg(m_layers.size() + 1));
    addLayer(l);
}

void Canvas::draw(QPainter* painter) const
{
    // БОЛЬШЕ НЕ ИСПОЛЬЗУЕТСЯ! Qt всё рисует сам.
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
    if (m_parent_sceene) m_parent_sceene->removeItem(layer);

    delete layer;
    m_layers.erase(m_layers.begin() + id);
}

void Canvas::renderCanvas()
{
    if (!m_parent_sceene) return;

    // 1. Рисуем белый фон холста один раз
    if (!m_bg_item) {
        m_bg_item = m_parent_sceene->addRect(0, 0, m_canvas_size.width(), m_canvas_size.height(),
                                             QPen(Qt::NoPen), QBrush(Qt::white));
        m_bg_item->setZValue(-1);
    }

    // 2. Просто обновляем Z-Value у слоев! Qt сам расставит объекты по порядку
    for (size_t i = 0; i < m_layers.size(); ++i) {
        m_layers[i]->setZValue(i);
    }
}

void Canvas::moveLayer(int id, int shift)
{
    int other_id = (m_layers.size() + id + shift) % m_layers.size();
    if (!ID_IN_BOUNDS(id) || !ID_IN_BOUNDS(other_id)) return;

    auto temp = m_layers[id];
    m_layers[id] = m_layers[other_id];
    m_layers[other_id] = temp;

    m_selected_index = (m_selected_index + shift) % m_layers.size();
    m_selected = m_layers[m_selected_index];

    renderCanvas(); // Обновит Z-индексы
}

void Canvas::selectLayer(int id)
{
    if (!ID_IN_BOUNDS(id)) return;
    m_selected_index = id;
    m_selected = m_layers[id];
}

void Canvas::addObjectToSelectedLayer(Object* obj)
{
    if(!m_selected || !obj) return;
    m_selected->addObject(obj);
}