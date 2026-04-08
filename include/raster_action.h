#ifndef RASTER_ACTION_H
#define RASTER_ACTION_H

#include <QUndoCommand>
#include <QImage>
#include <vector>
#include <QPointer>
#include "canvas.h"
#include "layer.h"

class RasterizeLayerCommand : public QUndoCommand {
public:
    RasterizeLayerCommand(Canvas* canvas, int layerId, QUndoCommand *parent = nullptr);
    ~RasterizeLayerCommand();

    void undo() override;
    void redo() override;

private:
    Canvas* m_canvas;
    int m_layer_id;

    std::vector<QPointer<Object>> m_objects;

    QImage m_new_raster_image;
    QImage m_old_raster_image;
    bool m_was_rasterized;
    bool m_is_active;
};

class RasterStrokeCommand : public QUndoCommand {
public:
    RasterStrokeCommand(Layer* layer, const QRect& rect, const QImage& oldSubImg, const QImage& newSubImg, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Layer* m_layer;
    QRect m_rect;
    QImage m_old_image;
    QImage m_new_image;
};

#endif // RASTER_ACTION_H