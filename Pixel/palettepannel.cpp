#include "palettepannel.h"

PalettePannel::PalettePannel(QWidget* parent)
    : QWidget(parent)
    , current_hue(0)
{
    layout = new QVBoxLayout(this);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, 359);
    slider->setValue(0);

    layout->addWidget(slider);
    connect(slider, &QSlider::valueChanged, this, &PalettePannel::setHue);
}

void PalettePannel::setHue(int hue)
{
    if (hue != current_hue) {
        current_hue = hue;
        updateColorImage();
        update();
    }
}

void PalettePannel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), color_palette);
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

    int sliderHeight = slider ? slider->height() : 30;
    int paletteHeight = h - sliderHeight - layout->spacing();
    if (paletteHeight <= 0) paletteHeight = 1;

    color_palette = QImage(w, paletteHeight, QImage::Format_ARGB32);

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < paletteHeight; ++y) {
            int saturation = (x * 255) / w;
            int value = 255 - (y * 255) / paletteHeight;

            QColor color;
            color.setHsv(current_hue, saturation, value);
            color_palette.setPixelColor(x, y, color);
        }
    }
}
