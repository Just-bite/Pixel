#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene_main(new QGraphicsScene(this))
    , m_view_main(new QGraphicsView(this))
{
    ui->setupUi(this);

    createMenuBar();
;
    // start main container config
    QWidget* container_main = new QWidget(this);
    QVBoxLayout* container_layout = new QVBoxLayout(container_main);
    container_layout->setContentsMargins(0,0,0,0);

    // start 1
    QWidget* context_pannel = new QWidget(container_main);
    m_context_pannel_layout = new ContextPannel(context_pannel);
    context_pannel->setStyleSheet("border: 2px solid #ff0000; border-radius: 5px;");
    // end 1

    // start 2
    QWidget* workspace = new QWidget(container_main);
    QHBoxLayout* workspace_layout = new QHBoxLayout(workspace);
    workspace_layout->setContentsMargins(1,1,1,1);

    //
    QWidget* instrument_pannel = new QWidget(workspace);
    m_instrument_pannel_layout = new InstrumentPannel(instrument_pannel);
    instrument_pannel->setStyleSheet("border: 2px solid #ffff00; border-radius: 5px;");
    //

    //
    m_scene_main->setSceneRect(QRect(this->width()/ 7,this->height()/ 9,4 * this->width()/ 7,7* this->height()/ 9));
    m_scene_main->addRect(m_scene_main->sceneRect());

    m_view_main->setScene(m_scene_main);
    m_view_main->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    m_view_main->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    //

    //
    QWidget* palette_layers_pannel = new QWidget(workspace);
    QVBoxLayout* palette_layers_pannel_layout = new QVBoxLayout(palette_layers_pannel);
    palette_layers_pannel->setStyleSheet("border: 2px solid #f3003f; border-radius: 5px;");

    QWidget* layers_pannel = new QWidget(workspace);
    QVBoxLayout* layers_pannel_layout = new QVBoxLayout(layers_pannel);
    QPushButton* filler_button3 = new QPushButton( "Filler button ", layers_pannel);
    layers_pannel_layout->addWidget(filler_button3);
    layers_pannel->setStyleSheet("border: 2px solid #ff00ff; border-radius: 5px;");

    QWidget* palette_pannel = new QWidget(workspace);
    QVBoxLayout* palette_pannel_layout = new QVBoxLayout(palette_pannel);
    PalettePannel* palette = new PalettePannel(workspace);
    QSlider* slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, 359);
    slider->setValue(0);

    palette_pannel_layout->addWidget(palette);
    palette_pannel_layout->addWidget(slider);

    connect(slider, &QSlider::valueChanged, palette, &PalettePannel::setHue);

    palette_pannel->setStyleSheet("border: 2px solid #f0400f; border-radius: 5px;");

    palette_layers_pannel_layout->addWidget(palette_pannel, 6);
    palette_layers_pannel_layout->addWidget(layers_pannel, 4);

    //
    workspace_layout->addWidget(instrument_pannel);
    workspace_layout->addWidget(m_view_main, 4);
    workspace_layout->addWidget(palette_layers_pannel, 2);

    // end 2

    // start 3
    QWidget* info_pannel = new QWidget(container_main);
    m_info_pannel_layout = new InfoPannel({m_scene_main->width(),m_scene_main->height()}, 1.0f, info_pannel);
    info_pannel->setStyleSheet("border: 2px solid #00ffff; border-radius: 5px;");
    // end 3

    container_layout->addWidget(context_pannel,1);
    container_layout->addWidget(workspace,8);
    container_layout->addWidget(info_pannel,0);
    container_layout->setMargin(0);
    container_layout->setSpacing(0);
    // end main container config

    setCentralWidget(container_main);

    {
        m_canvas = new Canvas();

        Layer* layer1 = new Layer(m_canvas);
        Layer* layer2 = new Layer(m_canvas);

        Ellipse* e1 = new Ellipse(QPointF(300, 300), 30, layer1);
        Ellipse* e2 = new Ellipse(QPointF(260, 300), 60, layer2);

        layer1->addObject(e1);
        layer2->addObject(e2);

        m_canvas->addLayer(layer1);
        m_canvas->addLayer(layer2);
    }

    renderCanvas();

}

void MainWindow::createMenuBar()
{
    QMenuBar* menu_bar = this->menuBar();

    QMenu* file_menu = menu_bar->addMenu("&File");
    file_menu->addAction("&Create file or project");
    file_menu->addAction("&Open file or project");
    file_menu->addAction("&Save project");
    file_menu->addAction("&Save project as");
    file_menu->addAction("&Close project");
    file_menu->addAction("&Print");

    QMenu* edit_menu = menu_bar->addMenu("&Edit");
    edit_menu->addAction("&Undo");
    edit_menu->addAction("&Redo");
    edit_menu->addAction("&Copy");
    edit_menu->addAction("&Paste");
    edit_menu->addAction("&Search");

    QMenu* view_menu = menu_bar->addMenu("&View");
    view_menu->addAction("&Me cant see");

    QMenu* help_menu = menu_bar->addMenu("&Help");
    help_menu->addAction("&Sos me die");
}

void MainWindow::renderCanvas()
{
    QRectF rect = m_scene_main->sceneRect();
    QPixmap buffer(rect.size().toSize());

    buffer.fill(Qt::white);

    QPainter painter(&buffer);


    if (m_canvas) {
        m_canvas->draw(&painter);
    }
    painter.end();

    m_scene_main->clear();
    m_scene_main->addPixmap(buffer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

