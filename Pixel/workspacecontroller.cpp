#include "workspacecontroller.h"
#include "action.h"
#include <QApplication>

WorkspaceController::WorkspaceController(QGraphicsView* view, QGraphicsScene* scene,
                                         ProjectManager* projectManager, ContextPannel* contextPannel,
                                         PalettePannel* palettePannel, QObject *parent)
    : QObject(parent), m_view(view), m_project_manager(projectManager), m_context_pannel(contextPannel)
{
    m_undo_stack = new QUndoStack(this);

    if (m_view) {
        m_view->installEventFilter(this);
        m_view->viewport()->installEventFilter(this);
    }

    if (scene) {
        connect(scene, &QGraphicsScene::selectionChanged, this, &WorkspaceController::onSelectionChanged);
    }

    if (m_context_pannel) {
        connect(m_context_pannel, &ContextPannel::propertyChanged, this, &WorkspaceController::onContextPropertyChanged);
        connect(m_context_pannel, &ContextPannel::colorTargetActivated, this, &WorkspaceController::onColorTargetChanged);
    }

    if (palettePannel) {
        connect(palettePannel, &PalettePannel::colorPreviewed, this, &WorkspaceController::onColorPickedPreview);
        connect(palettePannel, &PalettePannel::colorCommitted, this, &WorkspaceController::onColorPickedCommit);
    }
}

void WorkspaceController::setCurrentTool(InstrumentType type)
{
    m_current_tool = type;
    if (m_current_tool == InstrumentType::HAND) m_view->setCursor(Qt::OpenHandCursor);
    else if (m_current_tool == InstrumentType::FIGURE) m_view->setCursor(Qt::CrossCursor);
    else m_view->setCursor(Qt::ArrowCursor);

    m_context_pannel->setMode(m_selected_figure != nullptr, m_current_tool == InstrumentType::FIGURE);
}

void WorkspaceController::clearTransformBox()
{
    if (m_transform_box) {
        m_transform_box->setParentItem(nullptr);
        if (m_view->scene()) m_view->scene()->removeItem(m_transform_box);
        delete m_transform_box;
        m_transform_box = nullptr;
    }
}

bool WorkspaceController::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_view || !m_project_manager) return QObject::eventFilter(obj, event);


    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->modifiers() & Qt::ControlModifier) {
            if (keyEvent->key() == Qt::Key_Z) {
                m_view->scene()->clearSelection();
                m_undo_stack->undo();
                return true;
            }
            if (keyEvent->key() == Qt::Key_Y) {
                m_view->scene()->clearSelection();
                m_undo_stack->redo();
                return true;
            }
            // COPY
            if (keyEvent->key() == Qt::Key_C && m_selected_figure) {
                m_clipboard_state = m_selected_figure->getState();
                m_has_clipboard = true;
                return true;
            }
            // CUT
            if (keyEvent->key() == Qt::Key_X && m_selected_figure) {
                m_clipboard_state = m_selected_figure->getState();
                m_has_clipboard = true;
                m_undo_stack->push(new DeleteObjectCommand(m_selected_figure->parentItem(), m_selected_figure));
                return true;
            }
            // PASTE
            if (keyEvent->key() == Qt::Key_V && m_has_clipboard) {
                Canvas *canvas = m_project_manager->GetCurrentCanvas();
                if (canvas && canvas->getSelectedLayerid() >= 0) {
                    m_view->scene()->clearSelection();
                    Figure* fig = new Figure();

                    // Сдвигаем позицию на 20 пикселей, чтобы не сливалось
                    m_clipboard_state.pos += QPointF(20, 20);
                    fig->setState(m_clipboard_state);

                    canvas->addObjectToSelectedLayer(fig);
                    m_undo_stack->push(new AddObjectCommand(fig->parentItem(), fig));
                    fig->setSelected(true);
                }
                return true;
            }
        }

        if (keyEvent->key() == Qt::Key_Delete && !m_view->scene()->selectedItems().isEmpty()) {
            QGraphicsItem* selected = m_view->scene()->selectedItems().first();
            m_undo_stack->push(new DeleteObjectCommand(selected->parentItem(), selected));
            return true;
        }

        if (keyEvent->key() == Qt::Key_Space && !keyEvent->isAutoRepeat()) {
            m_space_pressed = true;
            if (!m_is_panning) m_view->setCursor(Qt::OpenHandCursor);
            return true;
        }
    }


    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Space && !keyEvent->isAutoRepeat()) {
            m_space_pressed = false;
            if (!m_is_panning) setCurrentTool(m_current_tool);
            return true;
        }
    }

    if (obj == m_view->viewport() || obj == m_view) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            double factor = (wheelEvent->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
            m_view->scale(factor, factor);
            emit viewportChanged();
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if ((mouseEvent->button() == Qt::LeftButton && (m_space_pressed || m_current_tool == InstrumentType::HAND)) || mouseEvent->button() == Qt::MiddleButton) {
                m_is_panning = true;
                m_last_pan_pos = mouseEvent->pos();
                m_view->setCursor(Qt::ClosedHandCursor);
                return true;
            }

            if (mouseEvent->button() == Qt::LeftButton && m_current_tool == InstrumentType::FIGURE) {
                m_is_drawing = true;
                m_draw_start_pos = m_view->mapToScene(mouseEvent->pos());

                Canvas *canvas = m_project_manager->GetCurrentCanvas();
                if (canvas && canvas->getSelectedLayerid() >= 0) {
                    m_view->scene()->clearSelection();
                    m_temp_figure = new Figure();

                    // ПРИМЕНЯЕМ НАСТРОЙКИ ПО УМОЛЧАНИЮ ИЗ ПАНЕЛИ
                    FigureState defState = m_context_pannel->getDefaultState();
                    defState.pos = m_draw_start_pos;
                    defState.rect = QRectF(0,0,0,0);
                    m_temp_figure->setState(defState);

                    canvas->addObjectToSelectedLayer(m_temp_figure);
                }
                return true;
            }
        }

        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (m_is_panning) {
                QPoint delta = mouseEvent->pos() - m_last_pan_pos;
                m_view->horizontalScrollBar()->setValue(m_view->horizontalScrollBar()->value() - delta.x());
                m_view->verticalScrollBar()->setValue(m_view->verticalScrollBar()->value() - delta.y());
                m_last_pan_pos = mouseEvent->pos();
                return true;
            }

            if (m_is_drawing && m_temp_figure && m_current_tool == InstrumentType::FIGURE) {
                QPointF currentPos = m_view->mapToScene(mouseEvent->pos());
                QRectF newRect = QRectF(m_draw_start_pos, currentPos).normalized();

                m_temp_figure->setPos(newRect.center());
                m_temp_figure->setLocalRect(QRectF(-newRect.width() / 2.0, -newRect.height() / 2.0, newRect.width(), newRect.height()));
                return true;
            }
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (m_is_panning) {
                m_is_panning = false;
                setCurrentTool(m_current_tool);
                return true;
            }

            if (m_is_drawing && mouseEvent->button() == Qt::LeftButton && m_current_tool == InstrumentType::FIGURE) {
                m_is_drawing = false;

                if (m_temp_figure) {
                    QRectF rect = m_temp_figure->getLocalRect();
                    if (rect.width() < 3.0 || rect.height() < 3.0) {
                        m_temp_figure->setParentItem(nullptr);
                        if (m_temp_figure->scene()) m_temp_figure->scene()->removeItem(m_temp_figure);
                        delete m_temp_figure;
                    } else {
                        m_undo_stack->push(new AddObjectCommand(m_temp_figure->parentItem(), m_temp_figure));
                        m_temp_figure->setSelected(true);
                    }
                    m_temp_figure = nullptr;
                }
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void WorkspaceController::onSelectionChanged()
{
    clearTransformBox();
    QList<QGraphicsItem*> selected = m_view->scene()->selectedItems();

    if (selected.size() == 1) {
        QGraphicsItem* item = selected.first();
        m_selected_figure = dynamic_cast<Figure*>(item);

        if (m_selected_figure) {
            m_transform_box = new TransformBox(item, m_undo_stack);
            m_context_pannel->setTarget(m_selected_figure);
        }
    } else {
        m_selected_figure = nullptr;
        m_context_pannel->setTarget(nullptr);
    }

    m_context_pannel->setMode(m_selected_figure != nullptr, m_current_tool == InstrumentType::FIGURE);
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

void WorkspaceController::onColorTargetChanged(bool isFill) { m_color_target_is_fill = isFill; }

void WorkspaceController::onColorPicked(const QColor& color)
{
    if (!m_selected_figure) return;

    FigureState oldState = m_selected_figure->getState();
    FigureState newState = oldState;

    if (m_color_target_is_fill) newState.fill = color;
    else newState.stroke = color;

    if (oldState != newState) {
        m_undo_stack->push(new ModifyFigureCommand(m_selected_figure, oldState, newState));
        m_context_pannel->setTarget(m_selected_figure);
    }
}

void WorkspaceController::onColorPickedPreview(const QColor& color) {
    if (!m_selected_figure) {
        m_context_pannel->setDefaultColor(m_color_target_is_fill, color);
        return;
    }
    if (!m_is_previewing) {
        m_state_before_preview = m_selected_figure->getState();
        m_is_previewing = true;
    }
    FigureState s = m_selected_figure->getState();
    if (m_color_target_is_fill) s.fill = color; else s.stroke = color;
    m_selected_figure->setState(s);
}

void WorkspaceController::onColorPickedCommit(const QColor& color) {
    if (!m_selected_figure) {
        m_context_pannel->setDefaultColor(m_color_target_is_fill, color);
        return;
    }
    FigureState newState = m_selected_figure->getState();
    if (m_color_target_is_fill) newState.fill = color; else newState.stroke = color;

    if (m_is_previewing) {
        m_selected_figure->setState(m_state_before_preview); // Откатываем визуально, чтобы команда сохранила правильно
        m_undo_stack->push(new ModifyFigureCommand(m_selected_figure, m_state_before_preview, newState));
        m_is_previewing = false;
    } else {
        FigureState oldState = m_selected_figure->getState();
        m_undo_stack->push(new ModifyFigureCommand(m_selected_figure, oldState, newState));
    }
    m_context_pannel->setTarget(m_selected_figure);
}