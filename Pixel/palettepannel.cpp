#include "palettepannel.h"

PalettePannel::PalettePannel(QWidget* parent)
    : QWidget(parent)
    , m_current_hue(0)
{
}

void PalettePannel::setHue(int hue)
{
    if (hue != m_current_hue) {
        m_current_hue = hue;
        updateColorImage();
        update();
    }
}

void PalettePannel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), m_color_palette);
}



void PalettePannel::resizeEvent(QResizeEvent *event)
{
    updateColorImage();
    QWidget::resizeEvent(event);
}

void PalettePannel::updateColorImage()
{
    int w = width();
    int h = height();
    if (w <= 0 || h <= 0) return;

    int paletteHeight = h;
    if (paletteHeight <= 0) paletteHeight = 1;

    m_color_palette = QImage(w, paletteHeight, QImage::Format_ARGB32);

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < paletteHeight; ++y) {
            int saturation = (x * 255) / w;
            int value = 255 - (y * 255) / paletteHeight;

            QColor color;
            color.setHsv(m_current_hue, saturation, value);
            m_color_palette.setPixelColor(x, y, color);
        }
    }
}
