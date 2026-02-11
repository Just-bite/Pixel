#ifndef LAYER_H
#define LAYER_H

#include <QPainter>
#include <QObject>
#include <vector>

#include "object.h"

class Layer : public QObject
{
    Q_OBJECT
public:
    explicit Layer(QObject* parent = nullptr);
    void draw(QPainter* painter) const;
    void addObject(Object* object);

private:
    bool m_visible;
    bool m_locked;

    std::vector<Object*> m_objects;
};

#endif // LAYER_H
