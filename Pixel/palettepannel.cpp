#include "palettepannel.h"
#include <QtGlobal> // Для qBound

PalettePannel::PalettePannel(QWidget* parent)
    : QWidget(parent), m_current_hue(0), m_selected_pos(0, 0)
{
}

void PalettePannel::setHue(int hue)
{
    if (hue != m_current_hue) {
        m_current_hue = hue;
        updateColorImage();
        pickColor(m_selected_pos); // Обновляем цвет при движении ползунка
        update();
    }
}

void PalettePannel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), m_color_palette);

    // Рисуем кружок выбора
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(m_selected_pos, 4, 4);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawEllipse(m_selected_pos, 5, 5);
}

void PalettePannel::resizeEvent(QResizeEvent *event)
{
    updateColorImage();
    QWidget::resizeEvent(event);
}

void PalettePannel::mousePressEvent(QMouseEvent *event) { pickColor(event->pos()); }

void PalettePannel::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) pickColor(event->pos());
}

void PalettePannel::pickColor(const QPoint& pos)
{
    // ИСПРАВЛЕНИЕ: Используем qBound вместо std::clamp
    int x = qBound(0, pos.x(), width() - 1);
    int y = qBound(0, pos.y(), height() - 1);
    m_selected_pos = QPoint(x, y);

    if (!m_color_palette.isNull()) {
        emit colorSelected(m_color_palette.pixelColor(x, y));
        update();
    }
}

void PalettePannel::updateColorImage()
{
    int w = width();
    int h = height();
    if (w <= 0 || h <= 0) return;

    m_color_palette = QImage(w, h, QImage::Format_ARGB32);
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            int saturation = (x * 255) / w;
            int value = 255 - (y * 255) / h;
            QColor color;
            color.setHsv(m_current_hue, saturation, value);
            m_color_palette.setPixelColor(x, y, color);
        }
    }
}