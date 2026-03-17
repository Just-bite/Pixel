#include "layerspannel.h"
#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QScrollBar>

LayerWidget::LayerWidget(QWidget* parent) : QWidget(parent) {
    setFixedHeight(32);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(3);
    m_layout->setContentsMargins(2, 2, 2, 2);

    m_lock_btn = new QPushButton("L"); m_lock_btn->setCheckable(true);
    m_eye_btn = new QPushButton("V");  m_eye_btn->setCheckable(true);
    m_up_btn = new QPushButton("u");
    m_down_btn = new QPushButton("d");
    m_delete_btn = new QPushButton("-");

    for (QPushButton* b : {m_lock_btn, m_eye_btn, m_down_btn, m_up_btn, m_delete_btn}) {
        b->setFixedSize(BTN_SIZE, BTN_SIZE);
    }

    m_layer_name = new QLineEdit(this);
    m_layer_name->setStyleSheet("background: transparent; border: none; color: #e0e0e0;");
    m_layer_name->installEventFilter(this); // ПЕРЕХВАТЫВАЕМ КЛИК!

    connect(m_layer_name, &QLineEdit::editingFinished, this, [this](){
        emit nameChanged(m_layer_name->text());
        m_layer_name->clearFocus();
    });


    m_layout->addWidget(m_lock_btn);
    m_layout->addWidget(m_eye_btn);
    m_layout->addWidget(m_layer_name);
    m_layout->addStretch();
    m_layout->addWidget(m_down_btn);
    m_layout->addWidget(m_up_btn);
    m_layout->addWidget(m_delete_btn);

    connect(m_delete_btn, &QPushButton::clicked, this, &LayerWidget::deleteClicked);
    connect(m_up_btn, &QPushButton::clicked, this, &LayerWidget::upClicked);
    connect(m_down_btn, &QPushButton::clicked, this, &LayerWidget::downClicked);
    connect(m_eye_btn, &QPushButton::toggled, this, &LayerWidget::onVisibleToggled);
    connect(m_lock_btn, &QPushButton::toggled, this, &LayerWidget::onLockedToggled);
}

void LayerWidget::setName(const QString& name) {
    m_layer_name->setText(name);
    m_layer_name->setCursorPosition(0);
}

void LayerWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt; opt.initFrom(this);
    QPainter p(this);
    if (m_is_selected) p.fillRect(rect(), QColor(60, 140, 220, 100));
    else style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LayerWidget::setSelected(bool selected) {
    if (m_is_selected != selected) { m_is_selected = selected; update(); }
}
void LayerWidget::setVisibleState(bool visible) { m_eye_btn->blockSignals(true); m_eye_btn->setChecked(!visible); m_eye_btn->blockSignals(false); }
void LayerWidget::setLockedState(bool locked) { m_lock_btn->blockSignals(true); m_lock_btn->setChecked(locked); m_lock_btn->blockSignals(false); }
void LayerWidget::onVisibleToggled(bool checked) { emit visibleToggled(!checked); } // Checked = Hidden
void LayerWidget::onLockedToggled(bool checked) { emit lockedToggled(checked); }
void LayerWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) emit layerClicked();
    QWidget::mousePressEvent(event);
}

bool LayerWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_layer_name && event->type() == QEvent::MouseButtonPress) {
        emit layerClicked(); // Эмулируем клик по слою
        // Возвращаем false, чтобы QLineEdit всё равно получил фокус для ввода текста
    }
    return QWidget::eventFilter(obj, event);
}

LayersPannel::LayersPannel(QWidget *parent, Canvas* canvas) : QWidget(parent), m_canvas_ptr(canvas) {
    m_main_layout = new QVBoxLayout(this);
    m_main_layout->setContentsMargins(0, 0, 0, 0);

    m_new_layer_btn = new QPushButton("+");
    m_new_layer_btn->setFixedSize(BTN_SIZE + 10, BTN_SIZE + 5);
    m_main_layout->addWidget(m_new_layer_btn);

    m_scroll_area = new QScrollArea(this);
    m_scroll_area->setWidgetResizable(true);
    m_scroll_area->setFrameShape(QFrame::NoFrame);

    QWidget* scrollContent = new QWidget();
    m_layers_layout = new QVBoxLayout(scrollContent);
    m_layers_layout->setAlignment(Qt::AlignTop);
    m_layers_layout->setContentsMargins(2, 2, 2, 2);
    m_layers_layout->setSpacing(2);

    m_scroll_area->setWidget(scrollContent);
    m_main_layout->addWidget(m_scroll_area);

    connect(m_new_layer_btn, &QPushButton::clicked, this, &LayersPannel::onNewLayerClicked);
    updateLayers();
}

void LayersPannel::updateLayers() {
    if (!m_canvas_ptr) return;
    for (LayerWidget* lw : m_layers) { m_layers_layout->removeWidget(lw); delete lw; }
    m_layers.clear();

    std::vector<LayerInfo> info = m_canvas_ptr->getLayersInfo();
    int selectedIdx = m_canvas_ptr->getSelectedLayerid();

    for (auto i = info.crbegin(); i != info.crend(); ++i) {
        LayerWidget* lw = new LayerWidget(this);
        lw->setName(i->name);
        int index = info.size() - int(i - info.crbegin() + 1);
        lw->setIndex(index);
        lw->setVisibleState(i->visible);
        lw->setLockedState(i->locked);

        if (index == selectedIdx) lw->setSelected(true);

        connect(lw, &LayerWidget::deleteClicked, this, &LayersPannel::onLayerDeleteClicked);
        connect(lw, &LayerWidget::upClicked, this, &LayersPannel::onLayerUpClicked);
        connect(lw, &LayerWidget::downClicked, this, &LayersPannel::onLayerDownClicked);
        connect(lw, &LayerWidget::layerClicked, this, &LayersPannel::onLayerClicked);
        connect(lw, &LayerWidget::visibleToggled, this, &LayersPannel::onLayerVisibleToggled);
        connect(lw, &LayerWidget::lockedToggled, this, &LayersPannel::onLayerLockedToggled);
        connect(lw, &LayerWidget::nameChanged, this, &LayersPannel::onLayerNameChanged);

        m_layers.push_back(lw);
        m_layers_layout->addWidget(lw);
    }
}

void LayersPannel::selectLayerFromOutside(int id) {
    if (!m_canvas_ptr || id < 0) return;
    m_canvas_ptr->selectLayer(id);
    refreshSelectionVisuals(id);
}

void LayersPannel::refreshSelectionVisuals(int selectedIndex) {
    for (LayerWidget* lw : m_layers) {
        bool isSel = (lw->getIndex() == selectedIndex);
        lw->setSelected(isSel);
        if (isSel) m_scroll_area->ensureWidgetVisible(lw, 0, 50);
    }
}

void LayersPannel::onLayerDeleteClicked() {
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    if (sender_layer && m_canvas_ptr) {
        m_canvas_ptr->deleteLayer(sender_layer->getIndex());
        updateLayers();
    }
}

void LayersPannel::onNewLayerClicked() { m_canvas_ptr->newLayer(); updateLayers(); }

void LayersPannel::onLayerUpClicked() {
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    if (sender_layer) { m_canvas_ptr->moveLayer(sender_layer->getIndex(), 1); updateLayers(); }
}

void LayersPannel::onLayerDownClicked() {
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    if (sender_layer) { m_canvas_ptr->moveLayer(sender_layer->getIndex(), -1); updateLayers(); }
}

void LayersPannel::onLayerClicked() {
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    if (sender_layer) selectLayerFromOutside(sender_layer->getIndex());
}

void LayersPannel::onLayerVisibleToggled(bool visible) {
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    if (sender_layer) m_canvas_ptr->setLayerVisible(sender_layer->getIndex(), visible);
}

void LayersPannel::onLayerLockedToggled(bool locked) {
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    if (sender_layer) m_canvas_ptr->setLayerLocked(sender_layer->getIndex(), locked);
}

void LayersPannel::onLayerNameChanged(const QString& newName) {
    LayerWidget* sender_layer = qobject_cast<LayerWidget*>(sender());
    if (sender_layer && m_canvas_ptr) {
        m_canvas_ptr->renameLayer(sender_layer->getIndex(), newName);
    }
}