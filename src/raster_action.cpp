#include "include/raster_action.h"

RasterizeLayerCommand::RasterizeLayerCommand(Canvas* canvas, int layerId, QUndoCommand *parent)
    : QUndoCommand(parent), m_canvas(canvas), m_layer_id(layerId), m_is_active(false)
{
    Layer* layer = m_canvas->getLayers()[m_layer_id];

    std::vector<Object*> raw_objects = layer->getObjects();
    m_objects.reserve(raw_objects.size());
    for (Object* obj : raw_objects) {
        m_objects.push_back(QPointer<Object>(obj));
    }

    m_was_rasterized = layer->isRasterized();
    m_old_raster_image = layer->getRasterImage();

    m_new_raster_image = m_canvas->renderLayerToImage(m_layer_id);
}

RasterizeLayerCommand::~RasterizeLayerCommand() {
    if (m_is_active) {
        for (auto& obj : m_objects) {
            if (obj) delete obj;
        }
    }
}

void RasterizeLayerCommand::redo() {
    Layer* layer = m_canvas->getLayers()[m_layer_id];
    for (auto& obj : m_objects) {
        if (obj) {
            layer->removeObject(obj);
            if (m_canvas->getScene()) m_canvas->getScene()->removeItem(obj);
        }
    }
    layer->setRasterized(true);
    layer->setRasterImage(m_new_raster_image);
    m_is_active = true;
}

void RasterizeLayerCommand::undo() {
    Layer* layer = m_canvas->getLayers()[m_layer_id];
    layer->setRasterized(m_was_rasterized);
    layer->setRasterImage(m_old_raster_image);

    for (auto& obj : m_objects) {
        if (obj) {
            if (m_canvas->getScene()) m_canvas->getScene()->addItem(obj);
            layer->addObject(obj);
        }
    }
    m_is_active = false;
}

RasterStrokeCommand::RasterStrokeCommand(Layer* layer, const QRect& rect, const QImage& oldSubImg, const QImage& newSubImg, QUndoCommand *parent)
    : QUndoCommand(parent), m_layer(layer), m_rect(rect), m_old_image(oldSubImg), m_new_image(newSubImg) {}

void RasterStrokeCommand::undo() {
    QPainter p(m_layer->getRasterImagePtr());

    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawImage(m_rect.topLeft(), m_old_image);
    m_layer->updateRasterArea(m_rect);
}

void RasterStrokeCommand::redo() {
    QPainter p(m_layer->getRasterImagePtr());
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawImage(m_rect.topLeft(), m_new_image);
    m_layer->updateRasterArea(m_rect);
}