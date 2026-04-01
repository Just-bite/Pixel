#include "contextpannel.h"

ContextPannel::ContextPannel(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(65);
    setMaximumHeight(65);

    QHBoxLayout* main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(5, 2, 5, 2);

    m_lbl_placeholder = new QLabel("Tool selected.", this);
    m_lbl_placeholder->setAlignment(Qt::AlignCenter);

    m_lbl_placeholder->setStyleSheet("border: none; background: transparent; font-size: 14px; color: #aaaaaa;");

    m_geometry_group = new QGroupBox("Transform", this);
    QHBoxLayout* geom_layout = new QHBoxLayout(m_geometry_group);
    geom_layout->setContentsMargins(5, 5, 5, 5);
    m_x_box = createSpinBox(-10000, 10000); m_y_box = createSpinBox(-10000, 10000);
    m_w_box = createSpinBox(1, 10000); m_h_box = createSpinBox(1, 10000);
    m_rot_box = createSpinBox(-360, 360);
    addLabeledWidget(geom_layout, "X:", m_x_box);
    addLabeledWidget(geom_layout, "Y:", m_y_box);
    addLabeledWidget(geom_layout, "W:", m_w_box);
    addLabeledWidget(geom_layout, "H:", m_h_box);
    addLabeledWidget(geom_layout, "Rot:", m_rot_box);

    m_style_group = new QGroupBox("Style", this);
    QHBoxLayout* style_layout = new QHBoxLayout(m_style_group);
    style_layout->setContentsMargins(5, 5, 5, 5);
    m_type_box = new QComboBox(this);
    m_type_box->addItem("Ellipse", static_cast<int>(FigureType::Ellipse));
    m_type_box->addItem("Rectangle", static_cast<int>(FigureType::Rectangle));
    m_thick_box = createSpinBox(0, 100);
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

    m_text_group = new QGroupBox("Text", this);
    QHBoxLayout* text_layout = new QHBoxLayout(m_text_group);
    text_layout->setContentsMargins(5, 5, 5, 5);
    m_font_box = new QFontComboBox(this); m_font_box->setFixedWidth(120);
    m_font_size_box = createSpinBox(1, 500);
    addLabeledWidget(text_layout, "Font:", m_font_box);
    addLabeledWidget(text_layout, "Size:", m_font_size_box);
    main_layout->addWidget(m_text_group);

    m_default_text_state.color = Qt::black;
    m_default_text_state.font = QFont("Arial", 16);

    // Убираем QGroupBox (рамку), делаем просто QWidget
    m_layer_group = new QWidget(this);
    QHBoxLayout* layer_layout = new QHBoxLayout(m_layer_group);
    layer_layout->setContentsMargins(5, 10, 5, 5);
    m_btn_layer_down = new QPushButton("Dn", this);
    m_btn_layer_down->setFixedSize(45, 26);
    m_btn_layer_up = new QPushButton("Up", this);
    m_btn_layer_up->setFixedSize(45, 26);
    layer_layout->addWidget(m_btn_layer_down);
    layer_layout->addWidget(m_btn_layer_up);

    // --- Группа Фильтра ---
    m_filter_group = new QGroupBox("Filter", this);
    QHBoxLayout* filter_layout = new QHBoxLayout(m_filter_group);
    filter_layout->setContentsMargins(5, 5, 5, 5);

    m_filter_type_box = new QComboBox(this);
    m_filter_type_box->addItem("None", static_cast<int>(FilterType::None));
    m_filter_type_box->addItem("Grayscale", static_cast<int>(FilterType::Grayscale));
    m_filter_type_box->addItem("Invert", static_cast<int>(FilterType::Invert));
    m_filter_type_box->addItem("Brightness/Contrast", static_cast<int>(FilterType::BrightnessContrast));
    m_filter_type_box->addItem("Blur", static_cast<int>(FilterType::Blur));

    m_filter_param1 = createSpinBox(-100, 100);
    m_filter_param2 = createSpinBox(-100, 100);

    m_lbl_filter_p1 = new QLabel("P1:", this);
    m_lbl_filter_p2 = new QLabel("P2:", this);

    filter_layout->addWidget(new QLabel("Type:", this)); filter_layout->addWidget(m_filter_type_box);
    filter_layout->addWidget(m_lbl_filter_p1); filter_layout->addWidget(m_filter_param1);
    filter_layout->addWidget(m_lbl_filter_p2); filter_layout->addWidget(m_filter_param2);



    main_layout->addWidget(m_lbl_placeholder);
    main_layout->addWidget(m_geometry_group);
    main_layout->addWidget(m_style_group);
    main_layout->addWidget(m_layer_group);
    main_layout->addWidget(m_filter_group);
    main_layout->addStretch();

    m_default_state.fill = Qt::cyan;
    m_default_state.stroke = Qt::black;
    m_default_state.thickness = 2.0f;
    m_default_state.type = FigureType::Ellipse;

    m_default_state.pos = QPointF(0, 0);
    m_default_state.rot = 0.0;
    m_default_state.rect = QRectF(0, 0, 0, 0);

    connect(m_btn_fill_color, &QPushButton::clicked, this, &ContextPannel::onColorFillClicked);
    connect(m_btn_stroke_color, &QPushButton::clicked, this, &ContextPannel::onColorStrokeClicked);
    connect(m_btn_layer_up, &QPushButton::clicked, this, &ContextPannel::onMoveUpClicked);
    connect(m_btn_layer_down, &QPushButton::clicked, this, &ContextPannel::onMoveDownClicked);

    void(QComboBox::*comboSignal)(int) = &QComboBox::currentIndexChanged;
    connect(m_type_box, comboSignal, this, &ContextPannel::onAnyUIChanged);
    connect(m_x_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_y_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_w_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_h_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_rot_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_thick_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_font_box, &QFontComboBox::currentFontChanged, this, &ContextPannel::onAnyUIChanged);
    connect(m_font_size_box, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onAnyUIChanged);
    connect(m_filter_type_box, comboSignal, this, &ContextPannel::onFilterUIChanged);
    connect(m_filter_param1, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onFilterUIChanged);
    connect(m_filter_param2, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onFilterUIChanged);

    setMode(false, false, false, false, "Pointer");
}

void ContextPannel::addLabeledWidget(QHBoxLayout* layout, const QString& text, QWidget* widget) {
    layout->addWidget(new QLabel(text, this)); layout->addWidget(widget);
}

QDoubleSpinBox* ContextPannel::createSpinBox(double min, double max) {
    QDoubleSpinBox* sb = new QDoubleSpinBox(this);
    sb->setRange(min, max); sb->setDecimals(1); sb->setButtonSymbols(QAbstractSpinBox::NoButtons); sb->setFixedWidth(50);
    return sb;
}

void ContextPannel::setMode(bool isFigSel, bool isTextSel, bool isFigTool, bool isTextTool, const QString& toolName) {
    bool isFilterSel = (m_current_filter_target != nullptr); // <--- Добавили эту строку
    bool isImgSel = (m_current_image_target != nullptr);
    bool somethingSelected = isFigSel || isTextSel || isImgSel || isFilterSel; // <--- Обновили условие

    m_lbl_placeholder->setVisible(!somethingSelected && !isFigTool && !isTextTool);
    if (!toolName.isEmpty()) m_lbl_placeholder->setText(QString("Tool: %1. No specific settings.").arg(toolName));

    m_filter_group->setVisible(isFilterSel);
    m_geometry_group->setVisible(somethingSelected && !isFilterSel);
    m_style_group->setVisible((isFigSel || isFigTool) && !isFilterSel);
    m_text_group->setVisible((isTextSel || isTextTool) && !isFilterSel);
    m_layer_group->setVisible(somethingSelected);

    if (!somethingSelected) {
        m_current_target = nullptr;
        m_current_text_target = nullptr;
        m_current_image_target = nullptr;
        m_current_filter_target = nullptr;

        blockSignals(true);
        if (isTextTool) {
            m_font_box->setCurrentFont(m_default_text_state.font);
            m_font_size_box->setValue(m_default_text_state.font.pointSizeF());
        }
        updateColorButtonsUI();
        blockSignals(false);
    }
}

void ContextPannel::setTarget(Figure* figure) {
    m_current_target = figure; m_current_text_target = nullptr;
    m_current_image_target = nullptr; m_current_filter_target = nullptr;
    if (!figure) return;
    blockSignals(true);
    FigureState s = figure->getState();
    m_style_group->setVisible(true);
    m_type_box->setCurrentIndex(m_type_box->findData(static_cast<int>(s.type)));
    m_thick_box->setValue(s.thickness);
    m_x_box->setValue(s.pos.x()); m_y_box->setValue(s.pos.y());
    m_w_box->setValue(s.rect.width()); m_h_box->setValue(s.rect.height());
    m_rot_box->setValue(s.rot);
    updateColorButtonsUI();
    blockSignals(false);
}

void ContextPannel::setTarget(TextObject* textObj) {
    m_current_target = nullptr; m_current_text_target = textObj;
    m_current_image_target = nullptr; m_current_filter_target = nullptr;

    if (!textObj) return;
    blockSignals(true);
    TextState s = textObj->getState();
    m_font_box->setCurrentFont(s.font);
    m_font_size_box->setValue(s.font.pointSizeF());
    m_x_box->setValue(s.pos.x()); m_y_box->setValue(s.pos.y());
    m_w_box->setValue(s.rect.width()); m_h_box->setValue(s.rect.height());
    m_rot_box->setValue(s.rot);
    updateColorButtonsUI();
    blockSignals(false);
}

void ContextPannel::setTarget(ImageObject* imageObj) {
    m_current_target = nullptr; m_current_text_target = nullptr;
    m_current_image_target = imageObj; m_current_filter_target = nullptr;
    if (!imageObj) return;
    blockSignals(true);
    ImageState s = imageObj->getState();
    m_x_box->setValue(s.pos.x()); m_y_box->setValue(s.pos.y());
    m_w_box->setValue(s.rect.width()); m_h_box->setValue(s.rect.height());
    m_rot_box->setValue(s.rot);
    blockSignals(false);
}

void ContextPannel::setTarget(FilterLayer* filterObj) {
    m_current_target = nullptr; m_current_text_target = nullptr;
    m_current_image_target = nullptr; m_current_filter_target = filterObj;
    if (!filterObj) return;

    blockSignals(true);
    FilterState s = filterObj->getFilterState();
    m_filter_type_box->setCurrentIndex(m_filter_type_box->findData(static_cast<int>(s.type)));
    m_filter_param1->setValue(s.param1);
    m_filter_param2->setValue(s.param2);
    updateFilterUIRanges();
    blockSignals(false);
}

void ContextPannel::onFilterUIChanged() {
    updateFilterUIRanges();
    if (m_current_filter_target)
        emit propertyChanged();
}

TextState ContextPannel::getUITextState(const TextState& baseState) const {
    TextState s = baseState;
    s.font = m_font_box->currentFont();
    s.font.setPointSizeF(m_font_size_box->value());
    s.pos = QPointF(m_x_box->value(), m_y_box->value()); s.rot = m_rot_box->value();
    s.rect = QRectF(-m_w_box->value()/2.0, -m_h_box->value()/2.0, m_w_box->value(), m_h_box->value());
    return s;
}

FigureState ContextPannel::getUIState(const FigureState& baseState) const {
    FigureState s = baseState;
    s.type = static_cast<FigureType>(m_type_box->currentData().toInt());
    s.thickness = m_thick_box->value();
    s.pos = QPointF(m_x_box->value(), m_y_box->value());
    s.rot = m_rot_box->value();
    s.rect = QRectF(-m_w_box->value()/2.0, -m_h_box->value()/2.0, m_w_box->value(), m_h_box->value());
    return s;
}

ImageState ContextPannel::getUIImageState(const ImageState& baseState) const {
    ImageState s = baseState;
    s.pos = QPointF(m_x_box->value(), m_y_box->value()); s.rot = m_rot_box->value();
    s.rect = QRectF(-m_w_box->value()/2.0, -m_h_box->value()/2.0, m_w_box->value(), m_h_box->value());
    return s;
}

FilterState ContextPannel::getUIFilterState() const {
    FilterState s;
    s.type = static_cast<FilterType>(m_filter_type_box->currentData().toInt());
    s.param1 = m_filter_param1->value();
    s.param2 = m_filter_param2->value();
    return s;
}

void ContextPannel::updateFilterUIRanges() {
    FilterType type = static_cast<FilterType>(m_filter_type_box->currentData().toInt());

    if (type == FilterType::Blur) {
        m_lbl_filter_p1->setText("Radius:"); m_lbl_filter_p1->show();
        m_filter_param1->setRange(0, 50);    m_filter_param1->show();
        m_lbl_filter_p2->hide();             m_filter_param2->hide();
    } else if (type == FilterType::BrightnessContrast) {
        m_lbl_filter_p1->setText("Bright:"); m_lbl_filter_p1->show();
        m_filter_param1->setRange(-100, 100);m_filter_param1->show();
        m_lbl_filter_p2->setText("Contr:");  m_lbl_filter_p2->show();
        m_filter_param2->setRange(-100, 100);m_filter_param2->show();
    } else {
        m_lbl_filter_p1->hide(); m_filter_param1->hide();
        m_lbl_filter_p2->hide(); m_filter_param2->hide();
    }
}

void ContextPannel::setDefaultColor(bool isFill, const QColor& color) {
    if (isFill)
        m_default_state.fill = color;
    else
        m_default_state.stroke = color;
    updateColorButtonsUI();
}

QColor ContextPannel::getActiveColor() const {
    if (m_current_text_target)
        return m_current_text_target->getState().color;
    FigureState s = m_current_target ? m_current_target->getState() : m_default_state;
    return m_active_is_fill ? s.fill : s.stroke;
}

void ContextPannel::updateColorButtonsUI() {
    FigureState s = m_current_target ? m_current_target->getState() : m_default_state;
    auto getContrastBorder = [](const QColor& c) {
        double luminance = (0.2126 * c.red() + 0.7152 * c.green() + 0.0722 * c.blue());
        return luminance > 128 ? "black" : "white";
    };

    QString fillBorder = m_active_is_fill ? QString("border: 2px solid %1;").arg(getContrastBorder(s.fill)) : "border: 1px solid #555;";
    QString strokeBorder = !m_active_is_fill ? QString("border: 2px solid %1;").arg(getContrastBorder(s.stroke)) : "border: 1px solid #555;";

    m_btn_fill_color->setStyleSheet(QString("background-color: %1; %2").arg(s.fill.name(QColor::HexArgb)).arg(fillBorder));
    m_btn_stroke_color->setStyleSheet(QString("background-color: %1; %2").arg(s.stroke.name(QColor::HexArgb)).arg(strokeBorder));
}

void ContextPannel::onColorFillClicked() {
    m_active_is_fill = true;
    updateColorButtonsUI();
    emit colorTargetActivated(true);
}
void ContextPannel::onColorStrokeClicked() {
    m_active_is_fill = false;
    updateColorButtonsUI();
    emit colorTargetActivated(false);
}
void ContextPannel::onMoveUpClicked() {
    emit moveObjectLayerRequested(1);
}
void ContextPannel::onMoveDownClicked() {
    emit moveObjectLayerRequested(-1);
}
void ContextPannel::onAnyUIChanged() {
    if (m_current_target || m_current_text_target || m_current_image_target)
        emit propertyChanged();
    else {
        m_default_state.type = static_cast<FigureType>(m_type_box->currentData().toInt());
        m_default_state.thickness = m_thick_box->value();
    }
}