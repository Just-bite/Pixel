#ifndef PALETTEPANEL_H
#define PALETTEPANEL_H

#include <QVBoxLayout>
#include <QSlider>
#include <QWidget>
#include <QPainter>

class PalettePannel : public QWidget
{
    Q_OBJECT
public:
    explicit PalettePannel(QWidget* parent = nullptr);
    const QVBoxLayout* getLayout() const { return layout; } // возвращаем указатель

public slots:
    void setHue(int hue);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    int current_hue = 0;
    QImage color_palette;
    QSlider* slider;
    QVBoxLayout* layout;  // переименовано для ясности

    void updateColorImage();

};
#endif // PALETTEPANEL_H
