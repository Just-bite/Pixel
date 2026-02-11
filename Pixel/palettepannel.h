#ifndef PALETTEPANEL_H
#define PALETTEPANEL_H

#include <QWidget>
#include <QPainter>

class PalettePannel : public QWidget
{
    Q_OBJECT
public:
    explicit PalettePannel(QWidget* parent = nullptr);

public slots:
    void setHue(int hue);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:  
    void updateColorImage();
    int m_current_hue = 0;
    QImage m_color_palette;

};
#endif // PALETTEPANEL_H
