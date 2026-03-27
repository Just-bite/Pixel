#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene_main(new QGraphicsScene(this))
    , m_view_main(new QGraphicsView(this))
    , m_project_manager(new ProjectManager())
{
    ui->setupUi(this);

    QWidget *container_main = new QWidget(this);
    QVBoxLayout *container_layout = new QVBoxLayout(container_main);
    container_layout->setContentsMargins(0, 0, 0, 0);

    m_context_pannel_layout = new ContextPannel(container_main);

    QWidget *workspace = new QWidget(container_main);
    QHBoxLayout *workspace_layout = new QHBoxLayout(workspace);
    workspace_layout->setContentsMargins(1, 1, 1, 1);

    QWidget *instrument_pannel = new QWidget(workspace);
    m_instrument_pannel_layout = new InstrumentPannel(instrument_pannel);

    m_scene_main->setSceneRect(-10000, -10000, 20000, 20000);
    m_view_main->viewport()->setMouseTracking(true);


    m_project_manager->createProject();

    connect(m_project_manager, &ProjectManager::projectLoaded, this, [this](){
        if(m_workspace_controller) m_workspace_controller->getUndoStack()->clear();
    });
    // Чтобы боковая панель слоев обновилась при открытии файла
    connect(m_project_manager, &ProjectManager::layersUpdated, this, [this](){
        if(m_layers_pannel) {
            // Чтобы вызвать приватный метод updateLayers, можно просто симулировать выбор слоя
            m_layers_pannel->selectLayerFromOutside(0);
            // А лучше добавь метод forceUpdate() в layerspannel.h ;)
        }
    });

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
    canvas->addLayer(layer1);
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

    m_workspace_controller = new WorkspaceController(m_view_main, m_scene_main, m_project_manager, m_context_pannel_layout, palette_widget, m_layers_pannel, this);
    connect(m_instrument_pannel_layout, &InstrumentPannel::instrumentSelected, m_workspace_controller, &WorkspaceController::setCurrentTool);
    connect(m_workspace_controller, &WorkspaceController::viewportChanged, this, &MainWindow::updateInfoPanel);

    connect(m_project_manager, &ProjectManager::projectAboutToClose, m_workspace_controller, &WorkspaceController::clearState);

    connect(m_project_manager, &ProjectManager::projectLoaded, this, [this](){
        updateInfoPanel();
    });

    connect(m_project_manager, &ProjectManager::layersUpdated, m_layers_pannel, &LayersPannel::updateLayers);

    createMenuBar();
    QTimer::singleShot(0, this, &MainWindow::onFitToScreen);
}


void MainWindow::createMenuBar()
{
    QMenuBar *menu_bar = this->menuBar();
    QMenu *file_menu = menu_bar->addMenu("&File");

    // File
    QAction *create_action = file_menu->addAction("&New Project");
    connect(create_action, &QAction::triggered, m_project_manager, &ProjectManager::createFile);
    create_action->setShortcut(QKeySequence::New);

    QAction *open_action = file_menu->addAction("&Open Project...");
    connect(open_action, &QAction::triggered, m_project_manager, &ProjectManager::openFile);
    open_action->setShortcut(QKeySequence::Open);

    QAction *save_action = file_menu->addAction("&Save Project");
    connect(save_action, &QAction::triggered, m_project_manager, &ProjectManager::saveFile);
    save_action->setShortcut(QKeySequence::Save);

    QAction *save_as_action = file_menu->addAction("Save Project &As...");
    connect(save_as_action, &QAction::triggered, m_project_manager, &ProjectManager::saveAsFile);
    save_as_action->setShortcut(QKeySequence::SaveAs);

    QAction *export_action = file_menu->addAction("&Export to PNG...");
    connect(export_action, &QAction::triggered, m_project_manager, &ProjectManager::exportPng);
    export_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));

    // Edit (Undo / Redo)
    QMenu *edit_menu = menu_bar->addMenu("&Edit");
    if (m_workspace_controller) {
        QAction* undoAct = m_workspace_controller->getUndoStack()->createUndoAction(this, "&Undo");
        undoAct->setShortcut(QKeySequence::Undo);
        edit_menu->addAction(undoAct);

        QAction* redoAct = m_workspace_controller->getUndoStack()->createRedoAction(this, "&Redo");
        redoAct->setShortcut(QKeySequence::Redo);
        edit_menu->addAction(redoAct);
    }

    // View (Камера)
    QMenu *view_menu = menu_bar->addMenu("&View");

    QAction *zoom_in_act = view_menu->addAction("Zoom &In");
    // Оставляем только ZoomIn и Ctrl+= (иногда плюс без шифта это равно)
    zoom_in_act->setShortcuts({QKeySequence::ZoomIn, QKeySequence(Qt::CTRL | Qt::Key_Equal)});
    connect(zoom_in_act, &QAction::triggered, this, &MainWindow::onZoomIn);

    QAction *zoom_out_act = view_menu->addAction("Zoom &Out");
    // Убрали дубликат! QKeySequence::ZoomOut уже включает в себя Ctrl+-
    zoom_out_act->setShortcut(QKeySequence::ZoomOut);
    connect(zoom_out_act, &QAction::triggered, this, &MainWindow::onZoomOut);

    QAction *fit_act = view_menu->addAction("&Fit to Screen");
    fit_act->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(fit_act, &QAction::triggered, this, &MainWindow::onFitToScreen);

    // Help
    QMenu *help_menu = menu_bar->addMenu("&Help");
    QAction *about_act = help_menu->addAction("&About Pixel");
    connect(about_act, &QAction::triggered, this, [this](){
        QMessageBox::about(this, "About Pixel", "Pixel - Vector Graphics Editor\nWritten in C++ with Qt.");
    });
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

MainWindow::~MainWindow() {
    if (m_workspace_controller) {
        m_workspace_controller->clearState();
    }
    if (m_project_manager && m_project_manager->GetCurrentCanvas()) {
        m_project_manager->GetCurrentCanvas()->clearCanvas();
    }
    delete ui;
}

void MainWindow::onZoomIn() { m_view_main->scale(1.15, 1.15); updateInfoPanel(); }
void MainWindow::onZoomOut() { m_view_main->scale(1.0 / 1.15, 1.0 / 1.15); updateInfoPanel(); }
void MainWindow::onFitToScreen() {
    if (m_view_main->viewport()->width() < 10 || m_view_main->viewport()->height() < 10) return;
    Canvas *canvas = m_project_manager->GetCurrentCanvas();
    if (!canvas) return;
    QRectF canvasRect(0, 0, canvas->getSize().width(), canvas->getSize().height());
    m_view_main->fitInView(canvasRect, Qt::KeepAspectRatio);
    m_view_main->scale(0.95, 0.95);
    m_view_main->centerOn(canvasRect.center());
    updateInfoPanel();
}
void MainWindow::onSetAbsoluteZoom(float scale) {
    m_view_main->resetTransform();
    m_view_main->scale(scale, scale);
    updateInfoPanel();
}