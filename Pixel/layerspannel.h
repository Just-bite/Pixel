#ifndef LAYERSPANNEL_H
#define LAYERSPANNEL_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QStyleOption>
#include <QPainter>

#include <vector>

#include "canvas.h"

class LayerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LayerWidget(QWidget* parent = nullptr);

    void setName(const QString& name) { m_layer_name->setText(name); }

protected:
    void paintEvent(QPaintEvent *event) override {
        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }

private:
    static constexpr int BTN_SIZE = 20;
    QHBoxLayout* m_layout;
    QPushButton* m_lock_btn;
    QPushButton* m_eye_btn;
    QPushButton* m_up_btn;
    QPushButton* m_down_btn;
    QPushButton* m_delete_btn;
    QLabel* m_layer_name;
};


class LayersPannel : public QWidget
{
Q_OBJECT
public:
    explicit LayersPannel(QWidget* parent = nullptr, Canvas* canvas = nullptr);

private:
    void updateLayers();

    static constexpr int BTN_SIZE = 20;
    QPushButton* m_new_layer_btn;
    Canvas* m_canvas_ptr;
    QVBoxLayout* m_main_layout;
    std::vector<LayerWidget*> m_layers;
};

#endif // LAYERSPANNEL_H
