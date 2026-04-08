#include "include\palettepannel.h"
#include <QRegularExpressionValidator>
#include <QRegularExpression>

ColorPickerArea::ColorPickerArea(QWidget* parent) : QWidget(parent), m_pos(0,0) {
    setMinimumSize(150, 150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    updateImage();
}

void ColorPickerArea::setHue(int hue) {
    m_hue = hue;
    updateImage();
    update();
}

void ColorPickerArea::setCursorColor(const QColor& color) {
    m_hue = std::max(0, color.hsvHue());
    int sat = color.hsvSaturation();
    int val = color.value();

    int x = (sat * (width() - 1)) / 255;
    int y = ((255 - val) * (height() - 1)) / 255;

    m_pos = QPoint(x, y);
    updateImage();
    update();
}

void ColorPickerArea::updateCursorPos() {
    int w = std::max(1, width() - 1);
    int h = std::max(1, height() - 1);
    m_pos = QPoint((m_sat * w) / 255, ((255 - m_val) * h) / 255);
}

QColor ColorPickerArea::currentColor() const { return m_image.isNull() ? Qt::white : m_image.pixelColor(m_pos); }

void ColorPickerArea::paintEvent(QPaintEvent *) {
    QPainter p(this); p.drawImage(rect(), m_image);
    p.setPen(QPen(Qt::white, 2)); p.setBrush(Qt::NoBrush); p.drawEllipse(m_pos, 4, 4);
    p.setPen(QPen(Qt::black, 1)); p.drawEllipse(m_pos, 5, 5);
}
void ColorPickerArea::resizeEvent(QResizeEvent *e) {
    updateCursorPos();
    QWidget::resizeEvent(e);
}
void ColorPickerArea::mousePressEvent(QMouseEvent *e) { pick(e->pos(), false); }
void ColorPickerArea::mouseMoveEvent(QMouseEvent *e) { if (e->buttons() & Qt::LeftButton) pick(e->pos(), false); }
void ColorPickerArea::mouseReleaseEvent(QMouseEvent *e) { pick(e->pos(), true); }

void ColorPickerArea::pick(const QPoint& pos, bool commit) {
    int x = qBound(0, pos.x(), width() - 1);
    int y = qBound(0, pos.y(), height() - 1);
    m_pos = QPoint(x, y);

    m_sat = (x * 255) / std::max(1, width() - 1);
    m_val = 255 - (y * 255) / std::max(1, height() - 1);

    QColor c;
    c.setHsv(m_hue, m_sat, m_val);

    update();
    if (commit) emit commitColor(c); else emit previewColor(c);
}

void ColorPickerArea::updateImage() {
    m_image = QImage(256, 256, QImage::Format_RGB32);
    for (int x = 0; x < 256; ++x) {
        for (int y = 0; y < 256; ++y) {
            QColor c; c.setHsv(m_hue, x, 255 - y);
            m_image.setPixelColor(x, y, c);
        }
    }
}

PalettePannel::PalettePannel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(2, 2, 2, 2);

    m_area = new ColorPickerArea(this);
    main_layout->addWidget(m_area, 1);

    QHBoxLayout* hue_layout = new QHBoxLayout();
    hue_layout->addWidget(new QLabel("Hue:"));
    m_hue_slider = new QSlider(Qt::Horizontal); m_hue_slider->setRange(0, 359);
    hue_layout->addWidget(m_hue_slider); main_layout->addLayout(hue_layout);

    QHBoxLayout* alpha_layout = new QHBoxLayout();
    alpha_layout->addWidget(new QLabel("Alpha:"));
    m_alpha_slider = new QSlider(Qt::Horizontal); m_alpha_slider->setRange(0, 255); m_alpha_slider->setValue(255);
    alpha_layout->addWidget(m_alpha_slider); main_layout->addLayout(alpha_layout);

    QHBoxLayout* hex_layout = new QHBoxLayout();
    hex_layout->addWidget(new QLabel("HEX:"));
    m_hex_edit = new QLineEdit(); m_hex_edit->setPlaceholderText("#RRGGBBAA");
    m_hex_edit->setValidator(new QRegularExpressionValidator(QRegularExpression("^#?[0-9A-Fa-f]{6,8}$"), this));
    hex_layout->addWidget(m_hex_edit); main_layout->addLayout(hex_layout);

    connect(m_area, &ColorPickerArea::previewColor, this, &PalettePannel::onAreaPreview);
    connect(m_area, &ColorPickerArea::commitColor, this, &PalettePannel::onAreaCommit);
    connect(m_hue_slider, &QSlider::valueChanged, this, &PalettePannel::onSlidersChanged);
    connect(m_alpha_slider, &QSlider::valueChanged, this, &PalettePannel::onSlidersChanged);
    connect(m_hue_slider, &QSlider::sliderReleased, this, &PalettePannel::onSliderReleased);
    connect(m_alpha_slider, &QSlider::sliderReleased, this, &PalettePannel::onSliderReleased);
    connect(m_hex_edit, &QLineEdit::editingFinished, this, &PalettePannel::onHexChanged);
}

void PalettePannel::setColor(const QColor& c) {
    if (m_syncing || !c.isValid()) return;
    m_syncing = true;
    m_current_color = c;
    m_hue_slider->setValue(std::max(0, c.hsvHue()));
    m_alpha_slider->setValue(c.alpha());

    m_area->setCursorColor(c);

    QString hex = QString("#%1%2%3%4")
                      .arg(c.red(), 2, 16, QChar('0')).arg(c.green(), 2, 16, QChar('0'))
                      .arg(c.blue(), 2, 16, QChar('0')).arg(c.alpha(), 2, 16, QChar('0')).toUpper();
    m_hex_edit->setText(hex);
    m_syncing = false;
}

void PalettePannel::updateUIFromColor(const QColor& c) { setColor(c); }

QColor PalettePannel::parseHexColor(const QString& hexStr) {
    QString h = hexStr; if (!h.startsWith("#")) h.prepend("#");
    if (h.length() == 7) return QColor(h);
    if (h.length() == 9) {
        int r = h.mid(1, 2).toInt(nullptr, 16), g = h.mid(3, 2).toInt(nullptr, 16);
        int b = h.mid(5, 2).toInt(nullptr, 16), a = h.mid(7, 2).toInt(nullptr, 16);
        return QColor(r, g, b, a);
    }
    return QColor();
}

void PalettePannel::onAreaPreview(const QColor& c) {
    if (m_syncing) return;
    QColor fullColor = c; fullColor.setAlpha(m_alpha_slider->value());
    m_current_color = fullColor; updateUIFromColor(fullColor); emit colorPreviewed(fullColor);
}
void PalettePannel::onAreaCommit(const QColor& c) {
    if (m_syncing) return;
    QColor fullColor = c; fullColor.setAlpha(m_alpha_slider->value());
    m_current_color = fullColor; emit colorCommitted(fullColor);
}

void PalettePannel::onSlidersChanged() {
    if (!m_syncing) {
        m_syncing = true;

        m_area->setHue(m_hue_slider->value());
        QColor cur = m_area->currentColor();
        cur.setAlpha(m_alpha_slider->value());
        m_current_color = cur;

        QString hex = QString("#%1%2%3%4")
                          .arg(cur.red(), 2, 16, QChar('0')).arg(cur.green(), 2, 16, QChar('0'))
                          .arg(cur.blue(), 2, 16, QChar('0')).arg(cur.alpha(), 2, 16, QChar('0')).toUpper();
        m_hex_edit->setText(hex);

        m_syncing = false;
        emit colorPreviewed(cur);
    }
}

void PalettePannel::onSliderReleased() { if (!m_syncing) emit colorCommitted(m_current_color); }
void PalettePannel::onHexChanged() {
    if (m_syncing) return;
    QColor c = parseHexColor(m_hex_edit->text());
    if (c.isValid()) { setColor(c); emit colorCommitted(m_current_color); }
    else updateUIFromColor(m_current_color);
}