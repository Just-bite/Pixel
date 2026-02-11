#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <vector>

#include "layer.h"


class Project : public QObject
{
    Q_OBJECT

public:
    Project();

private:
    std::vector<Layer*> m_layers;
    Layer* m_selected;
};

#endif // PROJECT_H
