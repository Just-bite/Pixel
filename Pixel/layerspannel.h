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

    void setSelected(bool selected);
    bool isSelected() const { return m_is_selected; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

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

    bool m_is_selected = false;
signals:
    void deleteClicked();
    void upClicked();
    void downClicked();
    void layerClicked();

private slots:
    void onDeleteClicked();
    void onUpClicked();
    void onDownClicked();
};


class LayersPannel : public QWidget
{
    Q_OBJECT
public:
    explicit LayersPannel(QWidget* parent = nullptr, Canvas* canvas = nullptr);

private:
    void updateLayers();
    void moveLayer(int id, int shift);
    void refreshSelectionVisuals(int selectedIndex);

    static constexpr int BTN_SIZE = 20;
    QPushButton* m_new_layer_btn;
    Canvas* m_canvas_ptr;

    QVBoxLayout* m_main_layout;
    QVBoxLayout* m_layers_layout;

    std::vector<LayerWidget*> m_layers;

signals:
    void deleteLayerClicked();
    void layerUpClicked();
    void layerDownClicked();
    void layerClicked();

private slots:
    void onLayerDeleteClicked();
    void onNewLayerClicked();
    void onLayerUpClicked();
    void onLayerDownClicked();
    void onLayerClicked();
};

#endif // LAYERSPANNEL_H
