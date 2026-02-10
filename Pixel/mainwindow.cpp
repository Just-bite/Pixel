#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene_main(new QGraphicsScene(this))
    , view_main(new QGraphicsView(this))
{
    ui->setupUi(this);

    // start main container config
    QWidget* container_main = new QWidget(this);
    QVBoxLayout* container_layout = new QVBoxLayout(container_main);
    container_layout->setContentsMargins(0,0,0,0);

    // start 1
    QWidget* context_pannel = new QWidget(container_main);
    context_pannel_layout = new ContextPannel(context_pannel);
    context_pannel->setStyleSheet("border: 2px solid #ff0000; border-radius: 5px;");
    // end 1

    // start 2
    QWidget* workspace = new QWidget(container_main);
    QHBoxLayout* workspace_layout = new QHBoxLayout(workspace);

    //
    QWidget* instrument_pannel = new QWidget(workspace);
    QVBoxLayout* instrument_pannel_layout = new QVBoxLayout(instrument_pannel);
    QPushButton* filler_button2 = new QPushButton( "Filler button ",instrument_pannel);
    instrument_pannel_layout->addWidget(filler_button2);
    instrument_pannel->setStyleSheet("border: 2px solid #ffff00; border-radius: 5px;");
    //

    //
    scene_main->setSceneRect(QRect(this->width()/ 7,this->height()/ 9,4 * this->width()/ 7,7* this->height()/ 9));
    scene_main->addRect(scene_main->sceneRect());
    view_main->setScene(scene_main);
    view_main->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    view_main->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    //

    //
    QWidget* color_layers_pannel = new QWidget(workspace);
    QVBoxLayout* color_layers_pannel_layout = new QVBoxLayout(color_layers_pannel);
    QPushButton* filler_button3 = new QPushButton( "Filler button ", color_layers_pannel);
    color_layers_pannel_layout->addWidget(filler_button3);
    color_layers_pannel->setStyleSheet("border: 2px solid #ff00ff; border-radius: 5px;");

    //
    workspace_layout->addWidget(instrument_pannel, 1);
    workspace_layout->addWidget(view_main, 4);
    workspace_layout->addWidget(color_layers_pannel, 2);

    // end 2

    // start 3
    QWidget* info_pannel = new QWidget(container_main);
    info_pannel_layout = new InfoPannel({scene_main->width(),scene_main->height()}, 1.0f, info_pannel);
    info_pannel->setStyleSheet("border: 2px solid #00ffff; border-radius: 5px;");
    // end 3

    container_layout->addWidget(context_pannel,1);
    container_layout->addWidget(workspace,7);
    container_layout->addWidget(info_pannel,1);
    container_layout->setMargin(0);
    container_layout->setSpacing(0);
    // end main container config

    setCentralWidget(container_main);
}

MainWindow::~MainWindow()
{
    delete ui;
}

