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
#include <QFontComboBox>

#include "object.h"
#include "filterlayer.h"

class ContextPannel : public QWidget {
    Q_OBJECT
public:
    explicit ContextPannel(QWidget* parent = nullptr);

    void setMode(bool isFigSel, bool isTextSel, bool isFigTool, bool isTextTool, const QString& toolName = "");

    void setTarget(Figure* figure);
    void setTarget(TextObject* textObj);
    void setTarget(ImageObject* imageObj);
    void setTarget(FilterLayer* filterObj);

    FigureState getUIState(const FigureState& baseState) const;
    TextState getUITextState(const TextState& baseState) const;
    ImageState getUIImageState(const ImageState& baseState) const;
    FilterState getUIFilterState() const;

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
    void onFilterUIChanged();
    void onMoveUpClicked();
    void onMoveDownClicked();

private:
    void updateColorButtonsUI();
    QDoubleSpinBox* createSpinBox(double min, double max);
    void addLabeledWidget(QHBoxLayout* layout, const QString& text, QWidget* widget);

    QLabel* m_lbl_placeholder;
    QGroupBox *m_geometry_group, *m_style_group;
    QWidget *m_layer_group;

    QComboBox* m_type_box;
    QDoubleSpinBox *m_x_box, *m_y_box, *m_w_box, *m_h_box, *m_rot_box, *m_thick_box;
    QPushButton *m_btn_fill_color, *m_btn_stroke_color;
    QPushButton *m_btn_layer_up, *m_btn_layer_down;

    Figure* m_current_target = nullptr;
    FigureState m_default_state;
    bool m_active_is_fill = true;

    QGroupBox *m_text_group;
    QFontComboBox* m_font_box;
    QDoubleSpinBox* m_font_size_box;

    TextObject* m_current_text_target = nullptr;
    TextState m_default_text_state;

    ImageObject* m_current_image_target = nullptr;

    QGroupBox *m_filter_group;
    QComboBox *m_filter_type_box;
    QDoubleSpinBox *m_filter_param1, *m_filter_param2;
    FilterLayer* m_current_filter_target = nullptr;
    QLabel *m_lbl_filter_p1, *m_lbl_filter_p2;
    void updateFilterUIRanges();
};

#endif // CONTEXTPANNEL_H
