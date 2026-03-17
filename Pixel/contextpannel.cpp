#include "contextpannel.h"

ContextPannel::ContextPannel(QWidget* parent) : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(5, 2, 5, 2);
    m_layout->setSpacing(10);

    m_type_box = new QComboBox(this);
    m_type_box->addItem("Ellipse", static_cast<int>(FigureType::Ellipse));
    m_type_box->addItem("Rectangle", static_cast<int>(FigureType::Rectangle));

    m_x_box = createSpinBox("X: ", -10000, 10000);
    m_y_box = createSpinBox("Y: ", -10000, 10000);
    m_w_box = createSpinBox("W: ", 1, 10000);
    m_h_box = createSpinBox("H: ", 1, 10000);
    m_rot_box = createSpinBox("R: ", -360, 360);
    m_thick_box = createSpinBox("Thick: ", 0, 100);

    m_btn_fill = new QPushButton("Fill", this);
    m_btn_stroke = new QPushButton("Stroke", this);

    m_layout->addWidget(new QLabel("Type:", this));
    m_layout->addWidget(m_type_box);
    m_layout->addWidget(m_x_box);
    m_layout->addWidget(m_y_box);
    m_layout->addWidget(m_w_box);
    m_layout->addWidget(m_h_box);
    m_layout->addWidget(m_rot_box);
    m_layout->addWidget(m_thick_box);
    m_layout->addWidget(m_btn_fill);
    m_layout->addWidget(m_btn_stroke);
    m_layout->addStretch();

    connect(m_btn_fill, &QPushButton::clicked, this, &ContextPannel::onColorFillClicked);
    connect(m_btn_stroke, &QPushButton::clicked, this, &ContextPannel::onColorStrokeClicked);

    // Связываем изменения значений с основным слотом
    // ИСПРАВЛЕНИЕ: Используем static_cast вместо QOverload для C++11 совместимости
    auto emitChange = [this]() { if (m_current_target) emit propertyChanged(); };
    connect(m_type_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, emitChange);

    connect(m_x_box, &QDoubleSpinBox::editingFinished, this, emitChange);
    connect(m_y_box, &QDoubleSpinBox::editingFinished, this, emitChange);
    connect(m_w_box, &QDoubleSpinBox::editingFinished, this, emitChange);
    connect(m_h_box, &QDoubleSpinBox::editingFinished, this, emitChange);
    connect(m_rot_box, &QDoubleSpinBox::editingFinished, this, emitChange);
    connect(m_thick_box, &QDoubleSpinBox::editingFinished, this, emitChange);

    setEnabled(false); // Выключено по умолчанию, пока нет выделения
}

QDoubleSpinBox* ContextPannel::createSpinBox(const QString& prefix, double min, double max)
{
    QDoubleSpinBox* sb = new QDoubleSpinBox(this);
    sb->setPrefix(prefix);
    sb->setRange(min, max);
    sb->setDecimals(1);
    sb->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sb->setFixedWidth(80);
    return sb;
}

void ContextPannel::setTarget(Figure* figure)
{
    m_current_target = figure;
    setEnabled(figure != nullptr);
    if (!figure) return;

    blockSignals(true);
    FigureState s = figure->getState();
    m_type_box->setCurrentIndex(m_type_box->findData(static_cast<int>(s.type)));
    m_x_box->setValue(s.pos.x());
    m_y_box->setValue(s.pos.y());
    m_w_box->setValue(s.rect.width());
    m_h_box->setValue(s.rect.height());
    m_rot_box->setValue(s.rot);
    m_thick_box->setValue(s.thickness);

    m_btn_fill->setStyleSheet(QString("background-color: %1; color: %2")
                                  .arg(s.fill.name()).arg(s.fill.lightness() > 128 ? "black" : "white"));
    m_btn_stroke->setStyleSheet(QString("background-color: %1; color: %2")
                                    .arg(s.stroke.name()).arg(s.stroke.lightness() > 128 ? "black" : "white"));
    blockSignals(false);
}

FigureState ContextPannel::getUIState(const FigureState& baseState) const
{
    FigureState s = baseState;
    s.type = static_cast<FigureType>(m_type_box->currentData().toInt());
    s.pos = QPointF(m_x_box->value(), m_y_box->value());
    s.rot = m_rot_box->value();
    s.rect = QRectF(-m_w_box->value()/2.0, -m_h_box->value()/2.0, m_w_box->value(), m_h_box->value());
    s.thickness = m_thick_box->value();
    return s;
}

void ContextPannel::onColorFillClicked() { emit colorTargetActivated(true); }
void ContextPannel::onColorStrokeClicked() { emit colorTargetActivated(false); }