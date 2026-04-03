#include "include/raster_action.h"

RasterizeLayerCommand::RasterizeLayerCommand(Canvas* canvas, int layerId, QUndoCommand *parent)
    : QUndoCommand(parent), m_canvas(canvas), m_layer_id(layerId), m_is_active(false)
{
    Layer* layer = m_canvas->getLayers()[m_layer_id];

    // Сохраняем объекты, которые будем "запекать"
    m_objects = layer->getObjects();

    // Запоминаем предыдущее состояние слоя
    m_was_rasterized = layer->isRasterized();
    m_old_raster_image = layer->getRasterImage();

    // Делаем снимок текущего слоя (вместе со старым растром и новыми объектами)
    m_new_raster_image = m_canvas->renderLayerToImage(m_layer_id);
}

RasterizeLayerCommand::~RasterizeLayerCommand() {
    // Если команда уничтожается (лимит стека Undo), и она сейчас применена,
    // значит векторные объекты больше никогда не понадобятся. Удаляем их из памяти.
    if (m_is_active) {
        for (Object* obj : m_objects) {
            delete obj;
        }
    }
}

void RasterizeLayerCommand::redo() {
    Layer* layer = m_canvas->getLayers()[m_layer_id];

    // Убираем векторные объекты И со слоя, И со сцены!
    for (Object* obj : m_objects) {
        layer->removeObject(obj);
        if (m_canvas->getScene()) {
            m_canvas->getScene()->removeItem(obj);
        }
    }

    layer->setRasterized(true);
    layer->setRasterImage(m_new_raster_image);
    m_is_active = true;
}

void RasterizeLayerCommand::undo() {
    Layer* layer = m_canvas->getLayers()[m_layer_id];

    // Возвращаем слой к предыдущему состоянию
    layer->setRasterized(m_was_rasterized);
    layer->setRasterImage(m_old_raster_image);

    // Возвращаем векторы на слой И на сцену
    for (Object* obj : m_objects) {
        if (m_canvas->getScene()) {
            m_canvas->getScene()->addItem(obj);
        }
        layer->addObject(obj);
    }
    m_is_active = false;
}

RasterStrokeCommand::RasterStrokeCommand(Layer* layer, const QImage& oldImg, const QImage& newImg, QUndoCommand *parent)
    : QUndoCommand(parent), m_layer(layer), m_old_image(oldImg), m_new_image(newImg) {}

void RasterStrokeCommand::undo() {
    m_layer->setRasterImage(m_old_image);
}

void RasterStrokeCommand::redo() {
    m_layer->setRasterImage(m_new_image);
}