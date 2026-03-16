#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QPushButton>
#include <QResizeEvent>
#include <QSlider>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScrollBar>

#include "contextpannel.h"
#include "infopannel.h"
#include "instrumentpannel.h"
#include "layerspannel.h"
#include "palettepannel.h"

#include "canvas.h"
#include "layer.h"
#include "object.h"
#include "projectmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void createMenuBar();
    void openFile();
    Ui::MainWindow *ui;

    QGraphicsScene *m_scene_main;
    QGraphicsView *m_view_main;

    InfoPannel *m_info_pannel_layout;
    ContextPannel *m_context_pannel_layout;
    InstrumentPannel *m_instrument_pannel_layout;
    LayersPannel *m_layers_pannel;
    PalettePannel *m_palette_pannel_layout;

    ProjectManager *m_project_manager;

    // Переменные состояния для панорамирования и инструментов
    bool m_space_pressed = false;
    bool m_is_panning = false;
    QPoint m_last_pan_pos;
    bool m_is_drawing = false; // Состояние для отрисовки инструмента

private slots:
    void updateInfoPanel();
};
#endif // MAINWINDOW_H