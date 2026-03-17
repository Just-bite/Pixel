#include "palettepannel.h"
#include <QtGlobal>
#include <QRegularExpressionValidator>

// --- ColorPickerArea ---
ColorPickerArea::ColorPickerArea(QWidget* parent) : QWidget(parent), m_pos(0,0) {
    setMinimumSize(150, 150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ColorPickerArea::setHue(int hue) {
    m_hue = hue;
    updateImage();
    update();
    pick(m_pos, false);
}

QColor ColorPickerArea::currentColor() const {
    if (m_image.isNull()) return Qt::white;
    return m_image.pixelColor(m_pos);
}

void ColorPickerArea::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.drawImage(rect(), m_image);
    p.setPen(QPen(Qt::white, 2)); p.setBrush(Qt::NoBrush);
    p.drawEllipse(m_pos, 4, 4);
    p.setPen(QPen(Qt::black, 1));
    p.drawEllipse(m_pos, 5, 5);
}

void ColorPickerArea::resizeEvent(QResizeEvent *e) {
    updateImage();
    QWidget::resizeEvent(e);
}

void ColorPickerArea::mousePressEvent(QMouseEvent *e) { pick(e->pos(), false); }
void ColorPickerArea::mouseMoveEvent(QMouseEvent *e) { if (e->buttons() & Qt::LeftButton) pick(e->pos(), false); }
void ColorPickerArea::mouseReleaseEvent(QMouseEvent *e) { pick(e->pos(), true); }

void ColorPickerArea::pick(const QPoint& pos, bool commit) {
    int x = qBound(0, pos.x(), width() - 1);
    int y = qBound(0, pos.y(), height() - 1);
    m_pos = QPoint(x, y);

    if (!m_image.isNull()) {
        QColor c = m_image.pixelColor(x, y);
        update();
        if (commit) emit commitColor(c);
        else emit previewColor(c);
    }
}

void ColorPickerArea::updateImage() {
    int w = width(), h = height();
    if (w <= 0 || h <= 0) return;
    m_image = QImage(w, h, QImage::Format_RGB32);
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            int sat = (x * 255) / w;
            int val = 255 - (y * 255) / h;
            QColor c; c.setHsv(m_hue, sat, val);
            m_image.setPixelColor(x, y, c);
        }
    }
}

// --- PalettePannel ---
PalettePannel::PalettePannel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(2, 2, 2, 2);

    m_area = new ColorPickerArea(this);
    main_layout->addWidget(m_area, 1); // 1 = Забирает всё свободное место, оставаясь квадратным

    // --- Слайдер Hue ---
    QHBoxLayout* hue_layout = new QHBoxLayout();
    hue_layout->addWidget(new QLabel("Hue:", this));
    m_hue_slider = new QSlider(Qt::Horizontal, this);
    m_hue_slider->setRange(0, 359);
    hue_layout->addWidget(m_hue_slider);
    main_layout->addLayout(hue_layout);

    // --- Слайдер Alpha ---
    QHBoxLayout* alpha_layout = new QHBoxLayout();
    alpha_layout->addWidget(new QLabel("Alpha:", this));
    m_alpha_slider = new QSlider(Qt::Horizontal, this);
    m_alpha_slider->setRange(0, 255);
    m_alpha_slider->setValue(255);
    alpha_layout->addWidget(m_alpha_slider);
    main_layout->addLayout(alpha_layout);

    // --- Поле HEX ---
    QHBoxLayout* hex_layout = new QHBoxLayout();
    hex_layout->addWidget(new QLabel("HEX:", this));
    m_hex_edit = new QLineEdit(this);
    m_hex_edit->setPlaceholderText("#RRGGBBAA");

    // Валидатор: # и от 6 до 8 HEX-символов
    QRegularExpression rx("^#?[0-9A-Fa-f]{6,8}$");
    m_hex_edit->setValidator(new QRegularExpressionValidator(rx, this));

    hex_layout->addWidget(m_hex_edit);
    main_layout->addLayout(hex_layout);

    connect(m_area, &ColorPickerArea::previewColor, this, &PalettePannel::onAreaPreview);
    connect(m_area, &ColorPickerArea::commitColor, this, &PalettePannel::onAreaCommit);

    connect(m_hue_slider, &QSlider::valueChanged, this, &PalettePannel::onSlidersChanged);
    connect(m_alpha_slider, &QSlider::valueChanged, this, &PalettePannel::onSlidersChanged);
    connect(m_hue_slider, &QSlider::sliderReleased, this, &PalettePannel::onSliderReleased);
    connect(m_alpha_slider, &QSlider::sliderReleased, this, &PalettePannel::onSliderReleased);

    connect(m_hex_edit, &QLineEdit::editingFinished, this, &PalettePannel::onHexChanged);
}

void PalettePannel::updateUIFromColor(const QColor& c) {
    if (m_syncing) return;
    m_syncing = true;
    m_alpha_slider->setValue(c.alpha());

    // Форматируем в #RRGGBBAA руками для совместимости с C++11 / старым Qt
    QString hex = QString("#%1%2%3%4")
                      .arg(c.red(), 2, 16, QChar('0'))
                      .arg(c.green(), 2, 16, QChar('0'))
                      .arg(c.blue(), 2, 16, QChar('0'))
                      .arg(c.alpha(), 2, 16, QChar('0')).toUpper();

    m_hex_edit->setText(hex);
    m_syncing = false;
}

QColor PalettePannel::parseHexColor(const QString& hexStr) {
    QString h = hexStr;
    if (!h.startsWith("#")) h.prepend("#");

    if (h.length() == 7) { // #RRGGBB
        return QColor(h);
    } else if (h.length() == 9) { // #RRGGBBAA
        int r = h.mid(1, 2).toInt(nullptr, 16);
        int g = h.mid(3, 2).toInt(nullptr, 16);
        int b = h.mid(5, 2).toInt(nullptr, 16);
        int a = h.mid(7, 2).toInt(nullptr, 16);
        return QColor(r, g, b, a);
    }
    return QColor(); // Invalid
}

void PalettePannel::onAreaPreview(const QColor& c) {
    QColor fullColor = c; fullColor.setAlpha(m_alpha_slider->value());
    m_current_color = fullColor;
    updateUIFromColor(fullColor);
    emit colorPreviewed(fullColor);
}

void PalettePannel::onAreaCommit(const QColor& c) {
    QColor fullColor = c; fullColor.setAlpha(m_alpha_slider->value());
    m_current_color = fullColor;
    emit colorCommitted(fullColor);
}

void PalettePannel::onSlidersChanged() {
    if (m_syncing) return;
    m_area->setHue(m_hue_slider->value());
}

void PalettePannel::onSliderReleased() {
    emit colorCommitted(m_current_color);
}

void PalettePannel::onHexChanged() {
    if (m_syncing) return;
    QColor c = parseHexColor(m_hex_edit->text());

    if (c.isValid()) {
        m_current_color = c;
        m_syncing = true;
        m_hue_slider->setValue(c.hue() >= 0 ? c.hue() : 0);
        m_alpha_slider->setValue(c.alpha());
        m_syncing = false;

        m_area->setHue(m_hue_slider->value());
        emit colorCommitted(m_current_color);
    } else {
        // Возвращаем старый текст, если введен бред
        updateUIFromColor(m_current_color);
    }
}