#include "layerspannel.h"

#include <QDebug>
#include <QScrollArea>
#include <QMouseEvent>

// --- LayerWidget ---

LayerWidget::LayerWidget(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(32);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(3);
    m_layout->setContentsMargins(2, 2, 2, 2);

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

    connect(m_delete_btn, &QPushButton::clicked, this, &LayerWidget::onDeleteClicked);
    connect(m_up_btn, &QPushButton::clicked, this, &LayerWidget::onUpClicked);
    connect(m_down_btn, &QPushButton::clicked, this, &LayerWidget::onDownClicked);
}

void LayerWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);

    if (m_is_selected)
        p.fillRect(rect(), QColor(60, 140, 220, 100));
    else {
        QStyleOption opt;
        opt.init(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }

}

void LayerWidget::setSelected(bool selected)
{
    if (m_is_selected != selected) {
        m_is_selected = selected;
        update();
    }
}

void LayerWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit layerClicked();
    }
    QWidget::mousePressEvent(event);
}

void LayerWidget::onDeleteClicked()
{
    emit deleteClicked();
}

void LayerWidget::onUpClicked()
{
    emit upClicked();
}

void LayerWidget::onDownClicked()
{
    emit downClicked();
}


// --- LayersPannel ---

LayersPannel::LayersPannel(QWidget *parent, Canvas* canvas)
    : QWidget(parent)
    , m_canvas_ptr(canvas)
{
    m_main_layout = new QVBoxLayout(this);
    m_main_layout->setContentsMargins(0, 0, 0, 0);

    m_new_layer_btn = new QPushButton("+");
    m_new_layer_btn->setFixedSize(BTN_SIZE + 10, BTN_SIZE + 5);
    m_main_layout->addWidget(m_new_layer_btn);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* scrollContent = new QWidget();
    m_layers_layout = new QVBoxLayout(scrollContent);

    m_layers_layout->setAlignment(Qt::AlignTop);
    m_layers_layout->setContentsMargins(2, 2, 2, 2);
    m_layers_layout->setSpacing(2);

    scrollArea->setWidget(scrollContent);
    m_main_layout->addWidget(scrollArea);

    updateLayers();

    connect(m_new_layer_btn, &QPushButton::clicked, this, &LayersPannel::onNewLayerClicked);
}

void LayersPannel::updateLayers()
{
    if (!m_canvas_ptr) {
        qDebug() << "err: no canvas given to layer pannel";
        return;
    }

    for (LayerWidget* lw : m_layers)
    {
        m_layers_layout->removeWidget(lw);
        delete lw;
    }
    m_layers.clear();

    std::vector<LayerInfo> info = m_canvas_ptr->getLayersInfo();

    int selectedIdx = m_canvas_ptr->getSelectedLayerid();

    for (auto i = info.crbegin(); i != info.crend(); ++i)
    {
        LayerWidget* lw = new LayerWidget(this);
        lw->setName(i->name);
        int index = info.size() - int(i - info.crbegin() + 1);
        lw->setIndex(index);

        if (index == selectedIdx)
            lw->setSelected(true);

        connect(lw, &LayerWidget::deleteClicked, this, &LayersPannel::onLayerDeleteClicked);
        connect(lw, &LayerWidget::upClicked, this, &LayersPannel::onLayerUpClicked);
        connect(lw, &LayerWidget::downClicked, this, &LayersPannel::onLayerDownClicked);

        connect(lw, &LayerWidget::layerClicked, this, &LayersPannel::onLayerClicked);

        m_layers.push_back(lw);
        m_layers_layout->addWidget(lw);
    }
}

void LayersPannel::onLayerDeleteClicked()
{
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    if (sender_layer && m_canvas_ptr) {
        m_canvas_ptr->deleteLayer(sender_layer->getIndex());
        updateLayers();
        m_canvas_ptr->renderCanvas();
    }
}

void LayersPannel::onNewLayerClicked()
{
    m_canvas_ptr->newLayer();
    updateLayers();
}

void LayersPannel::onLayerUpClicked()
{
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    moveLayer(sender_layer->getIndex(), 1);
}

void LayersPannel::onLayerDownClicked()
{
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    moveLayer(sender_layer->getIndex(), -1);
}

void LayersPannel::onLayerClicked()
{
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    if (!sender_layer || !m_canvas_ptr) return;

    int index = sender_layer->getIndex();
    m_canvas_ptr->selectLayer(index);
    refreshSelectionVisuals(index);
}

void LayersPannel::moveLayer(int id, int shift)
{
    m_canvas_ptr->moveLayer(id, shift);
    updateLayers();
    m_canvas_ptr->renderCanvas();
}

void LayersPannel::refreshSelectionVisuals(int selectedIndex)
{
    for (LayerWidget* lw : m_layers)
    {
        lw->setSelected(lw->getIndex() == selectedIndex);
    }
}
