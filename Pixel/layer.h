#ifndef LAYER_H
#define LAYER_H

#include <QGraphicsObject>
#include <QString>
#include <vector>

#include "object.h"

struct LayerInfo
{
    QString name;
    bool visible;
    bool locked;
};

// Теперь Layer - это невидимый контейнер (папка) на сцене Qt
class Layer : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit Layer(QGraphicsItem* parent = nullptr);
    explicit Layer(const QString& name, QGraphicsItem* parent = nullptr);

    ~Layer();

    void addObject(Object* object);

    void setName(const QString& name) { m_name = name; }
    QString getName() const { return m_name; }

    void setVisible(bool visible); // Изменим реализацию
    bool isVisible() const { return m_visible; }

    void setLocked(const bool locked) { m_locked = locked; }
    bool isLosked() const { return m_locked; }

    LayerInfo getInfo() const;

    // Обязательные методы QGraphicsItem (оставим их пустыми, слой сам себя не рисует)
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    bool m_visible;
    bool m_locked;
    QString m_name;
    std::vector<Object*> m_objects;
};

#endif // LAYER_H