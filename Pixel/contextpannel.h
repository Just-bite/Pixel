#ifndef CONTEXTPANNEL_H
#define CONTEXTPANNEL_H

#include <QWidget>
#include <QHBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include "object.h"

class ContextPannel : public QWidget
{
    Q_OBJECT
public:
    explicit ContextPannel(QWidget* parent = nullptr);

    // Загрузить данные фигуры в UI
    void setTarget(Figure* figure);

    // Получить стейт на основе текущих значений UI
    FigureState getUIState(const FigureState& baseState) const;

signals:
    void propertyChanged();
    void colorTargetActivated(bool isFill);

private slots:
    void onColorFillClicked();
    void onColorStrokeClicked();

private:
    QHBoxLayout* m_layout;

    QComboBox* m_type_box;
    QDoubleSpinBox* m_x_box;
    QDoubleSpinBox* m_y_box;
    QDoubleSpinBox* m_w_box;
    QDoubleSpinBox* m_h_box;
    QDoubleSpinBox* m_rot_box;
    QDoubleSpinBox* m_thick_box;

    QPushButton* m_btn_fill;
    QPushButton* m_btn_stroke;

    Figure* m_current_target = nullptr;

    QDoubleSpinBox* createSpinBox(const QString& prefix, double min, double max);
};

#endif // CONTEXTPANNEL_H