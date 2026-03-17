#ifndef OBJECT_H
#define OBJECT_H

#include <QGraphicsObject>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QStyleOptionGraphicsItem>

class Object : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit Object(QGraphicsItem* parent = nullptr) : QGraphicsObject(parent) {
        // Включаем встроенную магию Qt: объект можно выделять и таскать мышкой!
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }
    virtual ~Object() = default;

    // Эти методы требует QGraphicsItem
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override = 0;
    virtual QRectF boundingRect() const override = 0;
    virtual QPainterPath shape() const override = 0;
};

class StyleObject : public Object
{
    Q_OBJECT
public:
    explicit StyleObject(QGraphicsItem* parent = nullptr) : Object(parent), m_opacity(1.0) {}

    void setFillColor(const QColor& color) { m_fillColor = color; update(); }
    QColor fillColor() const { return m_fillColor; }

    void setStrokeColor(const QColor& color) { m_strokeColor = color; update(); }
    QColor getStrokeColor() const { return m_strokeColor; }

    void setStrokeWidth(float width) { m_strokeWidth = width; update(); }
    float getStrokeWidth() const { return m_strokeWidth; }

protected:
    QColor m_fillColor = Qt::transparent;
    QColor m_strokeColor = Qt::black;
    float m_strokeWidth = 2.0f;
    float m_opacity;
};

class Shape : public StyleObject
{
    Q_OBJECT
public:
    explicit Shape(QGraphicsItem* parent = nullptr) : StyleObject(parent), m_filled(true) {}
protected:
    bool m_filled;
};

class Ellipse : public Shape
{
    Q_OBJECT
public:
    Ellipse(const QPointF& center, qreal radius, QGraphicsItem* parent = nullptr);
    explicit Ellipse(QGraphicsItem* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void setCenter(const QPointF& center);
    QPointF getCenter() const;

    void setRadius(qreal radius);
    qreal getRadius() const;

private:
    QPointF m_center;
    qreal m_radius;
};

#endif // OBJECT_H