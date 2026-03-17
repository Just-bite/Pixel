#include "workspacecontroller.h"

WorkspaceController::WorkspaceController(QGraphicsView* view, ProjectManager* projectManager, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_project_manager(projectManager)
{
    if (m_view) {
        m_view->installEventFilter(this);
        m_view->viewport()->installEventFilter(this);
    }
}

void WorkspaceController::setCurrentTool(InstrumentType type)
{
    m_current_tool = type;
    qDebug() << "[Controller] Active tool changed to:" << static_cast<int>(type);

    // Сбрасываем курсор при смене инструмента
    if (m_current_tool == InstrumentType::HAND) m_view->setCursor(Qt::OpenHandCursor);
    else if (m_current_tool == InstrumentType::FIGURE) m_view->setCursor(Qt::CrossCursor);
    else m_view->setCursor(Qt::ArrowCursor);
}

bool WorkspaceController::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_view || !m_project_manager) return QObject::eventFilter(obj, event);

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Space && !keyEvent->isAutoRepeat()) {
            m_space_pressed = true;
            if (!m_is_panning) m_view->setCursor(Qt::OpenHandCursor);
            return true;
        }
    } else if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Space && !keyEvent->isAutoRepeat()) {
            m_space_pressed = false;
            m_is_panning = false;
            // Возвращаем курсор текущего инструмента
            setCurrentTool(m_current_tool);
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

            // Если нажат пробел, колесико ИЛИ выбран инструмент HAND -> Панорамируем
            if ((mouseEvent->button() == Qt::LeftButton && (m_space_pressed || m_current_tool == InstrumentType::HAND)) || mouseEvent->button() == Qt::MiddleButton) {
                m_is_panning = true;
                m_last_pan_pos = mouseEvent->pos();
                m_view->setCursor(Qt::ClosedHandCursor);
                return true;
            }

            // Инструмент: Фигура
            if (mouseEvent->button() == Qt::LeftButton && m_current_tool == InstrumentType::FIGURE) {
                m_is_drawing = true;
                m_draw_start_pos = m_view->mapToScene(mouseEvent->pos());

                Canvas *canvas = m_project_manager->GetCurrentCanvas();
                if (canvas && canvas->getSelectedLayerid() >= 0) {
                    m_temp_ellipse = new Ellipse(QRectF(m_draw_start_pos, QSizeF(0, 0)));
                    canvas->addObjectToSelectedLayer(m_temp_ellipse);
                }
                return true; // Блокируем стандартное выделение Qt
            }

            // Инструмент: Указатель (POINTER)
            if (m_current_tool == InstrumentType::POINTER) {
                // Мы НИЧЕГО не делаем и возвращаем false!
                // Это позволит движку Qt самому понять, что юзер кликнул по фигуре,
                // выделить её и начать перетаскивание.
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

                // normalized() гарантирует правильный прямоугольник, даже если мы тянем мышь вверх-влево
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
                setCurrentTool(m_current_tool); // Возвращаем курсор
                return true;
            }

            if (m_is_drawing && mouseEvent->button() == Qt::LeftButton && m_current_tool == InstrumentType::FIGURE) {
                m_is_drawing = false;
                m_temp_ellipse = nullptr;
                return true;
            }

            if (m_current_tool == InstrumentType::POINTER) return false;
        }
    }

    return QObject::eventFilter(obj, event);
}