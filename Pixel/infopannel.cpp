#include "infopannel.h"

InfoPannel::InfoPannel(std::pair<int, int> canvas_size, float scale, QWidget* parent)
    : canvas_size(canvas_size)
    , scale(scale)

{
    info_pannel_layout = new QHBoxLayout(parent);
    info_pannel_layout->setSpacing(5);
    info_pannel_layout->setContentsMargins(5,0,5,0);

    canvas_size_label = new QLabel(QString("Canvas size: %1 x %2").arg(canvas_size.first).arg(canvas_size.second), parent);

    scale_label = new QLabel(QString("Scale: 1 :"), parent);
    scale_edit = new QLineEdit(QString("%1").arg(scale), parent);

    btn_decrease = new QPushButton("-", parent);
    btn_increase = new QPushButton("+", parent);

    info_pannel_layout->addWidget(canvas_size_label);
    info_pannel_layout->addStretch();
    info_pannel_layout->addWidget(scale_label);
    info_pannel_layout->addWidget(scale_edit);
    info_pannel_layout->addWidget(btn_increase);
    info_pannel_layout->addWidget(btn_decrease);

}

void InfoPannel::setCanvasSize(std::pair<int,int> canvas_size)
{
    this->canvas_size = canvas_size;
}


const QHBoxLayout& InfoPannel::getLayout() const
{
    return *info_pannel_layout;
}

void InfoPannel::setScale(float scale)
{
    this->scale = scale;
}
