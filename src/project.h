#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <vector>

#include "canvas.h"

class Project : public QObject
{
    Q_OBJECT

public:
    Project();
    ~Project();

    Canvas* GetCanvas() {return m_canvas;}

private:
    Canvas* m_canvas;
};

#endif // PROJECT_H
