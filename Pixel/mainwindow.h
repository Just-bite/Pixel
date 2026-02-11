#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>

#include "infopannel.h"
#include "contextpannel.h"
#include "instrumentpannel.h"

#include "object.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QGraphicsScene* scene_main;
    QGraphicsView* view_main;
    InfoPannel* info_pannel_layout;
    ContextPannel* context_pannel_layout;
    InstrumentPannel* instrument_pannel_layout;
};
#endif // MAINWINDOW_H
