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
    InfoPannel(std::pair<int, int> canvas_size, float scale = 0,  QWidget* parent = nullptr);
    void setScale(float scale);
    void setCanvasSize(std::pair<int,int> canvas_size);
    const QHBoxLayout& getLayout() const;

private:
    std::pair<int, int> canvas_size;
    float scale;
    QPushButton* btn_decrease;
    QPushButton* btn_increase;
    QLineEdit* scale_edit;
    QLabel* scale_label;
    QLabel* canvas_size_label;
    QHBoxLayout* info_pannel_layout;

};

#endif // INFOPANNEL_H
