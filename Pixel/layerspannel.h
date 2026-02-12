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
    void setIndex(int id) { m_index = id; }
    int getIndex() { return m_index; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    static constexpr int BTN_SIZE = 20;
    QHBoxLayout* m_layout;
    QPushButton* m_lock_btn;
    QPushButton* m_eye_btn;
    QPushButton* m_up_btn;
    QPushButton* m_down_btn;
    QPushButton* m_delete_btn;
    QLabel* m_layer_name;
    int m_index;

signals:
    void deleteClicked();
private slots:
    void onDeleteClicked();
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

    QVBoxLayout* m_main_layout;   // Главный лэйаут всей панели
    QVBoxLayout* m_layers_layout; // Лэйаут внутри скролла (для списка слоев)

    std::vector<LayerWidget*> m_layers;

signals:
    void deleteLayerClicked();

private slots:
    void onLayerDeleteClicked();
    void onNewLayerClicked();
};

#endif // LAYERSPANNEL_H
