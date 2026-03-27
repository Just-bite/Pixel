#include "workspacecontroller.h"
#include "action.h"
#include <QApplication>
#include <QKeyEvent>
#include <QScrollBar>
#include <QClipboard>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QInputDialog>

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
    if (type == InstrumentType::TEXT) return "Text";
    return "Unknown Tool";
}

void WorkspaceController::setCurrentTool(InstrumentType type) {
    m_current_tool = type;
    if (m_current_tool == InstrumentType::HAND) m_view->setCursor(Qt::OpenHandCursor);
    else if (m_current_tool == InstrumentType::FIGURE) m_view->setCursor(Qt::CrossCursor);
    else if (m_current_tool == InstrumentType::TEXT) m_view->setCursor(Qt::IBeamCursor);
    else m_view->setCursor(Qt::ArrowCursor);

    m_context_pannel->setMode(m_selected_figure != nullptr, m_selected_text != nullptr,
                              m_current_tool == InstrumentType::FIGURE, m_current_tool == InstrumentType::TEXT,
                              getToolName(type));
}

void WorkspaceController::updateTransformBoxScale() {
    if (m_transform_box && m_view) {
        m_transform_box->setViewScale(m_view->transform().m11());
    }
}

void WorkspaceController::clearTransformBox() {
    if (m_transform_box) {
        delete m_transform_box;
        m_transform_box = nullptr;
    }
}

bool WorkspaceController::eventFilter(QObject *obj, QEvent *event) {
    if (!m_view || !m_project_manager) return QObject::eventFilter(obj, event);
    Canvas* canvas = m_project_manager->GetCurrentCanvas();

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kEvent = static_cast<QKeyEvent *>(event);
        if (kEvent->modifiers() & Qt::ControlModifier) {
            if (kEvent->key() == Qt::Key_Z) { m_view->scene()->clearSelection(); m_undo_stack->undo(); return true; }
            if (kEvent->key() == Qt::Key_Y) { m_view->scene()->clearSelection(); m_undo_stack->redo(); return true; }

            // Копирование
            if (kEvent->key() == Qt::Key_C) {
                if (m_selected_figure) { m_clipboard_state = m_selected_figure->getState(); m_has_clipboard = true; m_clipboard_is_text = false; }
                else if (m_selected_text) { m_clipboard_text_state = m_selected_text->getState(); m_has_clipboard = true; m_clipboard_is_text = true; }
                QApplication::clipboard()->clear(); // Очищаем системный буфер, чтобы приоритет был у внутреннего
                return true;
            }
            // Вырезка
            if (kEvent->key() == Qt::Key_X) {
                if (m_selected_figure) {
                    m_clipboard_state = m_selected_figure->getState(); m_has_clipboard = true; m_clipboard_is_text = false;
                    m_undo_stack->push(new DeleteObjectCommand(m_selected_figure->parentItem(), m_selected_figure));
                } else if (m_selected_text) {
                    m_clipboard_text_state = m_selected_text->getState(); m_has_clipboard = true; m_clipboard_is_text = true;
                    m_undo_stack->push(new DeleteObjectCommand(m_selected_text->parentItem(), m_selected_text));
                }
                QApplication::clipboard()->clear();
                return true;
            }
            // Вставка
            if (kEvent->key() == Qt::Key_V && canvas) {
                int activeId = canvas->getSelectedLayerid();
                if (activeId >= 0 && !canvas->getLayersInfo()[activeId].locked) {
                    const QClipboard *clipboard = QApplication::clipboard();
                    const QMimeData *mimeData = clipboard->mimeData();
                    QImage img;

                    // 1. Приоритет системного буфера обмена (внешнее копирование картинки)
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
                        m_view->scene()->clearSelection();
                        Figure* fig = new Figure();
                        FigureState s;
                        s.type = FigureType::Image;
                        s.image = img;
                        s.pos = m_last_mouse_scene_pos;
                        s.rect = QRectF(-img.width()/2.0, -img.height()/2.0, img.width(), img.height());
                        fig->setState(s);
                        canvas->addObjectToSelectedLayer(fig);
                        m_undo_stack->push(new AddObjectCommand(fig->parentItem(), fig));
                        fig->setSelected(true);
                        return true;
                    }

                    // 2. Вставка нашего внутреннего скопированного объекта
                    if (m_has_clipboard) {
                        m_view->scene()->clearSelection();
                        if (!m_clipboard_is_text) {
                            Figure* fig = new Figure();
                            m_clipboard_state.pos = m_last_mouse_scene_pos;
                            fig->setState(m_clipboard_state);
                            canvas->addObjectToSelectedLayer(fig);
                            m_undo_stack->push(new AddObjectCommand(fig->parentItem(), fig));
                            fig->setSelected(true);
                        } else {
                            TextObject* txt = new TextObject();
                            m_clipboard_text_state.pos = m_last_mouse_scene_pos;
                            txt->setState(m_clipboard_text_state);
                            canvas->addObjectToSelectedLayer(txt);
                            m_undo_stack->push(new AddObjectCommand(txt->parentItem(), txt));
                            txt->setSelected(true);
                        }
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

        // Перемещение стрелочками (Nudge)
        if (kEvent->key() == Qt::Key_Up || kEvent->key() == Qt::Key_Down || kEvent->key() == Qt::Key_Left || kEvent->key() == Qt::Key_Right) {
            if ((m_selected_figure || m_selected_text) && !m_space_pressed) {
                qreal step = 1.0 / m_view->transform().m11();
                QPointF delta(0, 0);
                if (kEvent->key() == Qt::Key_Up) delta.setY(-step);
                if (kEvent->key() == Qt::Key_Down) delta.setY(step);
                if (kEvent->key() == Qt::Key_Left) delta.setX(-step);
                if (kEvent->key() == Qt::Key_Right) delta.setX(step);

                if (m_selected_figure) {
                    FigureState oldState = m_selected_figure->getState();
                    FigureState newState = oldState;
                    newState.pos += delta;
                    m_undo_stack->push(new ModifyFigureCommand(m_selected_figure, oldState, newState));
                    m_context_pannel->setTarget(m_selected_figure);
                } else if (m_selected_text) {
                    TextState oldState = m_selected_text->getState();
                    TextState newState = oldState;
                    newState.pos += delta;
                    m_undo_stack->push(new ModifyTextCommand(m_selected_text, oldState, newState));
                    m_context_pannel->setTarget(m_selected_text);
                }
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

    // Обработка Drag & Drop
    if (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove) {
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

        if (event->type() == QEvent::MouseButtonDblClick && m_current_tool == InstrumentType::POINTER) {
            QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
            QGraphicsItem* item = m_view->itemAt(mEvent->pos());
            TextObject* txt = dynamic_cast<TextObject*>(item);
            if (!txt && item && item->parentItem()) txt = dynamic_cast<TextObject*>(item->parentItem());

            if (txt) {
                bool ok;
                QString newText = QInputDialog::getMultiLineText(m_view, "Edit Text", "Enter text:", txt->getState().text, &ok);
                if (ok && !newText.isEmpty()) {
                    TextState oldState = txt->getState();
                    TextState newState = oldState; newState.text = newText;
                    m_undo_stack->push(new ModifyTextCommand(txt, oldState, newState));
                }
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
            if (mEvent->button() == Qt::LeftButton && m_current_tool == InstrumentType::TEXT && canvas) {
                int activeId = canvas->getSelectedLayerid();
                if (activeId >= 0 && !canvas->getLayersInfo()[activeId].locked) {
                    m_is_drawing = true; m_draw_start_pos = m_view->mapToScene(mEvent->pos());
                    m_view->scene()->clearSelection();
                    m_temp_text = new TextObject();
                    TextState defState;
                    defState.pos = m_draw_start_pos; defState.rect = QRectF(0,0,0,0);
                    defState.font = m_context_pannel->getUITextState(defState).font;
                    defState.color = m_context_pannel->getActiveColor();
                    m_temp_text->setState(defState);
                    canvas->addObjectToSelectedLayer(m_temp_text);
                }
                return true;
            }
            if (mEvent->button() == Qt::LeftButton && m_current_tool == InstrumentType::POINTER) {
                QGraphicsItem* item = m_view->itemAt(mEvent->pos());

                Figure* fig = dynamic_cast<Figure*>(item);
                if (!fig && item && item->parentItem()) fig = dynamic_cast<Figure*>(item->parentItem());
                if (fig) { m_drag_target = fig; m_drag_start_state = fig->getState(); }
                else { m_drag_target = nullptr; }

                TextObject* txt = dynamic_cast<TextObject*>(item);
                if (!txt && item && item->parentItem()) txt = dynamic_cast<TextObject*>(item->parentItem());
                if (txt) { m_drag_target_text = txt; m_drag_start_text_state = txt->getState(); }
                else { m_drag_target_text = nullptr; }
            }
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
            if (m_is_drawing && m_temp_text) {
                QPointF cPos = m_view->mapToScene(mEvent->pos());
                QRectF nRect = QRectF(m_draw_start_pos, cPos).normalized();
                m_temp_text->setPos(nRect.center());
                m_temp_text->setLocalRect(QRectF(-nRect.width()/2.0, -nRect.height()/2.0, nRect.width(), nRect.height()));
                return true;
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
                        delete m_temp_figure;
                    } else {
                        m_undo_stack->push(new AddObjectCommand(m_temp_figure->parentItem(), m_temp_figure));
                        m_temp_figure->setSelected(true);
                    }
                    m_temp_figure = nullptr;
                }
                if (m_temp_text) {
                    QRectF r = m_temp_text->getLocalRect();
                    if (r.width() < 15.0 || r.height() < 10.0) {
                        delete m_temp_text;
                    } else {
                        bool ok;
                        QString newText = QInputDialog::getMultiLineText(m_view, "New Text", "Enter text:", "Text", &ok);
                        if (ok && !newText.isEmpty()) {
                            TextState s = m_temp_text->getState();
                            s.text = newText;
                            m_temp_text->setState(s);
                            m_undo_stack->push(new AddObjectCommand(m_temp_text->parentItem(), m_temp_text));
                            m_temp_text->setSelected(true);
                        } else {
                            delete m_temp_text;
                        }
                    }
                    m_temp_text = nullptr;
                }
                return true;
            }

            if (mEvent->button() == Qt::LeftButton && m_current_tool == InstrumentType::POINTER) {
                bool handledByTransformBox = (m_transform_box && m_transform_box->isInteracting());

                if (m_drag_target) {
                    FigureState newState = m_drag_target->getState();
                    if (!handledByTransformBox && newState != m_drag_start_state) {
                        m_undo_stack->push(new ModifyFigureCommand(m_drag_target, m_drag_start_state, newState));
                        if (m_drag_target == m_selected_figure) m_context_pannel->setTarget(m_selected_figure);
                    }
                    m_drag_target = nullptr;
                }
                if (m_drag_target_text) {
                    TextState newState = m_drag_target_text->getState();
                    if (!handledByTransformBox && newState != m_drag_start_text_state) {
                        m_undo_stack->push(new ModifyTextCommand(m_drag_target_text, m_drag_start_text_state, newState));
                        if (m_drag_target_text == m_selected_text) m_context_pannel->setTarget(m_selected_text);
                    }
                    m_drag_target_text = nullptr;
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
        m_selected_text = dynamic_cast<TextObject*>(obj);

        if (m_selected_figure) {
            m_transform_box = new TransformBox(item, m_undo_stack);
            updateTransformBoxScale();
            m_context_pannel->setTarget(m_selected_figure);
        } else if (m_selected_text) {
            m_transform_box = new TransformBox(item, m_undo_stack);
            updateTransformBoxScale();
            m_context_pannel->setTarget(m_selected_text);
        }

        if (m_selected_figure || m_selected_text) {
            m_palette_pannel->setColor(m_context_pannel->getActiveColor());
        }
    } else {
        m_selected_figure = nullptr;
        m_selected_text = nullptr;
        m_context_pannel->setTarget(static_cast<Figure*>(nullptr));
    }

    m_context_pannel->setMode(m_selected_figure != nullptr, m_selected_text != nullptr,
                              m_current_tool == InstrumentType::FIGURE, m_current_tool == InstrumentType::TEXT,
                              getToolName(m_current_tool));
}

void WorkspaceController::onMoveObjectLayerRequested(int shift) {
    Object* target = m_selected_figure ? (Object*)m_selected_figure : (Object*)m_selected_text;
    if (!target) return;
    Canvas* canvas = m_project_manager->GetCurrentCanvas();
    if (!canvas) return;

    int oldLayerId = canvas->getLayerIdOfObject(target);
    int newLayerId = oldLayerId + shift;
    std::vector<LayerInfo> infos = canvas->getLayersInfo();

    if (newLayerId >= 0 && newLayerId < (int)infos.size() && !infos[newLayerId].locked) {
        m_undo_stack->push(new MoveObjectLayerCommand(canvas, target, oldLayerId, newLayerId));
        m_layers_pannel->selectLayerFromOutside(newLayerId);
    }
}

void WorkspaceController::onContextPropertyChanged() {
    if (m_selected_figure) {
        FigureState oldState = m_selected_figure->getState();
        FigureState newState = m_context_pannel->getUIState(oldState);
        if (oldState != newState) {
            m_undo_stack->push(new ModifyFigureCommand(m_selected_figure, oldState, newState));
            m_context_pannel->setTarget(m_selected_figure);
        }
    } else if (m_selected_text) {
        TextState oldState = m_selected_text->getState();
        TextState newState = m_context_pannel->getUITextState(oldState);
        if (oldState != newState) {
            m_undo_stack->push(new ModifyTextCommand(m_selected_text, oldState, newState));
            m_context_pannel->setTarget(m_selected_text);
        }
    }
}

void WorkspaceController::onColorTargetChanged(bool isFill) {
    m_color_target_is_fill = isFill;
    m_palette_pannel->setColor(m_context_pannel->getActiveColor());
}

void WorkspaceController::onColorPickedPreview(const QColor& color) {
    if (m_selected_figure) {
        if (m_selected_figure->getState().type == FigureType::Image) return; // Картинки не красим
        if (!m_is_previewing) { m_state_before_preview = m_selected_figure->getState(); m_is_previewing = true; }
        FigureState s = m_selected_figure->getState();
        if (m_color_target_is_fill) s.fill = color; else s.stroke = color;
        m_selected_figure->setState(s);
    } else if (m_selected_text) {
        if (!m_is_previewing) { m_text_state_before_preview = m_selected_text->getState(); m_is_previewing = true; }
        TextState s = m_selected_text->getState();
        s.color = color;
        m_selected_text->setState(s);
    } else {
        m_context_pannel->setDefaultColor(m_color_target_is_fill, color);
    }
}

void WorkspaceController::onColorPickedCommit(const QColor& color) {
    if (m_selected_figure) {
        if (m_selected_figure->getState().type == FigureType::Image) return;
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
    } else if (m_selected_text) {
        TextState newState = m_selected_text->getState();
        newState.color = color;
        if (m_is_previewing) {
            m_selected_text->setState(m_text_state_before_preview);
            m_undo_stack->push(new ModifyTextCommand(m_selected_text, m_text_state_before_preview, newState));
            m_is_previewing = false;
        } else {
            TextState oldState = m_selected_text->getState();
            m_undo_stack->push(new ModifyTextCommand(m_selected_text, oldState, newState));
        }
        m_context_pannel->setTarget(m_selected_text);
    } else {
        m_context_pannel->setDefaultColor(m_color_target_is_fill, color);
    }
}

void WorkspaceController::clearState() {
    clearTransformBox();
    m_view->scene()->clearSelection();

    if (m_undo_stack) m_undo_stack->clear();

    m_selected_figure = nullptr;
    m_temp_figure = nullptr;
    m_drag_target = nullptr;

    m_selected_text = nullptr;
    m_temp_text = nullptr;
    m_drag_target_text = nullptr;

    m_has_clipboard = false;
    m_is_drawing = false;
    m_is_panning = false;
}