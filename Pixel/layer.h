#ifndef LAYER_H
#define LAYER_H

#include <QGraphicsObject>
#include <QString>

#include "object.h"

struct LayerInfo {
    QString name;
    bool visible;
    bool locked;
};

class Layer : public QGraphicsObject {
    Q_OBJECT
public:
    explicit Layer(QGraphicsItem* parent = nullptr);
    explicit Layer(const QString& name, QGraphicsItem* parent = nullptr);
    ~Layer() override;

    void addObject(Object* object);
    void removeObject(Object* object);

    void setName(const QString& name) { m_name = name; }
    QString getName() const { return m_name; }

    void setVisible(bool visible);
    bool isVisible() const { return m_visible; }

    void setLocked(const bool locked);
    bool isLocked() const { return m_locked; }

    LayerInfo getInfo() const;

    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    bool m_visible;
    bool m_locked;
    QString m_name;
};

#endif // LAYER_H