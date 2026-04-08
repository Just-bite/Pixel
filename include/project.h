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

    bool getAskRasterize() const { return m_ask_rasterize; }
    void setAskRasterize(bool val) { m_ask_rasterize = val; }

private:
    Canvas* m_canvas;
    bool m_ask_rasterize = true;
};

#endif // PROJECT_H