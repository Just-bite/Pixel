#include "layerspannel.h"

#include <QDebug>

LayerWidget::LayerWidget(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(3);

    m_lock_btn = new QPushButton("L");
    m_eye_btn = new QPushButton("V");
    m_up_btn = new QPushButton("u");
    m_down_btn = new QPushButton("d");
    m_delete_btn = new QPushButton("-");

    for (QPushButton* b : {m_lock_btn, m_eye_btn, m_down_btn, m_up_btn, m_delete_btn})
    {
        b->setFixedSize(BTN_SIZE, BTN_SIZE);
    }

    m_layer_name = new QLabel("lol");

    m_layout->addWidget(m_lock_btn);
    m_layout->addWidget(m_eye_btn);
    m_layout->addWidget(m_layer_name);
    m_layout->addStretch();
    m_layout->addWidget(m_down_btn);
    m_layout->addWidget(m_up_btn);
    m_layout->addWidget(m_delete_btn);
}



LayersPannel::LayersPannel(QWidget *parent, Canvas* canvas)
    : QWidget(parent)
    , m_canvas_ptr(canvas)
{
    m_main_layout = new QVBoxLayout(this);
    m_new_layer_btn = new QPushButton("+");
    m_new_layer_btn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_main_layout->addWidget(m_new_layer_btn);

    updateLayers();
}

void LayersPannel::updateLayers()
{
    if (!m_canvas_ptr) {
        qDebug() << "err: no canvas given to layer pannel";
        return;
    }

    std::vector<LayerInfo> info = m_canvas_ptr->getLayersInfo();

    for (auto i = info.cend(); i != info.cbegin(); )
    {
        --i;

        LayerWidget* lw = new LayerWidget(this);
        lw->setName(i->name);

        m_layers.push_back(lw);
        m_main_layout->addWidget(lw);
    }

}

