#ifndef OBJECT_H
#define OBJECT_H

#include <QGraphicsObject>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QStyleOptionGraphicsItem>

enum class FigureType {
    Ellipse,
    Rectangle
};

struct FigureState {
    QPointF pos;
    qreal rot;
    QRectF rect;
    FigureType type;
    QColor fill;
    QColor stroke;
    float thickness;

    bool operator==(const FigureState& o) const {
        return pos == o.pos && rot == o.rot && rect == o.rect &&
               type == o.type && fill == o.fill && stroke == o.stroke && thickness == o.thickness;
    }
    bool operator!=(const FigureState& o) const { return !(*this == o); }
};

class Object : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit Object(QGraphicsItem* parent = nullptr) : QGraphicsObject(parent) {
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }
    virtual ~Object() = default;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override = 0;
    virtual QRectF boundingRect() const override = 0;
    virtual QPainterPath shape() const override = 0;

    virtual void setLocalRect(const QRectF& rect) = 0;
    virtual QRectF getLocalRect() const = 0;
};

class Figure : public Object
{
    Q_OBJECT
public:
    explicit Figure(const QRectF& rect, FigureType type = FigureType::Ellipse, QGraphicsItem* parent = nullptr);
    explicit Figure(QGraphicsItem* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void setLocalRect(const QRectF& rect) override;
    QRectF getLocalRect() const override;

    FigureState getState() const;
    void setState(const FigureState& state);

private:
    FigureState m_state;
};

#endif // OBJECT_H