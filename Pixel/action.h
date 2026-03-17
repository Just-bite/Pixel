#ifndef ACTION_H
#define ACTION_H

#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>

// Команда создания объекта
class AddObjectCommand : public QUndoCommand
{
public:
    AddObjectCommand(QGraphicsItem* parentLayer, QGraphicsItem* object, QUndoCommand *parent = nullptr);
    ~AddObjectCommand();

    void undo() override;
    void redo() override;

private:
    QGraphicsItem* m_parent_layer;
    QGraphicsItem* m_object;
};

// Команда удаления объекта
class DeleteObjectCommand : public QUndoCommand
{
public:
    DeleteObjectCommand(QGraphicsItem* parentLayer, QGraphicsItem* object, QUndoCommand *parent = nullptr);
    ~DeleteObjectCommand();

    void undo() override;
    void redo() override;

private:
    QGraphicsItem* m_parent_layer;
    QGraphicsItem* m_object;
};

#endif // ACTION_H