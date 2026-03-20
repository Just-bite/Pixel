#include "workspacecontroller.h"
#include "action.h"
#include <QApplication>
#include <QKeyEvent>
#include <QScrollBar>
#include <QClipboard>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>

WorkspaceController::WorkspaceController(QGraphicsView* view, QGraphicsScene* scene, ProjectManager* pm, ContextPannel* cp, PalettePannel* pp, LayersPannel* lp, QObject *parent)
    : QObject(parent), m_view(view), m_project_manager(pm), m_context_pannel(cp), m_palette_pannel(pp), m_layers_pannel(lp), m_last_mouse_scene_pos(0,0)
{
    m_undo_stack = new QUndoStack(this);
    if (m_view) {
        m_view->installEventFilter(this);
        m_view->viewport()->installEventFilter(this);
    }
    if (scene) connect(scene, &QGraphicsScene::selectionChanged, this, &WorkspaceController::onSelectionChanged);

    if (m_context_pannel) {
        connect(m_context_pannel, &ContextPannel::propertyChanged, this, &WorkspaceController::onContextPropertyChanged);
        connect(m_context_pannel, &ContextPannel::colorTargetActivated, this, &WorkspaceController::onColorTargetChanged);
        connect(m_context_pannel, &ContextPannel::moveObjectLayerRequested, this, &WorkspaceController::onMoveObjectLayerRequested);
    }
    if (m_palette_pannel) {
        connect(m_palette_pannel, &PalettePannel::colorPreviewed, this, &WorkspaceController::onColorPickedPreview);
        connect(m_palette_pannel, &PalettePannel::colorCommitted, this, &WorkspaceController::onColorPickedCommit);
    }
}

QString WorkspaceController::getToolName(InstrumentType type) {
    if (type == InstrumentType::POINTER) return "Pointer";
    if (type == InstrumentType::HAND) return "Hand";
    if (type == InstrumentType::FIGURE) return "Figure";
    return "Unknown Tool";
}

void WorkspaceController::setCurrentTool(InstrumentType type) {
    m_current_tool = type;
    if (m_current_tool == InstrumentType::HAND) m_view->setCursor(Qt::OpenHandCursor);
    else if (m_current_tool == InstrumentType::FIGURE) m_view->setCursor(Qt::CrossCursor);
    else m_view->setCursor(Qt::ArrowCursor);

    m_context_pannel->setMode(m_selected_figure != nullptr, m_current_tool == InstrumentType::FIGURE, getToolName(type));
}

void WorkspaceController::updateTransformBoxScale() {
    if (m_transform_box && m_view) {
        m_transform_box->setViewScale(m_view->transform().m11());
    }
}

void WorkspaceController::clearTransformBox() {
    if (m_transform_box) { m_transform_box->setParentItem(nullptr); if (m_view->scene()) m_view->scene()->removeItem(m_transform_box); delete m_transform_box; m_transform_box = nullptr; }
}

bool WorkspaceController::eventFilter(QObject *obj, QEvent *event) {
    if (!m_view || !m_project_manager) return QObject::eventFilter(obj, event);
    Canvas* canvas = m_project_manager->GetCurrentCanvas();

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kEvent = static_cast<QKeyEvent *>(event);
        if (kEvent->modifiers() & Qt::ControlModifier) {
            if (kEvent->key() == Qt::Key_Z) { m_view->scene()->clearSelection(); m_undo_stack->undo(); return true; }
            if (kEvent->key() == Qt::Key_Y) { m_view->scene()->clearSelection(); m_undo_stack->redo(); return true; }
            if (kEvent->key() == Qt::Key_C && m_selected_figure) {
                m_clipboard_state = m_selected_figure->getState();
                m_has_clipboard = true;
                QApplication::clipboard()->clear(); // Очищаем системный буфер, чтобы избежать конфликтов при Ctrl+V
                return true;
            }
            if (kEvent->key() == Qt::Key_X && m_selected_figure) {
                m_clipboard_state = m_selected_figure->getState();
                m_has_clipboard = true;
                QApplication::clipboard()->clear();
                m_undo_stack->push(new DeleteObjectCommand(m_selected_figure->parentItem(), m_selected_figure));
                return true;
            }
            if (kEvent->key() == Qt::Key_V && canvas) {
                int activeId = canvas->getSelectedLayerid();
                if (activeId >= 0 && !canvas->getLayersInfo()[activeId].locked) {
                    const QClipboard *clipboard = QApplication::clipboard();
                    const QMimeData *mimeData = clipboard->mimeData();

                    // 1. Приоритет вставки картинки из ОС, если буфер обмена имеет изображение
                    if (mimeData->hasImage()) {
                        QImage img = qvariant_cast<QImage>(mimeData->imageData());
                        if (!img.isNull()) {
                            Figure* fig = new Figure();
                            FigureState s;
                            s.type = FigureType::Image;
                            s.image = img;
                            s.pos = m_last_mouse_scene_pos;
                            s.rect = QRectF(-img.width()/2.0, -img.height()/2.0, img.width(), img.height());
                            fig->setState(s);
                            m_view->scene()->clearSelection();
                            canvas->addObjectToSelectedLayer(fig);
                            m_undo_stack->push(new AddObjectCommand(fig->parentItem(), fig));
                            fig->setSelected(true);
                            return true;
                        }
                    }

                    // 2. Иначе вставляем внутренний скопированный объект
                    if (m_has_clipboard) {
                        m_view->scene()->clearSelection();
                        Figure* fig = new Figure();
                        m_clipboard_state.pos = m_last_mouse_scene_pos;
                        fig->setState(m_clipboard_state);
                        canvas->addObjectToSelectedLayer(fig);
                        m_undo_stack->push(new AddObjectCommand(fig->parentItem(), fig));
                        fig->setSelected(true);
                    }
                }
                return true;
            }
        }
        if (kEvent->key() == Qt::Key_Delete && !m_view->scene()->selectedItems().isEmpty()) {
            QGraphicsItem* selected = m_view->scene()->selectedItems().first();
            m_undo_stack->push(new DeleteObjectCommand(selected->parentItem(), selected));
            return true;
        }
        if (kEvent->key() == Qt::Key_Space && !kEvent->isAutoRepeat()) {
            m_space_pressed = true; if (!m_is_panning) m_view->setCursor(Qt::OpenHandCursor);
            return true;
        }

        // Логика перемещения объекта или камеры по стрелочкам
        if (kEvent->key() == Qt::Key_Up || kEvent->key() == Qt::Key_Down ||
            kEvent->key() == Qt::Key_Left || kEvent->key() == Qt::Key_Right) {

            if (m_selected_figure && !m_space_pressed) {
                qreal step = 1.0 / m_view->transform().m11();
                QPointF delta(0, 0);
                if (kEvent->key() == Qt::Key_Up) delta.setY(-step);
                if (kEvent->key() == Qt::Key_Down) delta.setY(step);
                if (kEvent->key() == Qt::Key_Left) delta.setX(-step);
                if (kEvent->key() == Qt::Key_Right) delta.setX(step);

                FigureState oldState = m_selected_figure->getState();
                FigureState newState = oldState;
                newState.pos += delta;

                m_undo_stack->push(new ModifyFigureCommand(m_selected_figure, oldState, newState));
                m_context_pannel->setTarget(m_selected_figure);
                return true;
            } else if (m_space_pressed) {
                int scrollStep = 15;
                if (kEvent->key() == Qt::Key_Up) m_view->verticalScrollBar()->setValue(m_view->verticalScrollBar()->value() - scrollStep);
                if (kEvent->key() == Qt::Key_Down) m_view->verticalScrollBar()->setValue(m_view->verticalScrollBar()->value() + scrollStep);
                if (kEvent->key() == Qt::Key_Left) m_view->horizontalScrollBar()->setValue(m_view->horizontalScrollBar()->value() - scrollStep);
                if (kEvent->key() == Qt::Key_Right) m_view->horizontalScrollBar()->setValue(m_view->horizontalScrollBar()->value() + scrollStep);
                return true;
            }
        }
    }

    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *kEvent = static_cast<QKeyEvent *>(event);
        if (kEvent->key() == Qt::Key_Space && !kEvent->isAutoRepeat()) {
            m_space_pressed = false; if (!m_is_panning) setCurrentTool(m_current_tool);
            return true;
        }
    }

    // Обработка Drag-and-Drop
    if (event->type() == QEvent::DragEnter) {
        QDragEnterEvent *dEvent = static_cast<QDragEnterEvent*>(event);
        if (dEvent->mimeData()->hasImage() || dEvent->mimeData()->hasUrls()) {
            dEvent->acceptProposedAction();
            return true;
        }
    }
    if (event->type() == QEvent::DragMove) {
        QDragMoveEvent *dEvent = static_cast<QDragMoveEvent*>(event);
        if (dEvent->mimeData()->hasImage() || dEvent->mimeData()->hasUrls()) {
            dEvent->acceptProposedAction();
            return true;
        }
    }
    if (event->type() == QEvent::Drop) {
        QDropEvent *dEvent = static_cast<QDropEvent*>(event);
        int activeId = canvas ? canvas->getSelectedLayerid() : -1;
        if (canvas && activeId >= 0 && !canvas->getLayersInfo()[activeId].locked) {
            QImage img;
            if (dEvent->mimeData()->hasImage()) {
                img = qvariant_cast<QImage>(dEvent->mimeData()->imageData());
            } else if (dEvent->mimeData()->hasUrls()) {
                for (const QUrl &url : dEvent->mimeData()->urls()) {
                    if (url.isLocalFile()) {
                        QImage temp(url.toLocalFile());
                        if (!temp.isNull()) { img = temp; break; }
                    }
                }
            }
            if (!img.isNull()) {
                QPointF scenePos = m_view->mapToScene(dEvent->pos());
                Figure* fig = new Figure();
                FigureState s;
                s.type = FigureType::Image;
                s.image = img;
                s.pos = scenePos;
                s.rect = QRectF(-img.width()/2.0, -img.height()/2.0, img.width(), img.height());
                fig->setState(s);
                m_view->scene()->clearSelection();
                canvas->addObjectToSelectedLayer(fig);
                m_undo_stack->push(new AddObjectCommand(fig->parentItem(), fig));
                fig->setSelected(true);
            }
        }
        dEvent->acceptProposedAction();
        return true;
    }

    if (obj == m_view->viewport() || obj == m_view) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wEvent = static_cast<QWheelEvent *>(event);
            m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            double factor = (wEvent->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
            m_view->scale(factor, factor);
            updateTransformBoxScale();
            emit viewportChanged(); return true;
        }

        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
            m_last_mouse_scene_pos = m_view->mapToScene(mEvent->pos());

            if (m_is_panning) {
                QPoint delta = mEvent->pos() - m_last_pan_pos;
                m_view->horizontalScrollBar()->setValue(m_view->horizontalScrollBar()->value() - delta.x());
                m_view->verticalScrollBar()->setValue(m_view->verticalScrollBar()->value() - delta.y());
                m_last_pan_pos = mEvent->pos(); return true;
            }
            if (m_is_drawing && m_temp_figure) {
                QPointF cPos = m_view->mapToScene(mEvent->pos());
                QRectF nRect = QRectF(m_draw_start_pos, cPos).normalized();
                m_temp_figure->setPos(nRect.center());
                m_temp_figure->setLocalRect(QRectF(-nRect.width()/2.0, -nRect.height()/2.0, nRect.width(), nRect.height()));
                return true;
            }
        }

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
            if ((mEvent->button() == Qt::LeftButton && (m_space_pressed || m_current_tool == InstrumentType::HAND)) || mEvent->button() == Qt::MiddleButton) {
                m_is_panning = true; m_last_pan_pos = mEvent->pos(); m_view->setCursor(Qt::ClosedHandCursor); return true;
            }
            if (mEvent->button() == Qt::LeftButton && m_current_tool == InstrumentType::FIGURE && canvas) {
                int activeId = canvas->getSelectedLayerid();
                if (activeId >= 0 && !canvas->getLayersInfo()[activeId].locked) {
                    m_is_drawing = true; m_draw_start_pos = m_view->mapToScene(mEvent->pos());
                    m_view->scene()->clearSelection();
                    m_temp_figure = new Figure();
                    FigureState defState = m_context_pannel->getDefaultState();
                    defState.pos = m_draw_start_pos; defState.rect = QRectF(0,0,0,0);
                    m_temp_figure->setState(defState);
                    canvas->addObjectToSelectedLayer(m_temp_figure);
                }
                return true;
            }

            if (mEvent->button() == Qt::LeftButton && m_current_tool == InstrumentType::POINTER) {
                QGraphicsItem* item = m_view->itemAt(mEvent->pos());
                Figure* fig = dynamic_cast<Figure*>(item);
                if (!fig && item && item->parentItem()) {
                    fig = dynamic_cast<Figure*>(item->parentItem());
                }
                if (fig) {
                    m_drag_target = fig;
                    m_drag_start_state = fig->getState();
                } else {
                    m_drag_target = nullptr;
                }
            }
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
            if (m_is_panning) { m_is_panning = false; setCurrentTool(m_current_tool); return true; }
            if (m_is_drawing && mEvent->button() == Qt::LeftButton) {
                m_is_drawing = false;
                if (m_temp_figure) {
                    QRectF r = m_temp_figure->getLocalRect();
                    if (r.width() < 3.0 || r.height() < 3.0) {
                        m_temp_figure->setParentItem(nullptr); if (m_temp_figure->scene()) m_temp_figure->scene()->removeItem(m_temp_figure); delete m_temp_figure;
                    } else { m_undo_stack->push(new AddObjectCommand(m_temp_figure->parentItem(), m_temp_figure)); m_temp_figure->setSelected(true); }
                    m_temp_figure = nullptr;
                }
                return true;
            }

            if (mEvent->button() == Qt::LeftButton && m_current_tool == InstrumentType::POINTER) {
                if (m_drag_target) {
                    FigureState newState = m_drag_target->getState();
                    bool handledByTransformBox = (m_transform_box && m_transform_box->isInteracting());

                    if (!handledByTransformBox && newState != m_drag_start_state) {
                        m_undo_stack->push(new ModifyFigureCommand(m_drag_target, m_drag_start_state, newState));

                        if (m_drag_target == m_selected_figure) {
                            m_context_pannel->setTarget(m_selected_figure);
                        }
                    }
                    m_drag_target = nullptr;
                }
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void WorkspaceController::onSelectionChanged() {
    clearTransformBox();
    QList<QGraphicsItem*> selected = m_view->scene()->selectedItems();
    Canvas* canvas = m_project_manager->GetCurrentCanvas();

    if (!selected.isEmpty() && canvas) {
        QGraphicsItem* item = selected.first();
        Object* obj = dynamic_cast<Object*>(item);

        int layerId = canvas->getLayerIdOfObject(obj);
        if (layerId != -1 && canvas->getLayersInfo()[layerId].locked) {
            item->setSelected(false);
            return;
        }

        if (layerId != -1 && layerId != canvas->getSelectedLayerid()) {
            if (m_layers_pannel) m_layers_pannel->selectLayerFromOutside(layerId);
            else canvas->selectLayer(layerId);
        }

        m_selected_figure = dynamic_cast<Figure*>(obj);
        if (m_selected_figure) {
            m_transform_box = new TransformBox(item, m_undo_stack);
            updateTransformBoxScale();
            m_context_pannel->setTarget(m_selected_figure);

            m_palette_pannel->setColor(m_context_pannel->getActiveColor());
        }
    } else {
        m_selected_figure = nullptr;
        m_context_pannel->setTarget(nullptr);
    }
    m_context_pannel->setMode(m_selected_figure != nullptr, m_current_tool == InstrumentType::FIGURE, getToolName(m_current_tool));
}

void WorkspaceController::onMoveObjectLayerRequested(int shift) {
    if (!m_selected_figure) return;
    Canvas* canvas = m_project_manager->GetCurrentCanvas();
    if (!canvas) return;

    int oldLayerId = canvas->getLayerIdOfObject(m_selected_figure);
    int newLayerId = oldLayerId + shift;
    std::vector<LayerInfo> infos = canvas->getLayersInfo();

    if (newLayerId >= 0 && newLayerId < (int)infos.size() && !infos[newLayerId].locked) {
        m_undo_stack->push(new MoveObjectLayerCommand(canvas, m_selected_figure, oldLayerId, newLayerId));
        m_layers_pannel->selectLayerFromOutside(newLayerId);
    }
}

void WorkspaceController::onContextPropertyChanged() {
    if (!m_selected_figure) return;
    FigureState oldState = m_selected_figure->getState();
    FigureState newState = m_context_pannel->getUIState(oldState);
    if (oldState != newState) {
        m_undo_stack->push(new ModifyFigureCommand(m_selected_figure, oldState, newState));
        m_context_pannel->setTarget(m_selected_figure);
    }
}

void WorkspaceController::onColorTargetChanged(bool isFill) {
    m_color_target_is_fill = isFill;
    m_palette_pannel->setColor(m_context_pannel->getActiveColor());
}

void WorkspaceController::onColorPickedPreview(const QColor& color) {
    if (!m_selected_figure || m_selected_figure->getState().type == FigureType::Image) {
        m_context_pannel->setDefaultColor(m_color_target_is_fill, color);
        return;
    }
    if (!m_is_previewing) { m_state_before_preview = m_selected_figure->getState(); m_is_previewing = true; }
    FigureState s = m_selected_figure->getState();
    if (m_color_target_is_fill) s.fill = color; else s.stroke = color;
    m_selected_figure->setState(s);
}

void WorkspaceController::onColorPickedCommit(const QColor& color) {
    if (!m_selected_figure || m_selected_figure->getState().type == FigureType::Image) {
        m_context_pannel->setDefaultColor(m_color_target_is_fill, color);
        return;
    }
    FigureState newState = m_selected_figure->getState();
    if (m_color_target_is_fill) newState.fill = color; else newState.stroke = color;
    if (m_is_previewing) {
        m_selected_figure->setState(m_state_before_preview);
        m_undo_stack->push(new ModifyFigureCommand(m_selected_figure, m_state_before_preview, newState));
        m_is_previewing = false;
    } else {
        FigureState oldState = m_selected_figure->getState();
        m_undo_stack->push(new ModifyFigureCommand(m_selected_figure, oldState, newState));
    }
    m_context_pannel->setTarget(m_selected_figure);
}