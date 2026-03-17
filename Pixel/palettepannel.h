#ifndef PALETTEPANEL_H
#define PALETTEPANEL_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class PalettePannel : public QWidget
{
    Q_OBJECT
public:
    explicit PalettePannel(QWidget* parent = nullptr);

public slots:
    void setHue(int hue);

signals:
    void colorSelected(const QColor& color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void updateColorImage();
    void pickColor(const QPoint& pos);

    int m_current_hue = 0;
    QImage m_color_palette;
    QPoint m_selected_pos;
};
#endif // PALETTEPANEL_H