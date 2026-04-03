#include "include/concrete_tools.h"
#include "include/action.h"
#include "include/projectmanager.h"
#include "include/contextpannel.h"
#include "include/palettepannel.h"
#include "include/layerspannel.h"
#include <QScrollBar>
#include <QInputDialog>
#include "include/workspacecontroller.h"
#include <QDebug>

// ==================== POINTER TOOL ====================
PointerTool::PointerTool(QObject* parent) : Tool(parent) {}

void PointerTool::onActivate(const WorkspaceContext& ctx) {
    ctx.view->setCursor(Qt::ArrowCursor);
    onSelectionChanged(ctx); // Восстанавливаем выделение
}

void PointerTool::onDeactivate(const WorkspaceContext& ctx) {
    clearTransformBox(ctx);
    ctx.contextPannel->setMode(false, false, false, false, "");
}

void PointerTool::clearTransformBox(const WorkspaceContext& ctx) {
    if (m_transform_box) {
        if (m_transform_box->scene()) {
            m_transform_box->scene()->removeItem(m_transform_box);
        }
        delete m_transform_box;
        m_transform_box = nullptr;
    }
}

void PointerTool::updateContextPanel(const WorkspaceContext& ctx) {
    QList<QGraphicsItem*> selected = ctx.scene->selectedItems();
    if (selected.isEmpty()) {
        ctx.contextPannel->setMode(false, false, false, false, "Pointer");
        ctx.contextPannel->setTarget(static_cast<Figure*>(nullptr));
        return;
    }

    Object* obj = dynamic_cast<Object*>(selected.first());
    Figure* fig = dynamic_cast<Figure*>(obj);
    TextObject* txt = dynamic_cast<TextObject*>(obj);
    ImageObject* img = dynamic_cast<ImageObject*>(obj);

    if (fig) ctx.contextPannel->setTarget(fig);
    else if (txt) ctx.contextPannel->setTarget(txt);
    else if (img) ctx.contextPannel->setTarget(img);

    ctx.contextPannel->setMode(fig != nullptr, txt != nullptr, false, false, "Pointer");
    if (fig || txt || img) {
        ctx.palettePannel->setColor(ctx.contextPannel->getActiveColor());
    }
}

void PointerTool::onSelectionChanged(const WorkspaceContext& ctx) {
    // ЗАЩИТА ОТ РЕКУРСИВНОГО СОЗДАНИЯ TRANSFORM BOX
    if (m_updating_selection) return;
    m_updating_selection = true;

    clearTransformBox(ctx);
    QList<QGraphicsItem*> selected = ctx.scene->selectedItems();
    Canvas* canvas = ctx.projectManager->GetCurrentCanvas();

    if (!selected.isEmpty() && canvas) {
        QGraphicsItem* item = selected.first();
        Object* obj = dynamic_cast<Object*>(item);

        int layerId = canvas->getLayerIdOfObject(obj);
        if (layerId != -1 && canvas->getLayersInfo()[layerId].locked) {
            item->setSelected(false);
            m_updating_selection = false; // Сбрасываем флаг перед выходом
            return;
        }

        if (layerId != -1 && layerId != canvas->getSelectedLayerid()) {
            if (ctx.layersPannel) ctx.layersPannel->selectLayerFromOutside(layerId);
            else canvas->selectLayer(layerId);
        }

        if (dynamic_cast<Figure*>(obj) || dynamic_cast<TextObject*>(obj) || dynamic_cast<ImageObject*>(obj)) {
            canvas->setFiltersInteractionActive(false);

            m_transform_box = new TransformBox(item, ctx.undoStack);
            ctx.scene->addItem(m_transform_box);
            onViewScaleChanged(ctx);

            connect(m_transform_box, &TransformBox::interactionEnded, this, [this, ctx]() {
                updateContextPanel(ctx);
            });
        }
    } else {
        if (canvas) canvas->setFiltersInteractionActive(true);
    }
    updateContextPanel(ctx);

    m_updating_selection = false; // Сбрасываем флаг в конце
}

void PointerTool::onViewScaleChanged(const WorkspaceContext& ctx) {
    if (m_transform_box && ctx.view) {
        m_transform_box->setViewScale(ctx.view->transform().m11());
    }
}

void PointerTool::onObjectModified(const WorkspaceContext& ctx) {
    if (m_transform_box) m_transform_box->syncPosition();
}

bool PointerTool::mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (event->button() != Qt::LeftButton) return false;

    // Игнорируем клики по самому TransformBox (он обрабатывается сам)
    for (QGraphicsItem* it : ctx.view->items(event->pos())) {
        if (dynamic_cast<TransformBox*>(it)) return false;
    }

    Object* targetObj = nullptr;
    for (QGraphicsItem* it : ctx.view->items(event->pos())) {
        targetObj = dynamic_cast<Object*>(it);
        if (!targetObj && it->parentItem()) targetObj = dynamic_cast<Object*>(it->parentItem());
        if (targetObj) break;
    }

    if (targetObj) {
        if (!targetObj->isSelected()) {
            ctx.scene->clearSelection();
            targetObj->setSelected(true);
        }
    } else {
        ctx.scene->clearSelection();
    }

    if ((m_drag_target = dynamic_cast<Figure*>(targetObj))) m_drag_start_state = m_drag_target->getState();
    if ((m_drag_target_text = dynamic_cast<TextObject*>(targetObj))) m_drag_start_text_state = m_drag_target_text->getState();
    if ((m_drag_target_image = dynamic_cast<ImageObject*>(targetObj))) m_drag_start_image_state = m_drag_target_image->getState();

    // ВОЗВРАЩАЕМ false, ЧТОБЫ QGRAPHICSVIEW САМ НАЧАЛ ПЕРЕТАСКИВАНИЕ ОБЪЕКТА
    return false;
}

bool PointerTool::mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (m_transform_box && !m_transform_box->isInteracting()) {
        m_transform_box->syncPosition();
    }
    return false;
}

bool PointerTool::mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    bool handledByTransformBox = (m_transform_box && m_transform_box->isInteracting());

    if (m_drag_target && !handledByTransformBox && m_drag_target->getState() != m_drag_start_state) {
        ctx.undoStack->push(new ModifyFigureCommand(m_drag_target, m_drag_start_state, m_drag_target->getState()));
        updateContextPanel(ctx);
    }
    if (m_drag_target_text && !handledByTransformBox && m_drag_target_text->getState() != m_drag_start_text_state) {
        ctx.undoStack->push(new ModifyTextCommand(m_drag_target_text, m_drag_start_text_state, m_drag_target_text->getState()));
        updateContextPanel(ctx);
    }
    if (m_drag_target_image && !handledByTransformBox && m_drag_target_image->getState() != m_drag_start_image_state) {
        ctx.undoStack->push(new ModifyImageCommand(m_drag_target_image, m_drag_start_image_state, m_drag_target_image->getState()));
        updateContextPanel(ctx);
    }

    m_drag_target = nullptr;
    m_drag_target_text = nullptr;
    m_drag_target_image = nullptr;

    // ЗАСТАВЛЯЕМ РАМКУ ДОГНАТЬ ОБЪЕКТ ПОСЛЕ ЗАВЕРШЕНИЯ ПЕРЕТАСКИВАНИЯ
    onObjectModified(ctx);

    // ОБЯЗАТЕЛЬНО FALSE, ЧТОБЫ QGRAPHICSVIEW КОРРЕКТНО ВЫШЕЛ ИЗ РЕЖИМА ПЕРЕТАСКИВАНИЯ (ИСПРАВЛЯЕТ ПРЫЖОК В 0,0)
    return false;
}

bool PointerTool::mouseDoubleClickEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    TextObject* txt = nullptr;
    for (QGraphicsItem* it : ctx.view->items(event->pos())) {
        txt = dynamic_cast<TextObject*>(it);
        if (!txt && it->parentItem()) txt = dynamic_cast<TextObject*>(it->parentItem());
        if (txt) break;
    }

    if (txt) {
        bool ok;
        QString newText = QInputDialog::getMultiLineText(ctx.view, "Edit Text", "Enter text:", txt->getState().text, &ok);
        if (ok && !newText.isEmpty()) {
            TextState newState = txt->getState(); // Копируем старое состояние
            newState.text = newText;              // Меняем только текст
            ctx.undoStack->push(new ModifyTextCommand(txt, txt->getState(), newState));
        }
        return true;
    }
    return false;
}

bool PointerTool::keyPressEvent(QKeyEvent* event, const WorkspaceContext& ctx) {
    // Движение стрелочками
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down || event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
        QList<QGraphicsItem*> selected = ctx.scene->selectedItems();
        if (selected.isEmpty()) return false;

        qreal step = 1.0 / ctx.view->transform().m11();
        QPointF delta(0, 0);
        if (event->key() == Qt::Key_Up) delta.setY(-step);
        if (event->key() == Qt::Key_Down) delta.setY(step);
        if (event->key() == Qt::Key_Left) delta.setX(-step);
        if (event->key() == Qt::Key_Right) delta.setX(step);

        Object* obj = dynamic_cast<Object*>(selected.first());
        if (Figure* fig = dynamic_cast<Figure*>(obj)) {
            FigureState s = fig->getState(); s.pos += delta;
            ctx.undoStack->push(new ModifyFigureCommand(fig, fig->getState(), s));
            onObjectModified(ctx);              // <--- ДОБАВИТЬ СИНХРОНИЗАЦИЮ РАМКИ
            ctx.contextPannel->setTarget(fig);  // <--- ОБНОВИТЬ ЦИФРЫ В ПАНЕЛИ
            return true;
        } else if (TextObject* txt = dynamic_cast<TextObject*>(obj)) {
            TextState s = txt->getState(); s.pos += delta;
            ctx.undoStack->push(new ModifyTextCommand(txt, txt->getState(), s));
            onObjectModified(ctx);              // <--- ДОБАВИТЬ СИНХРОНИЗАЦИЮ РАМКИ
            ctx.contextPannel->setTarget(txt);  // <--- ОБНОВИТЬ ЦИФРЫ В ПАНЕЛИ
            return true;
        }
    }
    return false;
}

// ==================== HAND TOOL ====================
void HandTool::onActivate(const WorkspaceContext& ctx) { ctx.view->setCursor(Qt::OpenHandCursor); }
void HandTool::onDeactivate(const WorkspaceContext& ctx) { ctx.view->setCursor(Qt::ArrowCursor); }

bool HandTool::mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        m_is_panning = true;
        m_last_pan_pos = event->pos();
        ctx.view->setCursor(Qt::ClosedHandCursor);
        return true;
    }
    return false;
}
bool HandTool::mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (m_is_panning) {
        QPoint delta = event->pos() - m_last_pan_pos;
        ctx.view->horizontalScrollBar()->setValue(ctx.view->horizontalScrollBar()->value() - delta.x());
        ctx.view->verticalScrollBar()->setValue(ctx.view->verticalScrollBar()->value() - delta.y());
        m_last_pan_pos = event->pos();
        return true;
    }
    return false;
}
bool HandTool::mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    m_is_panning = false;
    ctx.view->setCursor(Qt::OpenHandCursor);
    return true;
}

// ==================== FIGURE TOOL ====================
void FigureTool::onActivate(const WorkspaceContext& ctx) {
    ctx.view->setCursor(Qt::CrossCursor);
    ctx.contextPannel->setMode(false, false, true, false, "Figure");
}

bool FigureTool::mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (event->button() != Qt::LeftButton) return false;
    Canvas* canvas = ctx.projectManager->GetCurrentCanvas();
    if (!canvas) return false;

    int activeId = canvas->getSelectedLayerid();
    if (activeId >= 0 && !canvas->getLayersInfo()[activeId].locked && !canvas->getLayersInfo()[activeId].isFilter) {
        m_is_drawing = true;
        m_draw_start_pos = ctx.view->mapToScene(event->pos());
        ctx.scene->clearSelection();

        m_temp_figure = new Figure();
        FigureState defState = ctx.contextPannel->getDefaultState();
        defState.pos = m_draw_start_pos;
        defState.rect = QRectF(0,0,0,0);
        m_temp_figure->setState(defState);
        canvas->addObjectToSelectedLayer(m_temp_figure);
        return true;
    }
    return false;
}

bool FigureTool::mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (m_is_drawing && m_temp_figure) {
        QPointF cPos = ctx.view->mapToScene(event->pos());
        QRectF nRect = QRectF(m_draw_start_pos, cPos).normalized();
        m_temp_figure->setPos(nRect.center());
        m_temp_figure->setLocalRect(QRectF(-nRect.width()/2.0, -nRect.height()/2.0, nRect.width(), nRect.height()));
        return true;
    }
    return false;
}

bool FigureTool::mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (m_is_drawing && event->button() == Qt::LeftButton) {
        m_is_drawing = false;
        if (m_temp_figure) {
            QRectF r = m_temp_figure->getLocalRect();
            if (r.width() < 3.0 || r.height() < 3.0) {
                delete m_temp_figure;
            } else {
                ctx.undoStack->push(new AddObjectCommand(m_temp_figure->parentItem(), m_temp_figure));
                m_temp_figure->setSelected(true); // Передаст управление PointerTool
            }
            m_temp_figure = nullptr;
        }
        return true;
    }
    return false;
}

// ==================== TEXT TOOL ====================
void TextTool::onActivate(const WorkspaceContext& ctx) {
    ctx.view->setCursor(Qt::IBeamCursor);
    ctx.contextPannel->setMode(false, false, false, true, "Text");
}

bool TextTool::mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (event->button() != Qt::LeftButton) return false;
    Canvas* canvas = ctx.projectManager->GetCurrentCanvas();
    if (!canvas) return false;

    int activeId = canvas->getSelectedLayerid();
    if (activeId >= 0 && !canvas->getLayersInfo()[activeId].locked && !canvas->getLayersInfo()[activeId].isFilter) {
        m_is_drawing = true;
        m_draw_start_pos = ctx.view->mapToScene(event->pos());
        ctx.scene->clearSelection();

        m_temp_text = new TextObject();
        TextState defState;
        defState.pos = m_draw_start_pos;
        defState.font = ctx.contextPannel->getUITextState(defState).font;
        defState.color = ctx.contextPannel->getActiveColor();
        m_temp_text->setState(defState);
        canvas->addObjectToSelectedLayer(m_temp_text);
        return true;
    }
    return false;
}

bool TextTool::mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (m_is_drawing && m_temp_text) {
        QPointF cPos = ctx.view->mapToScene(event->pos());
        QRectF nRect = QRectF(m_draw_start_pos, cPos).normalized();
        m_temp_text->setPos(nRect.center());
        m_temp_text->setLocalRect(QRectF(-nRect.width()/2.0, -nRect.height()/2.0, nRect.width(), nRect.height()));
        return true;
    }
    return false;
}

bool TextTool::mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (m_is_drawing && event->button() == Qt::LeftButton) {
        m_is_drawing = false;
        if (m_temp_text) {
            QRectF r = m_temp_text->getLocalRect();
            if (r.width() < 15.0 || r.height() < 10.0) {
                delete m_temp_text;
            } else {
                bool ok;
                QString newText = QInputDialog::getMultiLineText(ctx.view, "New Text", "Enter text:", "Text", &ok);
                if (ok && !newText.isEmpty()) {
                    TextState s = m_temp_text->getState();
                    s.text = newText;
                    m_temp_text->setState(s);
                    ctx.undoStack->push(new AddObjectCommand(m_temp_text->parentItem(), m_temp_text));
                    m_temp_text->setSelected(true);
                } else {
                    delete m_temp_text;
                }
            }
            m_temp_text = nullptr;
        }
        return true;
    }
    return false;
}

// ==================== PENCIL TOOL (STUB) ====================
void PencilTool::onActivate(const WorkspaceContext& ctx) {
    ctx.view->setCursor(Qt::CrossCursor);
    ctx.contextPannel->setMode(false, false, false, false, "Pencil (Raster)");
}

bool PencilTool::mousePressEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (event->button() != Qt::LeftButton) return false;

    // Пытаемся растрировать слой
    if (ctx.controller && ctx.controller->tryRasterizeLayer()) {
        m_is_drawing = true;
        // Здесь в Этапе 2 мы начнем рисовать по QImage
        qDebug() << "Layer is ready for raster drawing!";
        return true;
    }
    return false;
}

bool PencilTool::mouseMoveEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (m_is_drawing) {
        // Здесь будет рисование линии в Этапе 2
        return true;
    }
    return false;
}

bool PencilTool::mouseReleaseEvent(QMouseEvent* event, const WorkspaceContext& ctx) {
    if (m_is_drawing && event->button() == Qt::LeftButton) {
        m_is_drawing = false;
        // Здесь будет сохранение "грязного прямоугольника" в Undo-стек
        return true;
    }
    return false;
}