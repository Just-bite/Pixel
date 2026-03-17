#ifndef CONTEXTPANNEL_H
#define CONTEXTPANNEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include "object.h"

class ContextPannel : public QWidget {
    Q_OBJECT
public:
    explicit ContextPannel(QWidget* parent = nullptr);

    void setMode(bool isFigureSelected, bool isFigureTool, const QString& toolName = "");
    void setTarget(Figure* figure);

    FigureState getUIState(const FigureState& baseState) const;
    FigureState getDefaultState() const { return m_default_state; }
    void setDefaultColor(bool isFill, const QColor& color);

    QColor getActiveColor() const;

signals:
    void propertyChanged();
    void colorTargetActivated(bool isFill);
    void moveObjectLayerRequested(int shift);

private slots:
    void onColorFillClicked();
    void onColorStrokeClicked();
    void onAnyUIChanged();
    void onMoveUpClicked();
    void onMoveDownClicked();

private:
    void updateColorButtonsUI();
    QDoubleSpinBox* createSpinBox(double min, double max);
    void addLabeledWidget(QHBoxLayout* layout, const QString& text, QWidget* widget);

    QLabel* m_lbl_placeholder;

    // ИСПРАВЛЕНИЕ: m_layer_group теперь QWidget*, а не QGroupBox*
    QGroupBox *m_geometry_group, *m_style_group;
    QWidget *m_layer_group;

    QComboBox* m_type_box;
    QDoubleSpinBox *m_x_box, *m_y_box, *m_w_box, *m_h_box, *m_rot_box, *m_thick_box;
    QPushButton *m_btn_fill_color, *m_btn_stroke_color;
    QPushButton *m_btn_layer_up, *m_btn_layer_down;

    Figure* m_current_target = nullptr;
    FigureState m_default_state;
    bool m_active_is_fill = true;
};

#endif // CONTEXTPANNEL_H