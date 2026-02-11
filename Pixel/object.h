#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QPainter>

class Object;

class Object : public QObject
{
    Q_OBJECT
public:
    explicit Object(QObject* parent = nullptr) {};
    virtual ~Object() = default;

    virtual void draw(QPainter* painter) const = 0;
    virtual bool contains(const QPointF& point) const = 0;
    virtual QRectF boundingRect() const = 0;

    void setPosition(const QPointF& pos);
    QPointF position() const;

    void setRotation(float angle);
    float getRotation() const;

    void setScale(float scale);
    float getScale() const;

    void setVisible(bool visible);
    bool isVisible() const;

    void move(const QPointF& delta);
    void rotate(float angle);
    void scaleBy(float factor);

protected:
    QPointF m_position;
    float m_rotation;
    float m_scale;
    bool m_visible;
};


class StyleObject : public Object
{
    Q_OBJECT
public:
    explicit StyleObject(QObject* parent = nullptr) {};

    void setFillColor(const QColor& color);
    QColor fillColor() const;

    void setStrokeColor(const QColor& color);
    QColor strokeColor() const;

    void setStrokeWidth(float width);
    float strokeWidth() const;

    void setOpacity(float opacity);
    float opacity() const;

protected:
    QColor m_fillColor;
    QColor m_strokeColor;
    float m_strokeWidth;
    float m_opacity;
};

class Shape : public StyleObject
{
    Q_OBJECT
public:
    explicit Shape(QObject* parent = nullptr) {};

    void setFilled(bool filled);
    bool isFilled() const;

protected:
    bool m_filled;
};

class Ellipse : public Shape
{
    Q_OBJECT
public:
    Ellipse(const QPointF& center, qreal radius, QObject* parent = nullptr);
    Ellipse(qreal x, qreal y, qreal radius, QObject* parent = nullptr);
    explicit Ellipse(QObject* parent = nullptr);

    void draw(QPainter* painter) const override;
    bool contains(const QPointF& point) const override;
    QRectF boundingRect() const override;

    void setCenter(const QPointF& center);
    QPointF center() const;

    void setRadius(qreal radius);
    qreal radius() const;

private:
    QPointF m_center;
    qreal m_radius;
};

class Rectangle : public Shape
{
    Q_OBJECT
public:
    Rectangle(const QRectF& rect, QObject* parent = nullptr);
    Rectangle(qreal x, qreal y, qreal width, qreal height, QObject* parent = nullptr);

    void draw(QPainter* painter) const override {};
    bool contains(const QPointF& point) const override {};
    QRectF boundingRect() const override {};

    void setRect(const QRectF& rect);
    QRectF rect() const;

    void setSize(const QSizeF& size);
    QSizeF size() const;

    void setCornerRadius(qreal radius);
    qreal cornerRadius() const;

private:
    QRectF m_rect;
    qreal m_cornerRadius;
};


class Text : public StyleObject
{
    Q_OBJECT
public:
    explicit Text(const QString& text = QString(), QObject* parent = nullptr);

    void draw(QPainter* painter) const override {};
    bool contains(const QPointF& point) const override {};
    QRectF boundingRect() const override {};

    void setText(const QString& text);
    QString text() const;

    void setFont(const QFont& font);
    QFont font() const;

    void setAlignment(Qt::Alignment alignment);
    Qt::Alignment alignment() const;


private:
    QString m_text;
    QFont m_font;
    Qt::Alignment m_alignment;
};


class Line : public StyleObject
{
    Q_OBJECT
public:
    Line(const QPointF& start, const QPointF& end, QObject* parent = nullptr);

    void draw(QPainter* painter) const override {};
    bool contains(const QPointF& point) const override {};
    QRectF boundingRect() const override {};

    void setStartPoint(const QPointF& point);
    QPointF startPoint() const;

    void setEndPoint(const QPointF& point);
    QPointF endPoint() const;

    void setLine(const QPointF& start, const QPointF& end);

private:
    QPointF m_start;
    QPointF m_end;
};


#endif // OBJECT_H
