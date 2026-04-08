#include "include\contextpannel.h"

ContextPannel::ContextPannel(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(65);
    setMaximumHeight(65);

    QHBoxLayout* main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(5, 2, 5, 2);

    m_lbl_placeholder = new QLabel("Tool selected.", this);
    m_lbl_placeholder->setAlignment(Qt::AlignCenter);
    m_lbl_placeholder->setStyleSheet("border: none; background: transparent; font-size: 14px; color: #aaaaaa;");
    m_lbl_placeholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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

    m_layer_group = new QWidget(this);
    QHBoxLayout* layer_layout = new QHBoxLayout(m_layer_group);
    layer_layout->setContentsMargins(5, 10, 5, 5);
    m_btn_layer_down = new QPushButton("Dn", this);
    m_btn_layer_down->setFixedSize(45, 26);
    m_btn_layer_up = new QPushButton("Up", this);
    m_btn_layer_up->setFixedSize(45, 26);
    layer_layout->addWidget(m_btn_layer_down);
    layer_layout->addWidget(m_btn_layer_up);

    m_raster_group = new QGroupBox("Brush", this);
    QHBoxLayout* raster_layout = new QHBoxLayout(m_raster_group);
    raster_layout->setContentsMargins(5, 5, 5, 5);
    m_raster_radius = new QSpinBox(this); m_raster_radius->setRange(1, 500); m_raster_radius->setValue(10);
    m_raster_density = new QSpinBox(this); m_raster_density->setRange(1, 100); m_raster_density->setValue(100);
    m_raster_hardness = new QSpinBox(this); m_raster_hardness->setRange(1, 100); m_raster_hardness->setValue(100);
    addLabeledWidget(raster_layout, "Radius:", m_raster_radius);
    addLabeledWidget(raster_layout, "Density %:", m_raster_density);
    addLabeledWidget(raster_layout, "Hardness %:", m_raster_hardness);

    m_fill_group = new QGroupBox("Fill", this);
    QHBoxLayout* fill_layout = new QHBoxLayout(m_fill_group);
    fill_layout->setContentsMargins(5, 5, 5, 5);
    m_fill_tolerance = new QSpinBox(this); m_fill_tolerance->setRange(0, 255); m_fill_tolerance->setValue(0);
    addLabeledWidget(fill_layout, "Tolerance:", m_fill_tolerance);

    m_filter_group = new QGroupBox("Filter", this);
    QHBoxLayout* filter_layout = new QHBoxLayout(m_filter_group);
    filter_layout->setContentsMargins(5, 5, 5, 5);
    m_filter_type_box = new QComboBox(this);
    m_filter_type_box->addItem("None", static_cast<int>(FilterType::None));
    m_filter_type_box->addItem("Grayscale", static_cast<int>(FilterType::Grayscale));
    m_filter_type_box->addItem("Invert", static_cast<int>(FilterType::Invert));
    m_filter_type_box->addItem("Brightness / Contrast", static_cast<int>(FilterType::BrightnessContrast));
    m_filter_type_box->addItem("Blur", static_cast<int>(FilterType::Blur));
    m_filter_type_box->addItem("Threshold", static_cast<int>(FilterType::Threshold));
    m_filter_type_box->addItem("Sharpen", static_cast<int>(FilterType::Sharpen));
    m_filter_type_box->addItem("HSL Color", static_cast<int>(FilterType::HSL));
    filter_layout->addWidget(new QLabel("Type:", this));
    filter_layout->addWidget(m_filter_type_box);
    m_filter_params_container = new QWidget(this);
    m_filter_params_layout = new QHBoxLayout(m_filter_params_container);
    m_filter_params_layout->setContentsMargins(0, 0, 0, 0);
    filter_layout->addWidget(m_filter_params_container);

    main_layout->addWidget(m_lbl_placeholder);
    main_layout->addWidget(m_geometry_group);
    main_layout->addWidget(m_style_group);
    main_layout->addWidget(m_layer_group);
    main_layout->addWidget(m_text_group);
    main_layout->addWidget(m_raster_group);
    main_layout->addWidget(m_fill_group);
    main_layout->addWidget(m_filter_group);
    main_layout->addStretch();

    m_default_state.fill = Qt::red;
    m_default_state.stroke = Qt::black;
    m_default_state.thickness = 2.0f;
    m_default_state.type = FigureType::Ellipse;

    m_default_text_state.color = Qt::black;
    m_default_text_state.font = QFont("Arial", 20);

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
    connect(m_raster_radius, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ContextPannel::onAnyUIChanged);
    connect(m_raster_density, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ContextPannel::onAnyUIChanged);
    connect(m_raster_hardness, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ContextPannel::onAnyUIChanged);
    connect(m_filter_type_box, comboSignal, this, &ContextPannel::onFilterTypeChanged);

    setActiveTool(InstrumentType::POINTER);
}

void ContextPannel::addLabeledWidget(QHBoxLayout* layout, const QString& text, QWidget* widget) {
    layout->addWidget(new QLabel(text, this)); layout->addWidget(widget);
}

QDoubleSpinBox* ContextPannel::createSpinBox(double min, double max) {
    QDoubleSpinBox* sb = new QDoubleSpinBox(this);
    sb->setRange(min, max); sb->setDecimals(1); sb->setButtonSymbols(QAbstractSpinBox::NoButtons); sb->setFixedWidth(50);
    return sb;
}

void ContextPannel::setActiveTool(InstrumentType tool) {
    m_current_tool = tool;
    updateUI();
}

void ContextPannel::clearTargets() {
    m_current_target = nullptr;
    m_current_text_target = nullptr;
    m_current_image_target = nullptr;
    m_current_filter_target = nullptr;
    updateUI();
}

void ContextPannel::updateUI() {
    bool isFilterSel = (m_current_filter_target != nullptr);
    bool isImgSel = (m_current_image_target != nullptr);
    bool isFigSel = (m_current_target != nullptr);
    bool isTextSel = (m_current_text_target != nullptr);
    bool somethingSelected = isFigSel || isTextSel || isImgSel || isFilterSel;

    bool isFigTool = (m_current_tool == InstrumentType::FIGURE);
    bool isTextTool = (m_current_tool == InstrumentType::TEXT);
    bool isRasterTool = (m_current_tool == InstrumentType::PENCIL || m_current_tool == InstrumentType::ERASER);
    bool isFillTool = (m_current_tool == InstrumentType::FILL);

    m_filter_group->setVisible(isFilterSel);
    m_geometry_group->setVisible(somethingSelected && !isFilterSel);
    m_style_group->setVisible((isFigSel || isFigTool) && !isFilterSel);
    m_text_group->setVisible((isTextSel || isTextTool) && !isFilterSel);
    m_raster_group->setVisible(isRasterTool);
    m_fill_group->setVisible(isFillTool);
    m_layer_group->setVisible(somethingSelected);

    bool anyGroupVisible = m_filter_group->isVisible() || m_geometry_group->isVisible() ||
                           m_style_group->isVisible() || m_text_group->isVisible() ||
                           m_raster_group->isVisible() || m_fill_group->isVisible();

    m_lbl_placeholder->setVisible(!anyGroupVisible);

    if (!anyGroupVisible) {
        QString toolName = "Pointer";
        switch(m_current_tool) {
        case InstrumentType::HAND: toolName = "Hand"; break;
        case InstrumentType::FIGURE: toolName = "Figure"; break;
        case InstrumentType::TEXT: toolName = "Text"; break;
        case InstrumentType::PENCIL: toolName = "Pencil"; break;
        case InstrumentType::ERASER: toolName = "Eraser"; break;
        case InstrumentType::FILL: toolName = "Fill"; break;
        case InstrumentType::POINTER: toolName = "Pointer"; break;
        default: break;
        }
        m_lbl_placeholder->setText(QString("Tool: %1. No specific settings.").arg(toolName));
    }

    if (!somethingSelected) {
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
    if (figure) {
        blockSignals(true);
        FigureState s = figure->getState();
        m_type_box->setCurrentIndex(m_type_box->findData(static_cast<int>(s.type)));
        m_thick_box->setValue(s.thickness);
        m_x_box->setValue(s.pos.x()); m_y_box->setValue(s.pos.y());
        m_w_box->setValue(s.rect.width()); m_h_box->setValue(s.rect.height());
        m_rot_box->setValue(s.rot);
        blockSignals(false);
    }
    updateUI();
}

void ContextPannel::setTarget(TextObject* textObj) {
    m_current_target = nullptr; m_current_text_target = textObj;
    m_current_image_target = nullptr; m_current_filter_target = nullptr;
    if (textObj) {
        blockSignals(true);
        TextState s = textObj->getState();
        m_font_box->setCurrentFont(s.font);
        m_font_size_box->setValue(s.font.pointSizeF());
        m_x_box->setValue(s.pos.x()); m_y_box->setValue(s.pos.y());
        m_w_box->setValue(s.rect.width()); m_h_box->setValue(s.rect.height());
        m_rot_box->setValue(s.rot);
        blockSignals(false);
    }
    updateUI();
}

void ContextPannel::setTarget(ImageObject* imageObj) {
    m_current_target = nullptr; m_current_text_target = nullptr;
    m_current_image_target = imageObj; m_current_filter_target = nullptr;
    if (imageObj) {
        blockSignals(true);
        ImageState s = imageObj->getState();
        m_x_box->setValue(s.pos.x()); m_y_box->setValue(s.pos.y());
        m_w_box->setValue(s.rect.width()); m_h_box->setValue(s.rect.height());
        m_rot_box->setValue(s.rot);
        blockSignals(false);
    }
    updateUI();
}

void ContextPannel::setTarget(FilterLayer* filterObj) {
    m_current_target = nullptr; m_current_text_target = nullptr;
    m_current_image_target = nullptr; m_current_filter_target = filterObj;
    if (filterObj) {
        blockSignals(true);
        FilterState s = filterObj->getFilterState();
        m_filter_type_box->setCurrentIndex(m_filter_type_box->findData(static_cast<int>(s.type)));
        rebuildFilterParamsUI(s.type);
        for (size_t i = 0; i < m_filter_param_boxes.size() && i < s.params.size(); ++i) {
            m_filter_param_boxes[i]->setValue(s.params[i]);
        }
        blockSignals(false);
    }
    updateUI();
}

void ContextPannel::rebuildFilterParamsUI(FilterType type) {
    QLayoutItem *child;
    while ((child = m_filter_params_layout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
    m_filter_param_boxes.clear();

    std::vector<FilterParamInfo> infos = FilterFactory::getParamInfo(type);
    for (const auto& info : infos) {
        m_filter_params_layout->addWidget(new QLabel(info.name + ":", this));
        QDoubleSpinBox* spin = createSpinBox(info.minVal, info.maxVal);
        spin->setValue(info.defaultVal);
        m_filter_params_layout->addWidget(spin);
        m_filter_param_boxes.push_back(spin);

        connect(spin, &QDoubleSpinBox::editingFinished, this, &ContextPannel::onFilterParamChanged);
    }
}

void ContextPannel::onFilterTypeChanged() {
    FilterType type = static_cast<FilterType>(m_filter_type_box->currentData().toInt());
    rebuildFilterParamsUI(type);
    if (m_current_filter_target) emit propertyChanged();
}

void ContextPannel::onFilterParamChanged() {
    if (m_current_filter_target) emit propertyChanged();
}

FilterState ContextPannel::getUIFilterState() const {
    FilterState s;
    s.type = static_cast<FilterType>(m_filter_type_box->currentData().toInt());
    for (QDoubleSpinBox* box : m_filter_param_boxes) {
        s.params.push_back(box->value());
    }
    return s;
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

void ContextPannel::setDefaultColor(bool isFill, const QColor& color) {
    if (isFill) m_default_state.fill = color; else m_default_state.stroke = color;
    updateColorButtonsUI();
}

QColor ContextPannel::getActiveColor() const {
    if (m_current_text_target) return m_current_text_target->getState().color;
    FigureState s = m_current_target ? m_current_target->getState() : m_default_state;
    return m_active_is_fill ? s.fill : s.stroke;
}

void ContextPannel::updateColorButtonsUI() {
    FigureState s = m_current_target ? m_current_target->getState() : m_default_state;
    auto getContrastBorder = [](const QColor& c) {
        return (0.2126 * c.red() + 0.7152 * c.green() + 0.0722 * c.blue()) > 128 ? "black" : "white";
    };

    QString fillBorder = m_active_is_fill ? QString("border: 2px solid %1;").arg(getContrastBorder(s.fill)) : "border: 1px solid #555;";
    QString strokeBorder = !m_active_is_fill ? QString("border: 2px solid %1;").arg(getContrastBorder(s.stroke)) : "border: 1px solid #555;";

    m_btn_fill_color->setStyleSheet(QString("background-color: %1; %2").arg(s.fill.name(QColor::HexArgb)).arg(fillBorder));
    m_btn_stroke_color->setStyleSheet(QString("background-color: %1; %2").arg(s.stroke.name(QColor::HexArgb)).arg(strokeBorder));
}

void ContextPannel::setRasterSettings(int radius, int density, int hardness) {
    blockSignals(true);
    m_raster_radius->setValue(radius);
    m_raster_density->setValue(density);
    m_raster_hardness->setValue(hardness);
    blockSignals(false);
}

void ContextPannel::onColorFillClicked() { m_active_is_fill = true; updateColorButtonsUI(); emit colorTargetActivated(true); }
void ContextPannel::onColorStrokeClicked() { m_active_is_fill = false; updateColorButtonsUI(); emit colorTargetActivated(false); }
void ContextPannel::onMoveUpClicked() { emit moveObjectLayerRequested(1); }
void ContextPannel::onMoveDownClicked() { emit moveObjectLayerRequested(-1); }
void ContextPannel::onAnyUIChanged() {
    emit rasterSettingsChanged();
    if (m_current_target || m_current_text_target || m_current_image_target) emit propertyChanged();
    else {
        m_default_state.type = static_cast<FigureType>(m_type_box->currentData().toInt());
        m_default_state.thickness = m_thick_box->value();
    }
}