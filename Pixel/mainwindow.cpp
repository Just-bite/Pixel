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
    workspace_layout->setContentsMargins(1,1,1,1);

    //
    QWidget* instrument_pannel = new QWidget(workspace);
    //QVBoxLayout* instrument_pannel_layout = new QVBoxLayout(instrument_pannel);
    instrument_pannel_layout = new InstrumentPannel(instrument_pannel);
   // QPushButton* filler_button2 = new QPushButton( "Filler button ",instrument_pannel);
    //filler_button2->setFixedSize(20,20);
    //instrument_pannel_layout->addWidget(filler_button2);
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
    QWidget* palette_layers_pannel = new QWidget(workspace);
   // palette_layers_pannel->setMaximumWidth(260);
    QVBoxLayout* palette_layers_pannel_layout = new QVBoxLayout(palette_layers_pannel);
    palette_layers_pannel->setStyleSheet("border: 2px solid #f3003f; border-radius: 5px;");

    QWidget* layers_pannel = new QWidget(workspace);
    QVBoxLayout* layers_pannel_layout = new QVBoxLayout(layers_pannel);
    QPushButton* filler_button3 = new QPushButton( "Filler button ", layers_pannel);
    layers_pannel_layout->addWidget(filler_button3);
    layers_pannel->setStyleSheet("border: 2px solid #ff00ff; border-radius: 5px;");

    QWidget* palette_pannel = new QWidget(workspace);
    QVBoxLayout* palette_pannel_layout = new QVBoxLayout(palette_pannel);
    QPushButton* filler_button5 = new QPushButton( "Filler button ", palette_pannel);
    palette_pannel_layout->addWidget(filler_button5);
    palette_pannel->setStyleSheet("border: 2px solid #f0400f; border-radius: 5px;");

    palette_layers_pannel_layout->addWidget(palette_pannel, 6);
    palette_layers_pannel_layout->addWidget(layers_pannel, 4);


    //
    workspace_layout->addWidget(instrument_pannel);
    workspace_layout->addWidget(view_main, 4);
    workspace_layout->addWidget(palette_layers_pannel, 2);

    // end 2

    // start 3
    QWidget* info_pannel = new QWidget(container_main);
    info_pannel_layout = new InfoPannel({scene_main->width(),scene_main->height()}, 1.0f, info_pannel);
    info_pannel->setStyleSheet("border: 2px solid #00ffff; border-radius: 5px;");
    // end 3

    container_layout->addWidget(context_pannel,1);
    container_layout->addWidget(workspace,8);
    container_layout->addWidget(info_pannel,0);
    container_layout->setMargin(0);
    container_layout->setSpacing(0);
    // end main container config

    setCentralWidget(container_main);
}

MainWindow::~MainWindow()
{
    delete ui;
}

