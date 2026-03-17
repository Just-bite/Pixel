#ifndef INFOPANNEL_H
#define INFOPANNEL_H

#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

class InfoPannel : public QWidget
{
    Q_OBJECT

public:
    explicit InfoPannel(std::pair<int, int> canvas_size, float scale = 0,  QWidget* parent = nullptr);
    void setScale(float scale);
    void setCanvasSize(std::pair<int,int> canvas_size);
    void updateCanvasSizeDisplay(int width, int height);
    void updateScaleDisplay(float scale);
    const QHBoxLayout& getLayout() const;

signals:
    // Сигналы для связи с MainWindow
    void zoomInRequested();
    void zoomOutRequested();
    void fitRequested();
    void scaleChanged(float newScale);

private:
    std::pair<int, int> m_canvas_size;
    QPushButton* m_btn_decrease;
    QPushButton* m_btn_increase;
    QPushButton* m_btn_fit; // Новая кнопка
    QLineEdit* m_scale_edit;
    QLabel* m_scale_label;
    QLabel* m_canvas_size_label;
    QHBoxLayout* m_info_pannel_layout;
    float m_scale;

public slots:
    void updateDisplay();
    void onScaleEditFinished(); // Слот для ручного ввода
};

#endif // INFOPANNEL_H