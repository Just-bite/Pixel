#include "canvas.h"
#include <QDebug>

Canvas::Canvas(QObject* parent) : QWidget(qobject_cast<QWidget*>(parent))
{
    m_selected = nullptr;
}

void Canvas::addLayer(Layer* layer)
{
    layer->setParent(this);
    m_layers.push_back(layer);
}

void Canvas::newLayer()
{
    Layer* l = new Layer(this);
    l->setName(QString("layer%1").arg(m_layers.size() + 1));
    m_layers.push_back(l);
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
    for (const auto& l : m_layers)
    {
        res.push_back(l->getInfo());
    }
    return res;
}

void Canvas::deleteLayer(const int id)
{
    if (id < 0 || id >= m_layers.size()) {
        qDebug() << "err: trying to remove layer " << id;
        return;
    }
    delete m_layers[id];
    m_layers.erase(m_layers.begin() + id);
}
