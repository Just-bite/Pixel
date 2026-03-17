#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene_main(new QGraphicsScene(this))
    , m_view_main(new QGraphicsView(this))
    , m_project_manager(new ProjectManager(this))
{
    ui->setupUi(this);

    createMenuBar();

    QWidget *container_main = new QWidget(this);
    QVBoxLayout *container_layout = new QVBoxLayout(container_main);
    container_layout->setContentsMargins(0, 0, 0, 0);

    // ИСПРАВЛЕНИЕ: Убрали лишний пустой QWidget-обертку. Сразу создаем панель на container_main.
    m_context_pannel_layout = new ContextPannel(container_main);

    QWidget *workspace = new QWidget(container_main);
    QHBoxLayout *workspace_layout = new QHBoxLayout(workspace);
    workspace_layout->setContentsMargins(1, 1, 1, 1);

    QWidget *instrument_pannel = new QWidget(workspace);
    m_instrument_pannel_layout = new InstrumentPannel(instrument_pannel);

    m_scene_main->setSceneRect(-10000, -10000, 20000, 20000);
    m_view_main->viewport()->setMouseTracking(true);

    m_project_manager = new ProjectManager();
    m_project_manager->createProject();
    Canvas *canvas = m_project_manager->GetCurrentCanvas();
    canvas->setScene(m_scene_main);
    m_view_main->setScene(m_scene_main);

    m_view_main->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    m_view_main->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    m_view_main->setDragMode(QGraphicsView::NoDrag);

    QWidget *palette_layers_pannel = new QWidget(workspace);
    QVBoxLayout *palette_layers_pannel_layout = new QVBoxLayout(palette_layers_pannel);
    palette_layers_pannel->setStyleSheet("border: 1px solid #555555; ");
    palette_layers_pannel->setMaximumWidth(300); // Немного сузили для компактности

    Layer *layer1 = new Layer("layer1");
    Layer *layer2 = new Layer("layer2");
    canvas->addLayer(layer1);
    canvas->addLayer(layer2);
    canvas->renderCanvas();

    m_layers_pannel = new LayersPannel(palette_layers_pannel, canvas);

    QWidget *palette_pannel = new QWidget(workspace);
    QVBoxLayout *palette_pannel_layout = new QVBoxLayout(palette_pannel);

    PalettePannel *palette_widget = new PalettePannel(workspace);
    palette_pannel_layout->addWidget(palette_widget);

    palette_layers_pannel_layout->addWidget(palette_pannel, 5);
    palette_layers_pannel_layout->addWidget(m_layers_pannel, 5);

    workspace_layout->addWidget(instrument_pannel);
    workspace_layout->addWidget(m_view_main, 4);
    workspace_layout->addWidget(palette_layers_pannel, 2);

    QWidget *info_pannel = new QWidget(container_main);
    m_info_pannel_layout = new InfoPannel({canvas->getSize().width(), canvas->getSize().height()}, 1.0f, info_pannel);

    connect(m_info_pannel_layout, &InfoPannel::zoomInRequested, this, &MainWindow::onZoomIn);
    connect(m_info_pannel_layout, &InfoPannel::zoomOutRequested, this, &MainWindow::onZoomOut);
    connect(m_info_pannel_layout, &InfoPannel::fitRequested, this, &MainWindow::onFitToScreen);
    connect(m_info_pannel_layout, &InfoPannel::scaleChanged, this, &MainWindow::onSetAbsoluteZoom);

    // ИСПРАВЛЕНИЕ: Добавляем контекстную панель напрямую в layout!
    container_layout->addWidget(m_context_pannel_layout, 0);
    container_layout->addWidget(workspace, 8);
    container_layout->addWidget(info_pannel, 0);
    container_layout->setContentsMargins(0, 0, 0, 0);
    container_layout->setSpacing(0);

    setCentralWidget(container_main);
    updateInfoPanel();

    m_workspace_controller = new WorkspaceController(
        m_view_main, m_scene_main, m_project_manager,
        m_context_pannel_layout, palette_widget, m_layers_pannel,
        this);

    connect(m_instrument_pannel_layout, &InstrumentPannel::instrumentSelected, m_workspace_controller, &WorkspaceController::setCurrentTool);
    connect(m_workspace_controller, &WorkspaceController::viewportChanged, this, &MainWindow::updateInfoPanel);

    QTimer::singleShot(0, this, &MainWindow::onFitToScreen);
}

void MainWindow::createMenuBar()
{
    QMenuBar *menu_bar = this->menuBar();
    QMenu *file_menu = menu_bar->addMenu("&File");

    QAction *create_action = file_menu->addAction("&Create file or project");
    connect(create_action, &QAction::triggered, m_project_manager, &ProjectManager::createFile);
    create_action->setShortcut(QKeySequence::New);

    QAction *open_action = file_menu->addAction("&Open file or project");
    connect(open_action, &QAction::triggered, m_project_manager, &ProjectManager::openFile);
    open_action->setShortcut(QKeySequence::Open);

    QAction *save_as_action = file_menu->addAction("&Save project as");
    connect(save_as_action, &QAction::triggered, m_project_manager, &ProjectManager::saveAsFile);
    save_as_action->setShortcut(QKeySequence::SaveAs);

    QAction *save_action = file_menu->addAction("&Save project");
    connect(save_action, &QAction::triggered, m_project_manager, &ProjectManager::saveFile);
    save_action->setShortcut(QKeySequence::Save);

    QAction *close_action = file_menu->addAction("&Close project");
    connect(close_action, &QAction::triggered, m_project_manager, &ProjectManager::closeFile);
    close_action->setShortcut(QKeySequence::Close);

    QAction *print_action = file_menu->addAction("&Print");
    connect(print_action, &QAction::triggered, m_project_manager, &ProjectManager::printFile);
    print_action->setShortcut(QKeySequence::Print);

    QMenu *edit_menu = menu_bar->addMenu("&Edit");
    edit_menu->addAction("&Undo");
    edit_menu->addAction("&Redo");

    QMenu *view_menu = menu_bar->addMenu("&View");
    QMenu *help_menu = menu_bar->addMenu("&Help");
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateInfoPanel();
}

void MainWindow::updateInfoPanel()
{
    if (!m_info_pannel_layout || !m_project_manager) return;

    Canvas *canvas = m_project_manager->GetCurrentCanvas();
    if (!canvas) return;

    int width = canvas->getSize().width();
    int height = canvas->getSize().height();

    m_info_pannel_layout->setCanvasSize({width, height});
    m_info_pannel_layout->updateCanvasSizeDisplay(width, height);

    qreal scaleX = m_view_main->transform().m11();
    float scale = static_cast<float>(scaleX);

    m_info_pannel_layout->setScale(scale);
    m_info_pannel_layout->updateScaleDisplay(scale);
}

MainWindow::~MainWindow() { delete ui; }
void MainWindow::onZoomIn() { m_view_main->scale(1.15, 1.15); updateInfoPanel(); }
void MainWindow::onZoomOut() { m_view_main->scale(1.0 / 1.15, 1.0 / 1.15); updateInfoPanel(); }
void MainWindow::onFitToScreen() {
    if (m_view_main->viewport()->width() < 10 || m_view_main->viewport()->height() < 10) return;
    Canvas *canvas = m_project_manager->GetCurrentCanvas();
    if (!canvas) return;
    QRectF canvasRect(0, 0, canvas->getSize().width(), canvas->getSize().height());
    m_view_main->fitInView(canvasRect, Qt::KeepAspectRatio);
    m_view_main->scale(0.95, 0.95);
    updateInfoPanel();
}
void MainWindow::onSetAbsoluteZoom(float scale) {
    m_view_main->resetTransform();
    m_view_main->scale(scale, scale);
    updateInfoPanel();
}