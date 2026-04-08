#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <qmath.h>

#include "include/workspacecontroller.h"
#include "include/action.h"
#include "include/raster_action.h"
#include "include/projectmanager.h"
#include "include/contextpannel.h"
#include "include/palettepannel.h"
#include "include/layerspannel.h"

WorkspaceController::WorkspaceController(const WorkspaceContext& ctx, QObject* parent)
    : QObject(parent), m_context(ctx)
{
    m_undo_stack = new QUndoStack(this);
    m_context.undoStack = m_undo_stack;
    m_context.controller = this;

    m_tools[InstrumentType::POINTER].reset(new PointerTool(this));
    m_tools[InstrumentType::HAND].reset(new HandTool(this));
    m_tools[InstrumentType::FIGURE].reset(new FigureTool(this));
    m_tools[InstrumentType::TEXT].reset(new TextTool(this));
    m_tools[InstrumentType::PENCIL].reset(new PencilTool(this));
    m_tools[InstrumentType::ERASER].reset(new EraserTool(this));
    m_tools[InstrumentType::FILL].reset(new FillTool(this));
    m_tools[InstrumentType::PIPETTE].reset(new PipetteTool(this));

    if (m_context.view) {
        m_context.view->installEventFilter(this);
        m_context.view->viewport()->installEventFilter(this);
    }

    if (m_context.scene)
        connect(m_context.scene, &QGraphicsScene::selectionChanged, this, &WorkspaceController::onSelectionChanged);

    if (m_context.contextPannel) {
        connect(m_context.contextPannel, &ContextPannel::propertyChanged, this, &WorkspaceController::onContextPropertyChanged);
        connect(m_context.contextPannel, &ContextPannel::colorTargetActivated, this, &WorkspaceController::onColorTargetChanged);
        connect(m_context.contextPannel, &ContextPannel::moveObjectLayerRequested, this, &WorkspaceController::onMoveObjectLayerRequested);

        connect(m_context.contextPannel, &ContextPannel::rasterSettingsChanged, this, [this](){
            if (m_active_tool) m_active_tool->onRasterSettingsChanged(m_context);
        });
    }

    if (m_context.palettePannel) {
        connect(m_context.palettePannel, &PalettePannel::colorPreviewed, this, &WorkspaceController::onColorPickedPreview);
        connect(m_context.palettePannel, &PalettePannel::colorCommitted, this, &WorkspaceController::onColorPickedCommit);
    }

    connect(m_undo_stack, &QUndoStack::indexChanged, this, [this](){
        if (Canvas* c = m_context.projectManager->GetCurrentCanvas()) c->updateFilters();
    });

    if (m_context.projectManager) {
        connect(m_context.projectManager->GetCurrentCanvas(), &Canvas::activeLayerChanged, this, &WorkspaceController::onActiveLayerChanged);
    }
}

void WorkspaceController::setCurrentTool(InstrumentType type) {
    if (m_active_tool) m_active_tool->onDeactivate(m_context);

    if (m_current_tool_type != type) {
        m_previous_tool_type = m_current_tool_type;
    }

    m_current_tool_type = type;
    if (m_tools.find(type) != m_tools.end()) {
        m_active_tool = m_tools[type].get();
    } else {
        m_active_tool = m_tools[InstrumentType::POINTER].get();
    }

    // Синхронизация UI
    if (m_context.instrumentPannel) {
        m_context.instrumentPannel->setActiveTool(type);
    }

    QString toolName;
    switch(type) {
    case InstrumentType::POINTER: toolName = "Pointer"; break;
    case InstrumentType::HAND: toolName = "Hand"; break;
    case InstrumentType::FIGURE: toolName = "Figure"; break;
    case InstrumentType::TEXT: toolName = "Text"; break;
    case InstrumentType::PENCIL: toolName = "Pencil"; break;
    case InstrumentType::ERASER: toolName = "Eraser"; break;
    case InstrumentType::FILL: toolName = "Fill Bucket"; break;
    case InstrumentType::PIPETTE: toolName = "Pipette"; break;
    default: toolName = "Unknown";
    }
    emit statusMessage(QString("Tool selected: %1").arg(toolName));

    if (m_active_tool) m_active_tool->onActivate(m_context);
}

void WorkspaceController::revertToPreviousTool() {
    setCurrentTool(m_previous_tool_type);
}

void WorkspaceController::updateTransformBoxScale() {
    if (m_active_tool) m_active_tool->onViewScaleChanged(m_context);
}

bool WorkspaceController::eventFilter(QObject *obj, QEvent *event) {
    if (!m_context.view || !m_context.projectManager) return QObject::eventFilter(obj, event);

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kEvent = static_cast<QKeyEvent *>(event);
        if (kEvent->key() == Qt::Key_Space && !kEvent->isAutoRepeat() && m_current_tool_type != InstrumentType::HAND) {
            setCurrentTool(InstrumentType::HAND);
            return true;
        }
        if (kEvent->key() == Qt::Key_Alt && !kEvent->isAutoRepeat() && m_current_tool_type != InstrumentType::PIPETTE) {
            setCurrentTool(InstrumentType::PIPETTE);
            return true;
        }
    } else if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *kEvent = static_cast<QKeyEvent *>(event);
        if (kEvent->key() == Qt::Key_Space && !kEvent->isAutoRepeat() && m_current_tool_type == InstrumentType::HAND) {
            revertToPreviousTool();
            return true;
        }
        if (kEvent->key() == Qt::Key_Alt && !kEvent->isAutoRepeat() && m_current_tool_type == InstrumentType::PIPETTE) {
            revertToPreviousTool();
            return true;
        }
    }

    if (obj == m_context.view->viewport() || obj == m_context.view) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wEvent = static_cast<QWheelEvent *>(event);
            double delta = wEvent->angleDelta().y();
            if (delta == 0) return true;

            m_context.view->setTransformationAnchor(QGraphicsView::NoAnchor);
            QPointF targetScenePos = m_context.view->mapToScene(wEvent->pos());

            double factor = qPow(1.15, delta / 120.0);

            Canvas* canvas = m_context.projectManager->GetCurrentCanvas();
            double cw = canvas ? canvas->getSize().width() : 800;
            double ch = canvas ? canvas->getSize().height() : 600;
            double vw = m_context.view->viewport()->width();
            double vh = m_context.view->viewport()->height();

            double minScaleW = vw / (10.0 * qMax(1.0, cw));
            double minScaleH = vh / (10.0 * qMax(1.0, ch));
            double minScale = qMin(minScaleW, minScaleH);
            if (minScale <= 0) minScale = 0.01;
            double maxScale = 150.0;

            double currentScale = m_context.view->transform().m11();
            double newScale = currentScale * factor;

            if (newScale < minScale) factor = minScale / currentScale;
            if (newScale > maxScale) factor = maxScale / currentScale;

            m_context.view->scale(factor, factor);

            QPointF newScenePos = m_context.view->mapToScene(wEvent->pos());
            QPointF moveDelta = newScenePos - targetScenePos;
            m_context.view->translate(moveDelta.x(), moveDelta.y());

            updateTransformBoxScale();
            emit viewportChanged();
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress && m_active_tool) {
            if (m_active_tool->mousePressEvent(static_cast<QMouseEvent*>(event), m_context)) return true;
        }
        else if (event->type() == QEvent::MouseMove && m_active_tool) {
            if (m_active_tool->mouseMoveEvent(static_cast<QMouseEvent*>(event), m_context)) return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease && m_active_tool) {
            if (m_active_tool->mouseReleaseEvent(static_cast<QMouseEvent*>(event), m_context)) return true;
        }
        else if (event->type() == QEvent::MouseButtonDblClick && m_active_tool) {
            if (m_active_tool->mouseDoubleClickEvent(static_cast<QMouseEvent*>(event), m_context)) return true;
        }
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kEvent = static_cast<QKeyEvent *>(event);
        if (kEvent->modifiers() & Qt::ControlModifier) {
            handleGlobalShortcuts(kEvent);
            return true;
        }
        if (kEvent->key() == Qt::Key_Delete && !m_context.scene->selectedItems().isEmpty()) {
            QGraphicsItem* selected = m_context.scene->selectedItems().first();
            m_undo_stack->push(new DeleteObjectCommand(selected->parentItem(), selected));
            return true;
        }
        if (m_active_tool && m_active_tool->keyPressEvent(kEvent, m_context)) return true;
    }

    if (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove) {
        QDragMoveEvent *dEvent = static_cast<QDragMoveEvent*>(event);
        if (dEvent->mimeData()->hasImage() || dEvent->mimeData()->hasUrls()) {
            dEvent->acceptProposedAction(); return true;
        }
    }
    if (event->type() == QEvent::Drop) {
        handleDrop(static_cast<QDropEvent*>(event));
        return true;
    }

    return QObject::eventFilter(obj, event);
}

void WorkspaceController::handleGlobalShortcuts(QKeyEvent* kEvent) {
    if (kEvent->key() == Qt::Key_Z) { m_context.scene->clearSelection(); m_undo_stack->undo(); }
    else if (kEvent->key() == Qt::Key_Y) { m_context.scene->clearSelection(); m_undo_stack->redo(); }
    else if (kEvent->key() == Qt::Key_C || kEvent->key() == Qt::Key_X) {
        if (!m_context.scene->selectedItems().isEmpty()) {
            QGraphicsItem* item = m_context.scene->selectedItems().first();
            if (Figure* fig = dynamic_cast<Figure*>(item)) {
                m_clipboard_figure = fig->getState(); m_clipboard_type = ClipboardType::Figure;
            } else if (TextObject* txt = dynamic_cast<TextObject*>(item)) {
                m_clipboard_text = txt->getState(); m_clipboard_type = ClipboardType::Text;
            } else if (ImageObject* img = dynamic_cast<ImageObject*>(item)) {
                m_clipboard_image = img->getState(); m_clipboard_type = ClipboardType::Image;
            }
            if (kEvent->key() == Qt::Key_X) m_undo_stack->push(new DeleteObjectCommand(item->parentItem(), item));
            QApplication::clipboard()->clear();
        }
    }
    else if (kEvent->key() == Qt::Key_V) handlePaste();
}

void WorkspaceController::handlePaste() {
    Canvas* canvas = m_context.projectManager->GetCurrentCanvas();
    if (!canvas) return;
    int activeId = canvas->getSelectedLayerid();
    if (activeId < 0 || canvas->getLayersInfo()[activeId].locked || canvas->getLayersInfo()[activeId].isFilter) return;

    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QImage img;

    if (mimeData->hasImage()) {
        img = qvariant_cast<QImage>(mimeData->imageData());
    } else if (mimeData->hasUrls()) {
        for (const QUrl &url : mimeData->urls()) {
            if (url.isLocalFile()) {
                QImage temp(url.toLocalFile());
                if (!temp.isNull()) { img = temp; break; }
            }
        }
    }

    if (!img.isNull()) {
        m_context.scene->clearSelection();
        ImageObject* imgObj = new ImageObject();
        ImageState s; s.image = img;
        s.pos = m_context.view->mapToScene(m_context.view->viewport()->rect().center());
        s.rect = QRectF(-img.width()/2.0, -img.height()/2.0, img.width(), img.height());
        imgObj->setState(s);
        canvas->addObjectToSelectedLayer(imgObj);
        m_undo_stack->push(new AddObjectCommand(imgObj->parentItem(), imgObj));
        imgObj->setSelected(true);
        return;
    }

    if (m_clipboard_type != ClipboardType::None) {
        m_context.scene->clearSelection();
        QPointF centerPos = m_context.view->mapToScene(m_context.view->viewport()->rect().center());
        if (m_clipboard_type == ClipboardType::Figure) {
            Figure* fig = new Figure(); m_clipboard_figure.pos = centerPos;
            fig->setState(m_clipboard_figure); canvas->addObjectToSelectedLayer(fig);
            m_undo_stack->push(new AddObjectCommand(fig->parentItem(), fig)); fig->setSelected(true);
        } else if (m_clipboard_type == ClipboardType::Text) {
            TextObject* txt = new TextObject(); m_clipboard_text.pos = centerPos;
            txt->setState(m_clipboard_text); canvas->addObjectToSelectedLayer(txt);
            m_undo_stack->push(new AddObjectCommand(txt->parentItem(), txt)); txt->setSelected(true);
        } else if (m_clipboard_type == ClipboardType::Image) {
            ImageObject* imgObj = new ImageObject(); m_clipboard_image.pos = centerPos;
            imgObj->setState(m_clipboard_image); canvas->addObjectToSelectedLayer(imgObj);
            m_undo_stack->push(new AddObjectCommand(imgObj->parentItem(), imgObj)); imgObj->setSelected(true);
        }
    }
}

void WorkspaceController::handleDrop(QDropEvent* event) {
    Canvas* canvas = m_context.projectManager->GetCurrentCanvas();
    if (!canvas) return;
    int activeId = canvas->getSelectedLayerid();
    if (activeId >= 0 && !canvas->getLayersInfo()[activeId].locked && !canvas->getLayersInfo()[activeId].isFilter) {
        QImage img;
        if (event->mimeData()->hasImage()) {
            img = qvariant_cast<QImage>(event->mimeData()->imageData());
        } else if (event->mimeData()->hasUrls()) {
            for (const QUrl &url : event->mimeData()->urls()) {
                if (url.isLocalFile()) {
                    QImage temp(url.toLocalFile());
                    if (!temp.isNull()) { img = temp; break; }
                }
            }
        }
        if (!img.isNull()) {
            QPointF scenePos = m_context.view->mapToScene(event->pos());
            ImageObject* imgObj = new ImageObject();
            ImageState s; s.image = img; s.pos = scenePos;
            s.rect = QRectF(-img.width()/2.0, -img.height()/2.0, img.width(), img.height());
            imgObj->setState(s);
            m_context.scene->clearSelection();
            canvas->addObjectToSelectedLayer(imgObj);
            m_undo_stack->push(new AddObjectCommand(imgObj->parentItem(), imgObj));
            imgObj->setSelected(true);
        }
    }
    event->acceptProposedAction();
}

void WorkspaceController::onSelectionChanged() {
    if (m_active_tool) m_active_tool->onSelectionChanged(m_context);
}

void WorkspaceController::onActiveLayerChanged(int id) {
    Canvas* canvas = m_context.projectManager->GetCurrentCanvas();
    if (!canvas) return;

    if (id >= 0 && canvas->getLayers()[id]->isFilter()) {
        m_context.scene->clearSelection();
        m_context.contextPannel->setTarget(static_cast<FilterLayer*>(canvas->getLayers()[id]));
    } else {
        setCurrentTool(m_current_tool_type);
        onSelectionChanged();
    }
}

void WorkspaceController::onContextPropertyChanged() {
    Canvas* canvas = m_context.projectManager->GetCurrentCanvas();
    if (canvas && canvas->getSelectedLayerid() >= 0) {
        Layer* activeLayer = canvas->getLayers()[canvas->getSelectedLayerid()];
        if (activeLayer->isFilter()) {
            FilterLayer* filter = static_cast<FilterLayer*>(activeLayer);
            FilterState oldState = filter->getFilterState();
            FilterState newState = m_context.contextPannel->getUIFilterState();
            if (oldState != newState) m_undo_stack->push(new ModifyFilterCommand(filter, oldState, newState));
            return;
        }
    }

    QList<QGraphicsItem*> selected = m_context.scene->selectedItems();
    if (selected.isEmpty()) return;
    Object* obj = dynamic_cast<Object*>(selected.first());

    if (Figure* fig = dynamic_cast<Figure*>(obj)) {
        FigureState oldState = fig->getState();
        FigureState newState = m_context.contextPannel->getUIState(oldState);
        if (oldState != newState) {
            m_undo_stack->push(new ModifyFigureCommand(fig, oldState, newState));
            m_context.contextPannel->setTarget(fig);
        }
    } else if (TextObject* txt = dynamic_cast<TextObject*>(obj)) {
        TextState oldState = txt->getState();
        TextState newState = m_context.contextPannel->getUITextState(oldState);
        if (oldState != newState) {
            m_undo_stack->push(new ModifyTextCommand(txt, oldState, newState));
            m_context.contextPannel->setTarget(txt);
        }
    } else if (ImageObject* img = dynamic_cast<ImageObject*>(obj)) {
        ImageState oldState = img->getState();
        ImageState newState = m_context.contextPannel->getUIImageState(oldState);
        if (oldState != newState) {
            m_undo_stack->push(new ModifyImageCommand(img, oldState, newState));
            m_context.contextPannel->setTarget(img);
        }
    }

    if (m_active_tool) m_active_tool->onObjectModified(m_context);
}

void WorkspaceController::onColorTargetChanged(bool isFill) {
    m_color_target_is_fill = isFill;
    m_context.palettePannel->setColor(m_context.contextPannel->getActiveColor());
}

void WorkspaceController::onColorPickedPreview(const QColor& color) {
    QList<QGraphicsItem*> selected = m_context.scene->selectedItems();
    if (!selected.isEmpty()) {

        if (Figure* fig = dynamic_cast<Figure*>(selected.first())) {
            if (!m_is_previewing) { m_state_before_preview = fig->getState(); m_is_previewing = true; }
            FigureState s = fig->getState(); if (m_color_target_is_fill) s.fill = color; else s.stroke = color;
            fig->setState(s);
        } else if (TextObject* txt = dynamic_cast<TextObject*>(selected.first())) {
            if (!m_is_previewing) { m_text_state_before_preview = txt->getState(); m_is_previewing = true; }
            TextState s = txt->getState(); s.color = color; txt->setState(s);
        }
    } else m_context.contextPannel->setDefaultColor(m_color_target_is_fill, color);
    if (m_active_tool) m_active_tool->onColorChanged(color, m_context);
}

void WorkspaceController::onColorPickedCommit(const QColor& color) {
    QList<QGraphicsItem*> selected = m_context.scene->selectedItems();
    if (!selected.isEmpty()) {
        if (Figure* fig = dynamic_cast<Figure*>(selected.first())) {
            FigureState newState = fig->getState();
            if (m_color_target_is_fill) newState.fill = color; else newState.stroke = color;

            if (m_is_previewing) { fig->setState(m_state_before_preview); m_is_previewing = false; }
            m_undo_stack->push(new ModifyFigureCommand(fig, fig->getState(), newState));
        } else if (TextObject* txt = dynamic_cast<TextObject*>(selected.first())) {
            TextState newState = txt->getState();
            newState.color = color;

            if (m_is_previewing) { txt->setState(m_text_state_before_preview); m_is_previewing = false; }
            m_undo_stack->push(new ModifyTextCommand(txt, txt->getState(), newState));
        }
    } else m_context.contextPannel->setDefaultColor(m_color_target_is_fill, color);
    if (m_active_tool) m_active_tool->onColorChanged(color, m_context);
}

void WorkspaceController::onMoveObjectLayerRequested(int shift) {
    QList<QGraphicsItem*> selected = m_context.scene->selectedItems();
    if (selected.isEmpty()) return;
    Object* target = dynamic_cast<Object*>(selected.first());
    Canvas* canvas = m_context.projectManager->GetCurrentCanvas();
    if (!canvas || !target) return;

    int oldLayerId = canvas->getLayerIdOfObject(target);
    int newLayerId = oldLayerId + shift;
    std::vector<LayerInfo> infos = canvas->getLayersInfo();

    while (newLayerId >= 0 && newLayerId < (int)infos.size() && (infos[newLayerId].locked || infos[newLayerId].isFilter)) {
        newLayerId += (shift > 0) ? 1 : -1;
    }
    if (newLayerId >= 0 && newLayerId < (int)infos.size() && newLayerId != oldLayerId) {
        m_undo_stack->push(new MoveObjectLayerCommand(canvas, target, oldLayerId, newLayerId));
        m_context.layersPannel->selectLayerFromOutside(newLayerId);
    }
}

void WorkspaceController::clearState() {
    if (m_active_tool) m_active_tool->onDeactivate(m_context);

    m_context.scene->clearSelection();
    if (m_undo_stack) m_undo_stack->clear();

    setCurrentTool(InstrumentType::POINTER);
}

RasterizeResult WorkspaceController::prepareRasterLayer() {
    Canvas* canvas = m_context.projectManager->GetCurrentCanvas();
    if (!canvas) return RasterizeResult::Cancelled;

    int activeId = canvas->getSelectedLayerid();
    if (activeId < 0) return RasterizeResult::Cancelled;

    Layer* layer = canvas->getLayers()[activeId];

    if (layer->isFilter()) {
        if (canvas->getMaskEditLayerId() == activeId) {
            if (layer->getRasterImage().isNull() || layer->getRasterImage().width() == 0) {
                QImage mask(canvas->getSize(), QImage::Format_ARGB32_Premultiplied);
                mask.fill(Qt::white);
                layer->setRasterImage(mask);
            }
            return RasterizeResult::Ready;
        }
        return RasterizeResult::Cancelled;
    }

    bool hasVectors = !layer->getObjects().empty();

    if (layer->isRasterized() && !hasVectors) return RasterizeResult::Ready;

    if (!layer->isRasterized() && !hasVectors) {
        m_undo_stack->push(new RasterizeLayerCommand(canvas, activeId));
        return RasterizeResult::RasterizedNow;
    }

    Project* proj = m_context.projectManager->getCurrentProject();
    if (proj->getAskRasterize()) {
        QDialog dlg;
        dlg.setWindowTitle("Rasterize Layer");
        QVBoxLayout* layout = new QVBoxLayout(&dlg);
        layout->addWidget(new QLabel("This layer must be rasterized to use raster tools.\nVector objects will be baked into the image."));
        QCheckBox* cb = new QCheckBox("Don't ask again");
        layout->addWidget(cb);
        QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(box);
        connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

        if (dlg.exec() != QDialog::Accepted) return RasterizeResult::Cancelled;
        if (cb->isChecked()) proj->setAskRasterize(false);
    }

    m_undo_stack->push(new RasterizeLayerCommand(canvas, activeId));
    m_context.scene->clearSelection();

    return RasterizeResult::RasterizedNow;
}