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

class ContextPannel : public QWidget
{
    Q_OBJECT
public:
    explicit ContextPannel(QWidget* parent = nullptr);

    void setMode(bool isFigureSelected, bool isFigureTool);
    void setTarget(Figure* figure);

    FigureState getUIState(const FigureState& baseState) const;
    FigureState getDefaultState() const { return m_default_state; }
    void setDefaultColor(bool isFill, const QColor& color);

signals:
    void propertyChanged();
    void colorTargetActivated(bool isFill);

private slots:
    void onColorFillClicked();
    void onColorStrokeClicked();
    void onAnyUIChanged();

private:
    void updateColorButtonsUI();
    QDoubleSpinBox* createSpinBox(double min, double max);
    void addLabeledWidget(QHBoxLayout* layout, const QString& text, QWidget* widget);

    QGroupBox* m_geometry_group;
    QGroupBox* m_style_group;

    QComboBox* m_type_box;
    QDoubleSpinBox *m_x_box, *m_y_box, *m_w_box, *m_h_box, *m_rot_box, *m_thick_box;

    // Новые элементы для цветов
    QPushButton *m_btn_fill_color;
    QPushButton *m_btn_stroke_color;

    Figure* m_current_target = nullptr;
    FigureState m_default_state;
    bool m_active_is_fill = true;
};

#endif // CONTEXTPANNEL_H