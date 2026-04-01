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

    WorkspaceController::WorkspaceController(const WorkspaceContext& ctx, QObject* parent)
    : QObject(parent), m_view(ctx.view), m_project_manager(ctx.projectManager),
m_context_pannel(ctx.contextPannel), m_palette_pannel(ctx.palettePannel),
m_layers_pannel(ctx.layersPannel), m_last_mouse_scene_pos(0,0)
{
    m_undo_stack = new QUndoStack(this);
    if (m_view) {
        m_view->installEventFilter(this);
        m_view->viewport()->installEventFilter(this);
    }
    if (ctx.scene) connect(ctx.scene, &QGraphicsScene::selectionChanged, this, &WorkspaceController::onSelectionChanged);

    if (m_context_pannel) {
        connect(m_context_pannel, &ContextPannel::propertyChanged, this, &WorkspaceController::onContextPropertyChanged);
        connect(m_context_pannel, &ContextPannel::colorTargetActivated, this, &WorkspaceController::onColorTargetChanged);
        connect(m_context_pannel, &ContextPannel::moveObjectLayerRequested, this, &WorkspaceController::onMoveObjectLayerRequested);
    }
    if (m_palette_pannel) {
        connect(m_palette_pannel, &PalettePannel::colorPreviewed, this, &WorkspaceController::onColorPickedPreview);
        connect(m_palette_pannel, &PalettePannel::colorCommitted, this, &WorkspaceController::onColorPickedCommit);
    }

    connect(m_undo_stack, &QUndoStack::indexChanged, this, [this](){
        if (Canvas* c = m_project_manager->GetCurrentCanvas()) c->updateFilters();
    });

    if (m_project_manager) {
        connect(m_project_manager->GetCurrentCanvas(), &Canvas::activeLayerChanged, this, &WorkspaceController::onActiveLayerChanged);
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
        if (m_transform_box->scene()) {
            m_transform_box->scene()->removeItem(m_transform_box);
        }
        delete m_transform_box; // НЕМЕДЛЕННОЕ удаление убивает "призрачные" рамки выделения
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

            if (kEvent->key() == Qt::Key_C) {
                if (m_selected_figure) { m_clipboard_figure = m_selected_figure->getState(); m_clipboard_type = ClipboardType::Figure; }
                else if (m_selected_text) { m_clipboard_text = m_selected_text->getState(); m_clipboard_type = ClipboardType::Text; }
                else if (m_selected_image) { m_clipboard_image = m_selected_image->getState(); m_clipboard_type = ClipboardType::Image; }
                QApplication::clipboard()->clear(); return true;
            }
            if (kEvent->key() == Qt::Key_X) {
                if (m_selected_figure) {
                    m_clipboard_figure = m_selected_figure->getState(); m_clipboard_type = ClipboardType::Figure;
                    m_undo_stack->push(new DeleteObjectCommand(m_selected_figure->parentItem(), m_selected_figure));
                } else if (m_selected_text) {
                    m_clipboard_text = m_selected_text->getState(); m_clipboard_type = ClipboardType::Text;
                    m_undo_stack->push(new DeleteObjectCommand(m_selected_text->parentItem(), m_selected_text));
                } else if (m_selected_image) {
                    m_clipboard_image = m_selected_image->getState(); m_clipboard_type = ClipboardType::Image;
                    m_undo_stack->push(new DeleteObjectCommand(m_selected_image->parentItem(), m_selected_image));
                }
                QApplication::clipboard()->clear(); return true;
            }
            if (kEvent->key() == Qt::Key_V && !kEvent->isAutoRepeat() && canvas) {
                int activeId = canvas->getSelectedLayerid();
                // Запрещаем вставку объектов на закрытые слои и слои-фильтры
                if (activeId >= 0 && !canvas->getLayersInfo()[activeId].locked && !canvas->getLayersInfo()[activeId].isFilter) {
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
                        m_view->scene()->clearSelection();
                        ImageObject* imgObj = new ImageObject();
                        ImageState s; s.image = img; s.pos = m_last_mouse_scene_pos;
                        s.rect = QRectF(-img.width()/2.0, -img.height()/2.0, img.width(), img.height());
                        imgObj->setState(s);
                        canvas->addObjectToSelectedLayer(imgObj);
                        m_undo_stack->push(new AddObjectCommand(imgObj->parentItem(), imgObj));
                        imgObj->setSelected(true);
                        return true;
                    }

                    if (m_clipboard_type != ClipboardType::None) {
                        m_view->scene()->clearSelection();
                        if (m_clipboard_type == ClipboardType::Figure) {
                            Figure* fig = new Figure(); m_clipboard_figure.pos = m_last_mouse_scene_pos;
                            fig->setState(m_clipboard_figure); canvas->addObjectToSelectedLayer(fig);
                            m_undo_stack->push(new AddObjectCommand(fig->parentItem(), fig)); fig->setSelected(true);
                        } else if (m_clipboard_type == ClipboardType::Text) {
                            TextObject* txt = new TextObject(); m_clipboard_text.pos = m_last_mouse_scene_pos;
                            txt->setState(m_clipboard_text); canvas->addObjectToSelectedLayer(txt);
                            m_undo_stack->push(new AddObjectCommand(txt->parentItem(), txt)); txt->setSelected(true);
                        } else if (m_clipboard_type == ClipboardType::Image) {
                            ImageObject* img = new ImageObject(); m_clipboard_image.pos = m_last_mouse_scene_pos;
                            img->setState(m_clipboard_image); canvas->addObjectToSelectedLayer(img);
                            m_undo_stack->push(new AddObjectCommand(img->parentItem(), img)); img->setSelected(true);
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
        // Запрещаем Drag&Drop объектов на закрытые слои и слои-фильтры
        if (canvas && activeId >= 0 && !canvas->getLayersInfo()[activeId].locked && !canvas->getLayersInfo()[activeId].isFilter) {
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
                ImageObject* imgObj = new ImageObject();
                ImageState s;
                s.image = img;
                s.pos = scenePos;
                s.rect = QRectF(-img.width()/2.0, -img.height()/2.0, img.width(), img.height());
                imgObj->setState(s);
                m_view->scene()->clearSelection();
                canvas->addObjectToSelectedLayer(imgObj);
                m_undo_stack->push(new AddObjectCommand(imgObj->parentItem(), imgObj));
                imgObj->setSelected(true);
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
            TextObject* txt = nullptr;

            for (QGraphicsItem* it : m_view->items(mEvent->pos())) {
                txt = dynamic_cast<TextObject*>(it);
                if (!txt && it->parentItem()) txt = dynamic_cast<TextObject*>(it->parentItem());
                if (txt) break;
            }

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
                // Запрещаем рисование на закрытом слое или слое-фильтре
                if (activeId >= 0 && !canvas->getLayersInfo()[activeId].locked && !canvas->getLayersInfo()[activeId].isFilter) {
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
                if (activeId >= 0 && !canvas->getLayersInfo()[activeId].locked && !canvas->getLayersInfo()[activeId].isFilter) {
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

                bool clickedTransformBox = false;
                for (QGraphicsItem* it : m_view->items(mEvent->pos())) {
                    if (dynamic_cast<TransformBox*>(it)) {
                        clickedTransformBox = true;
                        break;
                    }
                }

                if (clickedTransformBox) {
                    return QObject::eventFilter(obj, event);
                }

                Object* targetObj = nullptr;
                for (QGraphicsItem* it : m_view->items(mEvent->pos())) {
                    targetObj = dynamic_cast<Object*>(it);
                    if (!targetObj && it->parentItem()) targetObj = dynamic_cast<Object*>(it->parentItem());
                    if (targetObj) break;
                }

                if (targetObj) {
                    if (!targetObj->isSelected()) {
                        m_view->scene()->clearSelection();
                        targetObj->setSelected(true);
                    }
                } else {
                    m_view->scene()->clearSelection();
                }

                Figure* fig = dynamic_cast<Figure*>(targetObj);
                if (fig) { m_drag_target = fig; m_drag_start_state = fig->getState(); }
                else { m_drag_target = nullptr; }

                TextObject* txt = dynamic_cast<TextObject*>(targetObj);
                if (txt) { m_drag_target_text = txt; m_drag_start_text_state = txt->getState(); }
                else { m_drag_target_text = nullptr; }

                ImageObject* img = dynamic_cast<ImageObject*>(targetObj);
                if (img) { m_drag_target_image = img; m_drag_start_image_state = img->getState(); }
                else { m_drag_target_image = nullptr; }
            }
        }

        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
            m_last_mouse_scene_pos = m_view->mapToScene(mEvent->pos());

            if (m_transform_box && !m_transform_box->isInteracting()) {
                m_transform_box->syncPosition();
            }

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
                if (m_drag_target_image) {
                    ImageState newState = m_drag_target_image->getState();
                    if (!handledByTransformBox && newState != m_drag_start_image_state) {
                        m_undo_stack->push(new ModifyImageCommand(m_drag_target_image, m_drag_start_image_state, newState));
                        if (m_drag_target_image == m_selected_image) m_context_pannel->setTarget(m_selected_image);
                    }
                    m_drag_target_image = nullptr;
                }
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void WorkspaceController::onSelectionChanged() {
    // Предотвращаем рекурсивный вызов создания дубликатов TransformBox
    if (m_updating_selection) return;
    m_updating_selection = true;

    clearTransformBox();
    QList<QGraphicsItem*> selected = m_view->scene()->selectedItems();
    Canvas* canvas = m_project_manager->GetCurrentCanvas();

    if (!selected.isEmpty() && canvas) {
        QGraphicsItem* item = selected.first();
        Object* obj = dynamic_cast<Object*>(item);

        int layerId = canvas->getLayerIdOfObject(obj);
        if (layerId != -1 && canvas->getLayersInfo()[layerId].locked) {
            item->setSelected(false);
            m_updating_selection = false;
            return;
        }

        if (layerId != -1 && layerId != canvas->getSelectedLayerid()) {
            if (m_layers_pannel) m_layers_pannel->selectLayerFromOutside(layerId);
            else canvas->selectLayer(layerId);
        }

        m_selected_figure = dynamic_cast<Figure*>(obj);
        m_selected_text = dynamic_cast<TextObject*>(obj);
        m_selected_image = dynamic_cast<ImageObject*>(obj);

        if (m_selected_figure || m_selected_text || m_selected_image) {
            // Отключаем фильтры если выделен любой объект, это решает
            // проблему их наслоения поверх выделения и запекания TransformBox в фильтрах.
            canvas->setFiltersInteractionActive(false);

            m_transform_box = new TransformBox(item, m_undo_stack);
            m_view->scene()->addItem(m_transform_box);

            // Теперь мы прикрепляем обновление ContextPannel на событие окончания взаимодействия с Transform Box
            connect(m_transform_box, &TransformBox::interactionEnded, this, [this]() {
                if (m_selected_figure) m_context_pannel->setTarget(m_selected_figure);
                else if (m_selected_text) m_context_pannel->setTarget(m_selected_text);
                else if (m_selected_image) m_context_pannel->setTarget(m_selected_image);
            });

            updateTransformBoxScale();

            if (m_selected_figure) m_context_pannel->setTarget(m_selected_figure);
            else if (m_selected_text) m_context_pannel->setTarget(m_selected_text);
            else m_context_pannel->setTarget(m_selected_image);

            m_palette_pannel->setColor(m_context_pannel->getActiveColor());
        }
    } else {
        m_selected_figure = nullptr;
        m_selected_text = nullptr;
        m_selected_image = nullptr;
        m_context_pannel->setTarget(static_cast<Figure*>(nullptr));

        // Включаем фильтры обратно если нет выделенного объекта
        if (canvas) canvas->setFiltersInteractionActive(true);
    }

    m_context_pannel->setMode(
        m_selected_figure != nullptr,
        m_selected_text != nullptr,
        m_current_tool == InstrumentType::FIGURE,
        m_current_tool == InstrumentType::TEXT,
        getToolName(m_current_tool)
        );

    m_updating_selection = false;
}

void WorkspaceController::onMoveObjectLayerRequested(int shift) {
    Object* target = nullptr;
    if (m_selected_figure) target = m_selected_figure;
    else if (m_selected_text) target = m_selected_text;
    else if (m_selected_image) target = m_selected_image;

    if (!target) return;

    Canvas* canvas = m_project_manager->GetCurrentCanvas();
    if (!canvas) return;

    int oldLayerId = canvas->getLayerIdOfObject(target);
    int newLayerId = oldLayerId + shift;
    std::vector<LayerInfo> infos = canvas->getLayersInfo();

    while (newLayerId >= 0 && newLayerId < (int)infos.size()) {
        if (!infos[newLayerId].locked && !infos[newLayerId].isFilter) {
            break;
        }
        newLayerId += (shift > 0) ? 1 : -1;
    }

    if (newLayerId >= 0 && newLayerId < (int)infos.size() && newLayerId != oldLayerId) {
        m_undo_stack->push(new MoveObjectLayerCommand(canvas, target, oldLayerId, newLayerId));
        m_layers_pannel->selectLayerFromOutside(newLayerId);
    }
}

void WorkspaceController::onContextPropertyChanged() {
    Canvas* canvas = m_project_manager->GetCurrentCanvas();
    if (canvas && canvas->getSelectedLayerid() >= 0) {
        Layer* activeLayer = canvas->getLayers()[canvas->getSelectedLayerid()];
        if (activeLayer->isFilter()) {
            FilterLayer* filter = static_cast<FilterLayer*>(activeLayer);
            FilterState oldState = filter->getFilterState();
            FilterState newState = m_context_pannel->getUIFilterState();
            if (oldState != newState) {
                m_undo_stack->push(new ModifyFilterCommand(filter, oldState, newState));
            }
            return;
        }
    }

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
    } else if (m_selected_image) {
        ImageState oldState = m_selected_image->getState();
        ImageState newState = m_context_pannel->getUIImageState(oldState);
        if (oldState != newState) {
            m_undo_stack->push(new ModifyImageCommand(m_selected_image, oldState, newState));
            m_context_pannel->setTarget(m_selected_image);
            if (m_transform_box) m_transform_box->syncPosition();
        }
    }
}

void WorkspaceController::onColorTargetChanged(bool isFill) {
    m_color_target_is_fill = isFill;
    m_palette_pannel->setColor(m_context_pannel->getActiveColor());
}

void WorkspaceController::onColorPickedPreview(const QColor& color) {
    if (m_selected_figure) {
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

void WorkspaceController::onActiveLayerChanged(int id) {
    Canvas* canvas = m_project_manager->GetCurrentCanvas();
    if (!canvas) return;

    if (id >= 0 && canvas->getLayers()[id]->isFilter()) {
        FilterLayer* filter = static_cast<FilterLayer*>(canvas->getLayers()[id]);
        clearTransformBox();
        m_view->scene()->clearSelection();
        m_selected_figure = nullptr; m_selected_text = nullptr; m_selected_image = nullptr;
        m_context_pannel->setTarget(filter);
        m_context_pannel->setMode(false, false, false, false, "Filter Properties");
    } else {
        m_context_pannel->setTarget(static_cast<Figure*>(nullptr));
        setCurrentTool(m_current_tool);
        onSelectionChanged();
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

    m_selected_image = nullptr;
    m_drag_target_image = nullptr;

    m_has_clipboard = false;
    m_is_drawing = false;
    m_is_panning = false;
}