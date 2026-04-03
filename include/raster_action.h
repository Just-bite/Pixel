#ifndef RASTER_ACTION_H
#define RASTER_ACTION_H

#include <QUndoCommand>
#include <QImage>
#include <vector>
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

    std::vector<Object*> m_objects;

    QImage m_new_raster_image;
    QImage m_old_raster_image;
    bool m_was_rasterized;

    bool m_is_active; // Флаг для контроля памяти
};

#endif // RASTER_ACTION_H