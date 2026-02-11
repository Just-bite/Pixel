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
#include "palettepannel.h"

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

    QGraphicsScene* m_scene_main;
    QGraphicsView* m_view_main;
    InfoPannel* m_info_pannel_layout;
    ContextPannel* m_context_pannel_layout;
    InstrumentPannel* m_instrument_pannel_layout;
    PalettePannel* m_palette_pannel_layout;
};
#endif // MAINWINDOW_H
