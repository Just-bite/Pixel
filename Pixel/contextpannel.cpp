#include "contextpannel.h"

ContextPannel::ContextPannel(QWidget* parent) : QWidget(parent)
{
    // Фиксируем высоту, чтобы панель всегда была одного размера и не прыгала
    setMinimumHeight(65);
    setMaximumHeight(65);

    QHBoxLayout* main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(5, 2, 5, 2);

    // --- GEOMETRY GROUP ---
    m_geometry_group = new QGroupBox("Transform", this);
    QHBoxLayout* geom_layout = new QHBoxLayout(m_geometry_group);
    geom_layout->setContentsMargins(5, 5, 5, 5);

    m_x_box = createSpinBox(-10000, 10000); m_y_box = createSpinBox(-10000, 10000);
    m_w_box = createSpinBox(1, 10000); m_h_box = createSpinBox(1, 10000);
    m_rot_box = createSpinBox(-360, 360);

    addLabeledWidget(geom_layout, "X:", m_x_box); addLabeledWidget(geom_layout, "Y:", m_y_box);
    addLabeledWidget(geom_layout, "W:", m_w_box); addLabeledWidget(geom_layout, "H:", m_h_box);
    addLabeledWidget(geom_layout, "Rot:", m_rot_box);

    // --- STYLE GROUP ---
    m_style_group = new QGroupBox("Style", this);
    QHBoxLayout* style_layout = new QHBoxLayout(m_style_group);
    style_layout->setContentsMargins(5, 5, 5, 5);

    m_type_box = new QComboBox(this);
    m_type_box->addItem("Ellipse", static_cast<int>(FigureType::Ellipse));
    m_type_box->addItem("Rectangle", static_cast<int>(FigureType::Rectangle));

    m_thick_box = createSpinBox(0, 100);

    // Создаем красивые кнопки-квадратики для цвета
    m_btn_fill_color = new QPushButton(this);
    m_btn_fill_color->setFixedSize(24, 24);

    m_btn_stroke_color = new QPushButton(this);
    m_btn_stroke_color->setFixedSize(24, 24);

    addLabeledWidget(style_layout, "Type:", m_type_box);
    addLabeledWidget(style_layout, "Thick:", m_thick_box);

    style_layout->addSpacing(10);
    addLabeledWidget(style_layout, "Fill:", m_btn_fill_color);
    style_layout->addSpacing(5);
    addLabeledWidget(style_layout, "Outline:", m_btn_stroke_color);

    main_layout->addWidget(m_geometry_group);
    main_layout->addWidget(m_style_group);
    main_layout->addStretch();

    m_default_state.fill = Qt::cyan;
    m_default_state.stroke = Qt::black;
    m_default_state.thickness = 2.0f;
    m_default_state.type = FigureType::Ellipse;

    connect(m_btn_fill_color, &QPushButton::clicked, this, &ContextPannel::onColorFillClicked);
    connect(m_btn_stroke_color, &QPushButton::clicked, this, &ContextPannel::onColorStrokeClicked);

    connect(m_type_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ContextPannel::onAnyUIChanged);
    connect(m_x_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_y_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_w_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_h_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_rot_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_thick_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);

    setMode(false, false);
}

void ContextPannel::addLabeledWidget(QHBoxLayout* layout, const QString& text, QWidget* widget) {
    layout->addWidget(new QLabel(text, this));
    layout->addWidget(widget);
}

QDoubleSpinBox* ContextPannel::createSpinBox(double min, double max) {
    QDoubleSpinBox* sb = new QDoubleSpinBox(this);
    sb->setRange(min, max); sb->setDecimals(1); sb->setButtonSymbols(QAbstractSpinBox::NoButtons); sb->setFixedWidth(50);
    return sb;
}

void ContextPannel::setMode(bool isFigureSelected, bool isFigureTool) {
    // Вместо скрытия (что ломает layout), мы делаем блоки недоступными
    m_geometry_group->setEnabled(isFigureSelected);
    m_style_group->setEnabled(isFigureSelected || isFigureTool);

    if (!isFigureSelected && isFigureTool) {
        m_current_target = nullptr;
        blockSignals(true);
        m_type_box->setCurrentIndex(m_type_box->findData(static_cast<int>(m_default_state.type)));
        m_thick_box->setValue(m_default_state.thickness);

        m_x_box->setValue(0); m_y_box->setValue(0);
        m_w_box->setValue(0); m_h_box->setValue(0); m_rot_box->setValue(0);

        updateColorButtonsUI();
        blockSignals(false);
    } else if (!isFigureSelected && !isFigureTool) {
        m_current_target = nullptr;
        blockSignals(true);
        m_x_box->setValue(0); m_y_box->setValue(0);
        m_w_box->setValue(0); m_h_box->setValue(0); m_rot_box->setValue(0); m_thick_box->setValue(0);
        m_btn_fill_color->setStyleSheet("background-color: transparent; border: 1px solid #555;");
        m_btn_stroke_color->setStyleSheet("background-color: transparent; border: 1px solid #555;");
        blockSignals(false);
    }
}

void ContextPannel::setTarget(Figure* figure) {
    m_current_target = figure;
    if (!figure) return;

    blockSignals(true);
    FigureState s = figure->getState();
    m_type_box->setCurrentIndex(m_type_box->findData(static_cast<int>(s.type)));
    m_x_box->setValue(s.pos.x()); m_y_box->setValue(s.pos.y());
    m_w_box->setValue(s.rect.width()); m_h_box->setValue(s.rect.height());
    m_rot_box->setValue(s.rot); m_thick_box->setValue(s.thickness);
    updateColorButtonsUI();
    blockSignals(false);
}

FigureState ContextPannel::getUIState(const FigureState& baseState) const {
    FigureState s = baseState;
    s.type = static_cast<FigureType>(m_type_box->currentData().toInt());
    s.pos = QPointF(m_x_box->value(), m_y_box->value());
    s.rot = m_rot_box->value();
    s.rect = QRectF(-m_w_box->value()/2.0, -m_h_box->value()/2.0, m_w_box->value(), m_h_box->value());
    s.thickness = m_thick_box->value();
    return s;
}

void ContextPannel::setDefaultColor(bool isFill, const QColor& color) {
    if (isFill) m_default_state.fill = color; else m_default_state.stroke = color;
    updateColorButtonsUI();
}

void ContextPannel::updateColorButtonsUI() {
    if (!m_style_group->isEnabled()) return;

    FigureState s = m_current_target ? m_current_target->getState() : m_default_state;

    QString baseFill = QString("background-color: %1;").arg(s.fill.name(QColor::HexArgb));
    QString baseStroke = QString("background-color: %1;").arg(s.stroke.name(QColor::HexArgb));

    QString activeBorder = "border: 2px solid white; outline: 1px solid #ffcc00;";
    QString inactiveBorder = "border: 1px solid #555;";

    m_btn_fill_color->setStyleSheet(baseFill + (m_active_is_fill ? activeBorder : inactiveBorder));
    m_btn_stroke_color->setStyleSheet(baseStroke + (!m_active_is_fill ? activeBorder : inactiveBorder));
}

void ContextPannel::onColorFillClicked() { m_active_is_fill = true; updateColorButtonsUI(); emit colorTargetActivated(true); }
void ContextPannel::onColorStrokeClicked() { m_active_is_fill = false; updateColorButtonsUI(); emit colorTargetActivated(false); }

void ContextPannel::onAnyUIChanged() {
    if (m_current_target) emit propertyChanged();
    else {
        m_default_state.type = static_cast<FigureType>(m_type_box->currentData().toInt());
        m_default_state.thickness = m_thick_box->value();
    }
}