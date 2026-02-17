#include "canvas.h"

#include <QDebug>

#include "layer.h"
#include "object.h"

#define ID_IN_BOUNDS(id) (id >= 0 || id < m_layers.size())

Canvas::Canvas(QObject* parent)
    : QWidget(qobject_cast<QWidget*>(parent))
    , m_parent_sceene(nullptr)
    , m_selected(nullptr)
{
}

void Canvas::addLayer(Layer* layer)
{
    if (!layer)
    {
        qDebug() << "err: adding null layer";
        return;
    }

    layer->setParent(this);
    m_layers.push_back(layer);

    if (!m_selected)
    {
        selectLayer(0);
    }
}

void Canvas::newLayer()
{
    Layer* l = new Layer(this);
    l->setName(QString("layer%1").arg(m_layers.size() + 1));
    addLayer(l);
}

void Canvas::draw(QPainter* painter) const
{
    for (const auto& layer : m_layers)
    {
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
    if (!ID_IN_BOUNDS(id))
    {
        qDebug() << "err: trying to remove layer " << id;
        return;
    }
    delete m_layers[id];
    m_layers.erase(m_layers.begin() + id);
}

void Canvas::renderCanvas()
{
    if (!this)
    {
        qDebug() << "err: null canvas";
        return;
    }

    if (!m_parent_sceene)
    {
        qDebug() << "err: canvas does not have scene";
        return;
    }

    QRectF rect = m_parent_sceene->sceneRect();
    QPixmap buffer(rect.size().toSize());

    buffer.fill(Qt::white);

    QPainter painter(&buffer);

    if (this)
        this->draw(&painter);

    painter.end();

    m_parent_sceene->clear();
    m_parent_sceene->addPixmap(buffer);
}

void Canvas::moveLayer(int id, int shift)
{
    int other_id = (m_layers.size() + id + shift) % m_layers.size();

    if (!ID_IN_BOUNDS(id) || !ID_IN_BOUNDS(other_id)
        || ! m_layers[id] || ! m_layers[other_id])
    {
        qDebug() << "err: wrong layer id or null on moveLayer";
        return;
    }

    auto temp = m_layers[id];
    m_layers[id] = m_layers[other_id];
    m_layers[other_id] = temp;

    m_selected_index = (m_selected_index + shift) % m_layers.size();
    m_selected = m_layers[m_selected_index];
}

void Canvas::selectLayer(int id)
{
    if (!ID_IN_BOUNDS(id))
    {
        qDebug() << "err: trying to select layer " << id;
        return;
    }
    m_selected_index = id;
    m_selected = m_layers[id];
}

void Canvas::addObjectToSelectedLayer(Object* obj)
{
    if(!m_selected)
    {
        qDebug() << "err: trying to add obj to NULL layer";
        return;
    }
    if(!obj)
    {
        qDebug() << "err: trying to add NULL obj";
        return;
    }
    m_selected->addObject(obj);
}
