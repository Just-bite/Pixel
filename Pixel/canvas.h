#ifndef CANVAS_H
#define CANVAS_H

#include <QObject>
#include <QPainter>
#include <QWidget>
#include <QPaintEvent>

class Canvas : public QWidget
{
    Q_OBJECT

public:
    explicit Canvas(QObject* parent = nullptr);
    void draw(QPainter* painter) const;

private:

public slots:
    void paintEvent(QPaintEvent* event) const {};
};


#endif // CANVAS_H
