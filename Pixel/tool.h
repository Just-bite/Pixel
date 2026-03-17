#ifndef TOOL_H
#define TOOL_H

#include "canvas.h"
#include "object.h"
#include "projectmanager.h"

/* Как вариант, возможно даже лучше, сделать не наследуемые классы от Tool,
 * а один единый класс Tool, а enum выбора самого инструмента вынести в ToolActArg.
 * Оно, вероятно будет проще, но не факт что лучше.*/


struct ToolActArg
{
    QPointF first;
    QPointF second;
    void* extra;
};

class Tool
{
public:
    Tool();

    virtual void actOnSelectedArea(Canvas* canvas, ToolActArg arg) = 0;
};


class FigureTool : public Tool
{
public:
    FigureTool();

    void actOnSelectedArea(Canvas* canvas, ToolActArg arg) override
    {
        // Создаем прямоугольник 40x40 вокруг кликнутой точки
        QRectF rect(arg.first.x() - 20, arg.first.y() - 20, 40, 40);
        Ellipse* el = new Ellipse(rect);
        el->setFillColor(Qt::cyan);
        canvas->addObjectToSelectedLayer(el);
    }
};

#endif // TOOL_H
