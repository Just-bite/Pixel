#ifndef PALETTEPANEL_H
#define PALETTEPANEL_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>

class ColorPickerArea : public QWidget {
    Q_OBJECT
public:
    explicit ColorPickerArea(QWidget* parent = nullptr);
    void setHue(int hue);
    QColor currentColor() const;

signals:
    void previewColor(const QColor& color);
    void commitColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateImage();
    void pick(const QPoint& pos, bool commit);

    int m_hue = 0;
    QPoint m_pos;
    QImage m_image;
};

class PalettePannel : public QWidget {
    Q_OBJECT
public:
    explicit PalettePannel(QWidget* parent = nullptr);

signals:
    void colorPreviewed(const QColor& color);
    void colorCommitted(const QColor& color);

private slots:
    void onAreaPreview(const QColor& c);
    void onAreaCommit(const QColor& c);
    void onSlidersChanged();
    void onSliderReleased();
    void onHexChanged();

private:
    void updateUIFromColor(const QColor& c);
    QColor parseHexColor(const QString& hexStr);

    ColorPickerArea* m_area;
    QSlider* m_hue_slider;
    QSlider* m_alpha_slider;
    QLineEdit* m_hex_edit;

    QColor m_current_color;
    bool m_syncing = false;
};

#endif // PALETTEPANEL_H