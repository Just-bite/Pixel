#include "workspacecontroller.h"

WorkspaceController::WorkspaceController(QGraphicsView* view, ProjectManager* projectManager, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_project_manager(projectManager)
{
    // Устанавливаем себя как перехватчика событий для View и его Viewport
    if (m_view) {
        m_view->installEventFilter(this);
        m_view->viewport()->installEventFilter(this);
    }
}

bool WorkspaceController::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_view || !m_project_manager) return QObject::eventFilter(obj, event);

    // Клавиатура (Пробел)
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
            m_view->setCursor(Qt::ArrowCursor);
            return true;
        }
    }

    // Мышь на Viewport
    if (obj == m_view->viewport() || obj == m_view) {

        // Зум колесиком
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            int delta = wheelEvent->angleDelta().y();
            double factor = (delta > 0) ? 1.15 : (1.0 / 1.15);
            m_view->scale(factor, factor);

            emit viewportChanged(); // Сообщаем окну обновить инфо-панель
            return true;
        }

        // Клик мыши
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            // Панорамирование
            if ((mouseEvent->button() == Qt::LeftButton && m_space_pressed) || mouseEvent->button() == Qt::MiddleButton) {
                m_is_panning = true;
                m_last_pan_pos = mouseEvent->pos();
                m_view->setCursor(Qt::ClosedHandCursor);
                return true;
            }

            // Рисование эллипса
            if (mouseEvent->button() == Qt::LeftButton) {
                m_is_drawing = true;
                QPointF internalPos = m_view->mapToScene(mouseEvent->pos());
                m_draw_start_pos = internalPos;

                Canvas *canvas = m_project_manager->GetCurrentCanvas();
                if (canvas && canvas->getSelectedLayerid() >= 0) {
                    m_temp_ellipse = new Ellipse(internalPos, 0);
                    canvas->addObjectToSelectedLayer(m_temp_ellipse);
                    //canvas->renderCanvas();
                }
            }
        }

        // Движение мыши
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (m_is_panning) {
                QPoint delta = mouseEvent->pos() - m_last_pan_pos;
                m_view->horizontalScrollBar()->setValue(m_view->horizontalScrollBar()->value() - delta.x());
                m_view->verticalScrollBar()->setValue(m_view->verticalScrollBar()->value() - delta.y());
                m_last_pan_pos = mouseEvent->pos();
                return true;
            }

            if (m_is_drawing && m_temp_ellipse) {
                QPointF internalPos = m_view->mapToScene(mouseEvent->pos());
                Canvas *canvas = m_project_manager->GetCurrentCanvas();

                QPointF center = (m_draw_start_pos + internalPos) / 2.0;
                qreal radius = qAbs(internalPos.x() - m_draw_start_pos.x()) / 2.0;

                m_temp_ellipse->setCenter(center);
                m_temp_ellipse->setRadius(radius);

                //if (canvas) canvas->renderCanvas();
            }
        }

        // Отпускание мыши
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (m_is_panning) {
                m_is_panning = false;
                m_view->setCursor(m_space_pressed ? Qt::OpenHandCursor : Qt::ArrowCursor);
                return true;
            }

            if (m_is_drawing && mouseEvent->button() == Qt::LeftButton) {
                m_is_drawing = false;
                m_temp_ellipse = nullptr;
                qDebug() << "[Controller] Figure finalized.";
            }
        }
    }

    return QObject::eventFilter(obj, event);
}