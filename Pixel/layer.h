#ifndef LAYER_H
#define LAYER_H

#include <QPainter>
#include <QObject>
#include <QString>
#include <vector>

#include "object.h"

struct LayerInfo
{
    QString name;
    bool visible;
    bool locked;
};

class Layer : public QObject
{
    Q_OBJECT
public:
    explicit Layer(QObject* parent = nullptr);
    explicit Layer(const QString& name, QObject* parent = nullptr);

    ~Layer();

    void draw(QPainter* painter) const;
    void addObject(Object* object);

    void setName(const QString& name) { m_name = name; }
    QString getName() const { return m_name; }

    void setVisible(const bool vissible) { m_visible = vissible; }
    bool isVisible() const { return m_visible; }

    void setLocked(const bool locked) { m_locked = locked; }
    bool isLosked() const { return m_locked; }

    LayerInfo getInfo() const;



private:
    bool m_visible;
    bool m_locked;
    QString m_name;
    std::vector<Object*> m_objects;
};

#endif // LAYER_H
