#ifndef OBJECT_H
#define OBJECT_H

#include <QGraphicsObject>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QImage>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <qmath.h>

struct TransformState {
    QPointF pos;
    qreal rot = 0.0;
    QRectF rect;

    bool operator==(const TransformState& o) const {
        return pos == o.pos && qAbs(rot - o.rot) < 0.1 && rect == o.rect;
    }
    bool operator!=(const TransformState& o) const { return !(*this == o); }
};

enum class FigureType { Ellipse, Rectangle };

struct FigureState : public TransformState {
    FigureType type = FigureType::Ellipse;
    QColor fill = Qt::cyan;
    QColor stroke = Qt::black;
    float thickness = 2.0f;

    bool operator==(const FigureState& o) const {
        return TransformState::operator==(o) && type == o.type && fill == o.fill && stroke == o.stroke && thickness == o.thickness;
    }
    bool operator!=(const FigureState& o) const { return !(*this == o); }
};

struct TextState : public TransformState {
    QString text;
    QFont font;
    QColor color;

    bool operator==(const TextState& o) const {
        return TransformState::operator==(o) && text == o.text && font == o.font && color == o.color;
    }
    bool operator!=(const TextState& o) const { return !(*this == o); }
};

struct ImageState : public TransformState {
    QImage image;

    bool operator==(const ImageState& o) const {
        return TransformState::operator==(o) && image == o.image;
    }
    bool operator!=(const ImageState& o) const { return !(*this == o); }
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

class Figure : public Object {
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

class TextObject : public Object {
    Q_OBJECT
public:
    explicit TextObject(const QRectF& rect, QGraphicsItem* parent = nullptr);
    explicit TextObject(QGraphicsItem* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void setLocalRect(const QRectF& rect) override;
    QRectF getLocalRect() const override;

    TextState getState() const;
    void setState(const TextState& state);
private:
    TextState m_state;
};

class ImageObject : public Object {
    Q_OBJECT
public:
    explicit ImageObject(const QRectF& rect, const QImage& img, QGraphicsItem* parent = nullptr);
    explicit ImageObject(QGraphicsItem* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void setLocalRect(const QRectF& rect) override;
    QRectF getLocalRect() const override;

    ImageState getState() const;
    void setState(const ImageState& state);
private:
    ImageState m_state;
    QPixmap m_cached_pixmap;
    QRectF m_cached_rect;
};

#endif // OBJECT_H