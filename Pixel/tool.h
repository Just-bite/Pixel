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
        Ellipse* el = new Ellipse(arg.first, 20, nullptr);
        el->setFillColor(Qt::cyan);
        canvas->addObjectToSelectedLayer(el);
    }
};

#endif // TOOL_H
