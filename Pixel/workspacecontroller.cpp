#include "workspacecontroller.h"
#include "action.h"
#include <QApplication>

// ИСПОЛЬЗУЕМ ПЕРЕДАННУЮ СЦЕНУ ДЛЯ ПОДКЛЮЧЕНИЯ
WorkspaceController::WorkspaceController(QGraphicsView* view, QGraphicsScene* scene, ProjectManager* projectManager, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_project_manager(projectManager)
{
    m_undo_stack = new QUndoStack(this);

    if (m_view) {
        m_view->installEventFilter(this);
        m_view->viewport()->installEventFilter(this);
    }

    // ТЕПЕРЬ ПОДКЛЮЧЕНИЕ ГАРАНТИРОВАННО СРАБОТАЕТ
    if (scene) {
        connect(scene, &QGraphicsScene::selectionChanged, this, &WorkspaceController::onSelectionChanged);
    }
}

// ... остальной код файла без изменений ...

void WorkspaceController::setCurrentTool(InstrumentType type)
{
    m_current_tool = type;
    qDebug() << "[Controller] Active tool changed to:" << static_cast<int>(type);

    if (m_current_tool == InstrumentType::HAND) m_view->setCursor(Qt::OpenHandCursor);
    else if (m_current_tool == InstrumentType::FIGURE) m_view->setCursor(Qt::CrossCursor);
    else m_view->setCursor(Qt::ArrowCursor);
}

bool WorkspaceController::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_view || !m_project_manager) return QObject::eventFilter(obj, event);

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->modifiers() & Qt::ControlModifier) {
            if (keyEvent->key() == Qt::Key_Z) {
                m_undo_stack->undo();
                return true;
            }
            if (keyEvent->key() == Qt::Key_Y) {
                m_undo_stack->redo();
                return true;
            }
        }

        if (keyEvent->key() == Qt::Key_Delete) {
            if (m_view->scene()->selectedItems().isEmpty()) return false;

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

    if (obj == m_view->viewport() || obj == m_view) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            int delta = wheelEvent->angleDelta().y();
            double factor = (delta > 0) ? 1.15 : (1.0 / 1.15);
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

                    m_temp_ellipse = new Ellipse(QRectF(m_draw_start_pos, QSizeF(0, 0)));
                    canvas->addObjectToSelectedLayer(m_temp_ellipse);
                }
                return true;
            }

            if (m_current_tool == InstrumentType::POINTER) {
                return false;
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

            if (m_is_drawing && m_temp_ellipse && m_current_tool == InstrumentType::FIGURE) {
                QPointF currentPos = m_view->mapToScene(mouseEvent->pos());
                QRectF newRect = QRectF(m_draw_start_pos, currentPos).normalized();
                m_temp_ellipse->setRect(newRect);
                return true;
            }

            if (m_current_tool == InstrumentType::POINTER) return false;
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

                if (m_temp_ellipse) {
                    m_temp_ellipse->setRect(m_temp_ellipse->getRect().normalized());
                    m_undo_stack->push(new AddObjectCommand(m_temp_ellipse->parentItem(), m_temp_ellipse));

                    m_temp_ellipse->setSelected(true);
                }
                m_temp_ellipse = nullptr;
                return true;
            }

            if (m_current_tool == InstrumentType::POINTER) return false;
        }
    }

    return QObject::eventFilter(obj, event);
}

void WorkspaceController::onSelectionChanged()
{
    // Безопасное удаление старой рамки
    if (m_transform_box) {
        m_transform_box->setParentItem(nullptr);
        delete m_transform_box;
        m_transform_box = nullptr;
    }

    QList<QGraphicsItem*> selected = m_view->scene()->selectedItems();

    if (selected.size() == 1) {
        QGraphicsItem* item = selected.first();

        // Так как фон и слои не выделяемы, это 100% объект Фигуры.
        // Просто создаем рамку и привязываем к нему!
        m_transform_box = new TransformBox(item);

        qDebug() << "[Controller] TransformBox CREATED successfully for item!";
    } else {
        qDebug() << "[Controller] Selection cleared or multiple items selected.";
    }
}