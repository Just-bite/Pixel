#include "infopannel.h"

InfoPannel::InfoPannel(std::pair<int, int> canvas_size, float scale, QWidget* parent)
    : m_canvas_size(canvas_size)
    , m_scale(scale)
{
    m_info_pannel_layout = new QHBoxLayout(parent);
    m_info_pannel_layout->setSpacing(5);
    m_info_pannel_layout->setContentsMargins(5, 0, 5, 0);

    m_canvas_size_label = new QLabel(QString("Canvas size: %1 x %2").arg(canvas_size.first).arg(canvas_size.second), parent);

    m_scale_label = new QLabel(QString("Scale (%):"), parent);
    m_scale_edit = new QLineEdit(QString("%1").arg(scale * 100), parent);

    m_btn_decrease = new QPushButton("-", parent);
    m_btn_increase = new QPushButton("+", parent);

    m_info_pannel_layout->addWidget(m_canvas_size_label);
    m_info_pannel_layout->addStretch();
    m_info_pannel_layout->addWidget(m_scale_label);
    m_info_pannel_layout->addWidget(m_scale_edit);
    m_info_pannel_layout->addWidget(m_btn_increase);
    m_info_pannel_layout->addWidget(m_btn_decrease);
}

void InfoPannel::setCanvasSize(std::pair<int,int> canvas_size)
{
    this->m_canvas_size = canvas_size;
     updateDisplay();
}

const QHBoxLayout& InfoPannel::getLayout() const
{
    return *m_info_pannel_layout;
}

void InfoPannel::setScale(float scale)
{
    this->m_scale = scale;
    updateDisplay();
}

void InfoPannel::updateDisplay()
{
    if (m_canvas_size_label) {
        m_canvas_size_label->setText(
            QString("Canvas size: %1 x %2").arg(m_canvas_size.first).arg(m_canvas_size.second)
        );
    }

    if (m_scale_edit) {
        m_scale_edit->setText(QString::number(m_scale * 100, 'f', 1));
    }
}


void InfoPannel::updateCanvasSizeDisplay(int width, int height)
{
    if (m_canvas_size_label) {
        m_canvas_size_label->setText(
            QString("Canvas size: %1 x %2").arg(width).arg(height)
        );
    }
}

void InfoPannel::updateScaleDisplay(float scale)
{
    if (m_scale_edit)
        m_scale_edit->setText(QString::number(scale * 100, 'f', 1));
}


