#ifndef LAYERSPANNEL_H
#define LAYERSPANNEL_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollArea>
#include <vector>
#include "canvas.h"

class LayerWidget : public QWidget {
    Q_OBJECT
public:
    explicit LayerWidget(QWidget* parent = nullptr);

    void setName(const QString& name);
    void setIndex(int id) { m_index = id; }
    int getIndex() const { return m_index; }
    void setIsFilter(bool isF);

    void setSelected(bool selected);
    bool isSelected() const { return m_is_selected; }

    void setVisibleState(bool visible);
    void setLockedState(bool locked);

signals:
    void deleteClicked();
    void upClicked();
    void downClicked();
    void layerClicked();
    void visibleToggled(bool visible);
    void lockedToggled(bool locked);
    void nameChanged(const QString& newName);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onVisibleToggled(bool checked);
    void onLockedToggled(bool checked);

private:
    static constexpr int BTN_SIZE = 30;
    QHBoxLayout* m_layout;
    QPushButton *m_lock_btn, *m_eye_btn, *m_up_btn, *m_down_btn, *m_delete_btn;
    QLineEdit* m_layer_name;
    int m_index = 0;
    bool m_is_selected = false;
};

class LayersPannel : public QWidget {
    Q_OBJECT
public:
    explicit LayersPannel(QWidget* parent = nullptr, Canvas* canvas = nullptr);
    void selectLayerFromOutside(int id);

public slots:
    void updateLayers();

private:
    void refreshSelectionVisuals(int selectedIndex);

    static constexpr int BTN_SIZE = 20;
    QPushButton* m_new_layer_btn;
    Canvas* m_canvas_ptr;
    QVBoxLayout *m_main_layout, *m_layers_layout;
    QScrollArea* m_scroll_area;
    std::vector<LayerWidget*> m_layers;
    QPushButton* m_new_filter_btn;

private slots:
    void onLayerDeleteClicked();
    void onNewLayerClicked();
    void onLayerUpClicked();
    void onLayerDownClicked();
    void onLayerClicked();
    void onLayerVisibleToggled(bool visible);
    void onLayerLockedToggled(bool locked);
    void onLayerNameChanged(const QString& newName);
    void onNewFilterClicked();
};

#endif // LAYERSPANNEL_H