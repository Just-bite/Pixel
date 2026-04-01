#ifndef TOOL_H
#define TOOL_H

#include "canvas.h"
#include "object.h"
#include "projectmanager.h"

struct ToolActArg
{
    QPointF first;
    QPointF second;
    void* extra;
};

class Tool
{
public:
    Tool() = default;

    virtual void actOnSelectedArea(Canvas* canvas, ToolActArg arg) = 0;
};

class FigureTool : public Tool
{
public:
    FigureTool() = default;

    void actOnSelectedArea(Canvas* canvas, ToolActArg arg) override
    {
        QRectF rect(arg.first.x() - 20, arg.first.y() - 20, 40, 40);

        Figure* fig = new Figure(rect, FigureType::Ellipse);

        FigureState s = fig->getState();
        s.fill = Qt::cyan;
        fig->setState(s);

        canvas->addObjectToSelectedLayer(fig);
    }
};

#endif // TOOL_H