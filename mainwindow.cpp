#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMenuBar>
#include <QMessageBox>
#include <QMenu>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QSurfaceFormat>
#include "mapsizedialog.h"  // 添加头文件

#include <QFileDialog>       // QFileDialog
#include <QStandardPaths>    // QStandardPaths
#include <QFile>             // QFile
#include <QFileInfo>         // QFileInfo
#include <QTextStream>       // QTextStream
#include <QSettings>

#include <fstream>  // 添加文件流支持
#include "third_party/json-develop/include/nlohmann/json.hpp"
#include <nlohmann/json.hpp>  // 如果路径配置正确，可以直接这样写
using json = nlohmann::json;  // 添加类型别名

// 主窗口构造函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化窗口属性
    setWindowTitle("栅格地图编辑器");
    // 初始化图形视图
    view = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    view->setScene(scene);

    setCentralWidget(view);  // 将视图设置为主窗口中心部件

    // 设置滚动条策略（默认即可自动显示）
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 启用触控板惯性滚动
    QScroller::grabGesture(view->viewport(), QScroller::LeftMouseButtonGesture);
    view->setDragMode(QGraphicsView::ScrollHandDrag);  // 手掌拖拽模式
    view->setInteractive(true);  // 确保视图可交互

    // 设置抗锯齿
    view->setRenderHint(QPainter::Antialiasing);

    // 优化视图渲染参数
    view->setOptimizationFlags(QGraphicsView::DontSavePainterState |
                               QGraphicsView::DontAdjustForAntialiasing);
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // 启用OpenGL加速（需Qt配置支持）
    QSurfaceFormat format;
    format.setSamples(4); // 多重采样抗锯齿
    QOpenGLWidget *glWidget = new QOpenGLWidget;
    glWidget->setFormat(format);
    view->setViewport(glWidget);

    setCentralWidget(view);  // 重要！必须设置视图为中央部件

    // ------------------------------------------------------
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);

    // 左侧文件内容区域
    fileContentWidget = new CodeEditor;  // 替换为自定义编辑器
    fileContentWidget->setReadOnly(true);
    mainSplitter->addWidget(fileContentWidget);

    // 右侧地图视图（原视图改造）
    view->setScene(scene);
    mainSplitter->addWidget(view);

    // 样式设置
    fileContentWidget->setStyleSheet(R"(
    QPlainTextEdit {
        background-color: #ffffff;
        color: #333333;
    }
    .CodeEditor > QWidget {
        border-right: 1px solid #cccccc;
    }
)");

    // 设置分割器属性
    mainSplitter->setHandleWidth(3); // 分割线宽度
    mainSplitter->setStyleSheet("QSplitter::handle { background: #666; }");
    mainSplitter->setSizes({200, 600}); // 初始宽度分配

    setCentralWidget(mainSplitter);

    // 创建菜单系统
    createMenus();

}

MainWindow::~MainWindow()
{
    delete ui;
}

// 菜单系统构建模块，创建菜单栏及各类菜单项
void MainWindow::createMenus()
{
    // 获取主菜单栏（从UI自动生成的）
    QMenuBar *menuBar = this->menuBar();

    // 文件菜单
    QMenu *fileMenu = menuBar->addMenu("文件(&F)");
    QAction *newMapAction = fileMenu->addAction("新建地图");
    connect(newMapAction, &QAction::triggered, this, &MainWindow::onNewMapTriggered);

    // 新增窗口动作
    QAction *newWindowAction = fileMenu->addAction("新建窗口");
    connect(newWindowAction, &QAction::triggered,
            this, &MainWindow::onNewWindowTriggered);
    fileMenu->addSeparator();

    // 打开文件动作
    QAction *openAction = fileMenu->addAction("打开文件");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, [this]() {
        this->onOpenFileTriggered(); // 不传递参数，使用默认值
    });

    // 读入地图动作
    QAction *loadMapAction = fileMenu->addAction("读入地图");
    connect(loadMapAction, &QAction::triggered, this, &MainWindow::onLoadMapTriggered);

    QAction *saveAction = fileMenu->addAction("保存地图");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFileTriggered);

    fileMenu->addSeparator();
    // 退出动作
    QAction *exitAction = fileMenu->addAction("退出");
    exitAction->setShortcut(QKeySequence::Quit); // 设置快捷键（如Ctrl+Q）
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit); // 连接退出信号

    // 编辑菜单
    QMenu *editMenu = menuBar->addMenu("编辑(&E)");
    editMenu->addAction("撤销");
    editMenu->addAction("恢复");
    editMenu->addSeparator();
    editMenu->addAction("随机生成障碍");


    // 运行菜单
    QMenu *runMenu = menuBar->addMenu("运行(&R)");
    runMenu->addAction("运行代码");
    runMenu->addSeparator();
    runMenu->addAction("启动调试");

    // 工具菜单
    QMenu *toolsMenu = menuBar->addMenu("工具(&T)");
    toolsMenu->addAction("计时");

    // 帮助菜单
    QMenu *helpMenu = menuBar->addMenu("帮助(&H)");
    helpMenu->addAction("用户手册");
    helpMenu->addAction("报告bug");
    helpMenu->addAction("关于");
}

// 记录鼠标位置，鼠标移动事件处理，更新最后记录的鼠标位置
void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    m_lastMousePos = event->pos();  // 更新鼠标位置
    QMainWindow::mouseMoveEvent(event);
}

// 新建地图槽函数实现，新建地图菜单项触发，弹出尺寸设置对话框
void MainWindow::onNewMapTriggered()
{
    MapSizeDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        if (dlg.getRows() <= 0 || dlg.getCols() <= 0) {
            QMessageBox::warning(this, "错误", "必须输入有效行列数");
            return;
        }
        createGrid(dlg.getRows(), dlg.getCols());
    }
}

// 实现网格绘制函数，根据行列数创建栅格地图场景
void MainWindow::createGrid(int rows, int cols, int cellSize)
{
    scene->clear();
    m_gridData.clear();
    m_startPos = QPoint(-1, -1);
    m_endPos = QPoint(-1, -1);

    m_rows = rows;  // 新增行数记录
    m_cols = cols;  // 新增列数记录

    // 初始化全为可通行状态
    QVector<QVector<GridCell::CellType>> newGrid(rows,QVector<GridCell::CellType>(cols, GridCell::Passable));
    m_gridData.swap(newGrid);

    const int margin = 10;
    m_cellSize = (cellSize > 0) ? cellSize : qMax(20, 400 / qMax(rows, cols));

    // // 创建栅格单元格
    // for (int row = 0; row < rows; ++row) {
    //     QVector<GridCell::CellType> rowData;
    //     for (int col = 0; col < cols; ++col) {
    //         GridCell* cell = new GridCell(row, col, m_cellSize, this);
    //         cell->setPos(margin + col * m_cellSize, margin + row * m_cellSize);
    //         scene->addItem(cell);
    //         rowData.append(GridCell::Passable);
    //     }
    //     m_gridData.append(rowData);
    // }

    // 创建栅格单元格
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            GridCell* cell = new GridCell(row, col, m_cellSize, this);
            cell->setPos(margin + col * m_cellSize, margin + row * m_cellSize);
            scene->addItem(cell);
        }
    }

    // 设置场景尺寸
    scene->setSceneRect(0, 0,
        margin * 2 + cols * m_cellSize,
        margin * 2 + rows * m_cellSize);
}

// 修改保存文件槽函数，保存文件菜单项触发，执行地图保存操作
void MainWindow::onSaveFileTriggered()  // 移除参数
{
    if (m_rows <= 0 || m_cols <= 0 || scene->items().empty()) {
        QMessageBox::warning(this, "错误", "请先创建有效地图");
        return;
    }

    // 添加对起点终点的检测
    if (m_startPos == QPoint(-1, -1)) {
        QMessageBox::warning(this, "警告", "未设置起点");
    }
    if (m_endPos == QPoint(-1, -1)) {
        QMessageBox::warning(this, "警告", "未设置终点");
    }

    QString initialDir = getLastFileDirectory();
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存地图文件",
        initialDir,
        "JSON文件 (*.json);;所有文件 (*)"
        );

    if (!fileName.isEmpty()) {
        saveMapToFile(fileName);
        updateLastFileDirectory(fileName);
    }
}

// 读入地图槽函数实现
void MainWindow::onLoadMapTriggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开地图文件",
        getLastFileDirectory(),
        "JSON文件 (*.json);;所有文件 (*)"
        );
    if (!fileName.isEmpty()) {
        loadMapFromFile(fileName);
    }
}

// 从JSON文件加载地图
void MainWindow::loadMapFromFile(const QString &filePath)
{
    try {
        std::ifstream file(filePath.toStdString());
        if (!file.is_open()) {
            throw std::runtime_error("无法打开文件");
        }

        json j;
        file >> j;

        // 解析元数据
        int rows = j["metadata"]["rows"];
        int cols = j["metadata"]["cols"];
        int cellSize = j["metadata"]["cell_size"];

        // 验证元数据
        if (rows <= 0 || cols <= 0 || cellSize <= 0) {
            throw std::runtime_error("元数据中的行、列或单元格大小无效");
        }

        // 创建网格（使用保存的单元格大小）
        createGrid(rows, cols, cellSize);

        // 解析网格数据
        auto& gridData = j["grid"];
        if (gridData.size() != static_cast<size_t>(rows)) {
            throw std::runtime_error("行数不匹配");
        }
        for (int row = 0; row < rows; ++row) {
            if (gridData[row].size() != static_cast<size_t>(cols)) {
                throw std::runtime_error(QString("第%1行列数不匹配").arg(row).toStdString());
            }
            for (int col = 0; col < cols; ++col) {
                int val = gridData[row][col];
                m_gridData[row][col] = (val == 1) ? GridCell::Obstacle : GridCell::Passable;

                // 更新单元格显示
                GridCell* cell = getCell(QPoint(col, row));
                if (cell) {
                    cell->setType(m_gridData[row][col]);
                    updateCellVisual(row, col);
                }
            }
        }

        // 处理特殊点
        if (j.contains("special_points")) {
            auto& sp = j["special_points"];

            // 设置起点
            if (sp.contains("start")) {
                auto start = sp["start"];
                int x = start[0];
                int y = start[1];
                if (x < 0 || x >= cols || y < 0 || y >= rows) {
                    throw std::runtime_error("起点坐标超出范围");
                }
                updateStartPos(QPoint(x, y));
            }

            // 设置终点
            if (sp.contains("end")) {
                auto end = sp["end"];
                int x = end[0];
                int y = end[1];
                if (x < 0 || x >= cols || y < 0 || y >= rows) {
                    throw std::runtime_error("终点坐标超出范围");
                }
                updateEndPos(QPoint(x, y));
            }
        }

        // 更新最后访问目录
        updateLastFileDirectory(filePath);
        QMessageBox::information(this, "成功", "地图加载成功");

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("加载失败: %1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(this, "错误", "发生未知错误");
    }
}

// 将当前地图数据序列化为JSON格式保存
void MainWindow::saveMapToFile(const QString& filePath) {
    try {
        json j;
        j["metadata"]["rows"] = m_rows;
        j["metadata"]["cols"] = m_cols;
        j["metadata"]["cell_size"] = m_cellSize;

        // 记录特殊点坐标
        if (m_startPos != QPoint(-1, -1)) {
            j["special_points"]["start"] = json::array({m_startPos.x(), m_startPos.y()});
        }
        if (m_endPos != QPoint(-1, -1)) {
            j["special_points"]["end"] = json::array({m_endPos.x(), m_endPos.y()});
        }

        // 优化存储结构
        json gridData = json::array();
        for (int row = 0; row < m_rows; ++row) {
            json rowData = json::array();
            for (int col = 0; col < m_cols; ++col) {
                rowData.push_back(m_gridData[row][col] == GridCell::Obstacle ? 1 : 0);
            }
            gridData.push_back(rowData);
        }
        j["grid"] = gridData;

        std::ofstream file(filePath.toStdString());
        file << std::setw(4) << j;
    } catch (...) {
        QMessageBox::critical(this, "错误", "保存文件失败");
    }
}

// 添加鼠标事件处理，鼠标左键点击处理，设置障碍物单元格
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = view->mapToScene(event->pos());
        int col = static_cast<int>((scenePos.x() - 10) / m_cellSize);
        int row = static_cast<int>((scenePos.y() - 10) / m_cellSize);

        if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
            m_gridData[row][col] = GridCell::Obstacle;
            updateCellVisual(row, col);
        }
    }
    QMainWindow::mousePressEvent(event);  // 调用基类处理
}

// 添加单元格更新函数，更新指定单元格的视觉表现（颜色）
void MainWindow::updateCellVisual(int row, int col)
{
    QRectF rect(
        10 + col * m_cellSize + 1,
        10 + row * m_cellSize + 1,
        m_cellSize - 2,
        m_cellSize - 2
        );

    QList<QGraphicsItem*> items = scene->items(rect, Qt::IntersectsItemBoundingRect);
    for (QGraphicsItem* item : items) {
        if (item->data(0).toString() == "cell_fill") {
            delete item;
        }
    }

    if (m_gridData[row][col] != 0) {
        QGraphicsRectItem* fill = scene->addRect(rect);
        fill->setBrush(QBrush(Qt::darkGray));
        fill->setData(0, "cell_fill");
        fill->setZValue(-1);
    }
}

// 更新网格数据模型中指定单元格的类型
void MainWindow::updateGridData(int row, int col, GridCell::CellType type) {
    if (row >=0 && row < m_rows && col >=0 && col < m_cols) {
        QPoint currentPos(col, row);
        // 清除无效的起点
        if (currentPos == m_startPos && type != GridCell::Start) {
            updateStartPos(QPoint(-1, -1));
        }
        // 清除无效的终点
        if (currentPos == m_endPos && type != GridCell::End) {
            updateEndPos(QPoint(-1, -1));
        }
        m_gridData[row][col] = type;
    }
}

// 新建窗口函数实现，新建窗口菜单项触发，创建新的主窗口实例
void MainWindow::onNewWindowTriggered()
{
    // 创建新窗口（不设置父对象，独立存在）
    MainWindow *newWindow = new MainWindow(nullptr);

    // 设置关闭时自动释放内存
    newWindow->setAttribute(Qt::WA_DeleteOnClose);

    // 偏移显示位置（避免完全重叠）
    static int windowOffset = 20;
    newWindow->move(this->x() + windowOffset,
                    this->y() + windowOffset);
    windowOffset = (windowOffset + 30) % 100; // 循环偏移

    // 显示窗口
    newWindow->show();
}

// 打开文件菜单项触发，加载并显示文件内容
void MainWindow::onOpenFileTriggered(const QString &fileName)
{
    QString actualFileName = fileName;
    // 如果参数为空，弹出文件对话框
    if (actualFileName.isEmpty()) {
        QString initialDir = getLastFileDirectory();
        actualFileName = QFileDialog::getOpenFileName(
            this,
            "打开文件",
            initialDir,
            "C++文件 (*.cpp);;C文件 (*.c);;Java文件 (*.java);;Python文件 (*.py);;Json文件(*.json);;所有文件 (*)"
            );
    }
    // 如果文件名有效则加载文件
    if (!actualFileName.isEmpty()) {
        loadAndDisplayFile(actualFileName);
        updateLastFileDirectory(actualFileName);
    }
}

// 加载指定文件内容到编辑器并更新界面
void MainWindow::loadAndDisplayFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件");
        return;
    }

    // 读取并显示文件内容
    QTextStream in(&file);
    fileContentWidget->setPlainText(in.readAll());

    // 更新窗口标题
    setWindowTitle(QString("栅格地图编辑器 - %1").arg(QFileInfo(filePath).fileName()));

    // 根据内容长度调整分割比例
    int charCount = fileContentWidget->toPlainText().length();
    QList<int> sizes = { charCount > 500 ? 300 : 200, width() - 300 };
    mainSplitter->setSizes(sizes);

}

// 设置或更新起点位置并更新场景显示
void MainWindow::updateStartPos(QPoint pos)
{
    // 清除旧起点图标
    if (m_startIcon) {
        scene->removeItem(m_startIcon);
        delete m_startIcon;
        m_startIcon = nullptr;
    }

    // 清除旧起点单元格状态
    if (m_startPos != QPoint(-1, -1)) {
        GridCell* oldCell = getCell(m_startPos);
        if (oldCell) oldCell->setType(GridCell::Passable);
    }

    m_startPos = pos;
    if (pos.x() >=0 && pos.x() < m_cols && pos.y() >=0 && pos.y() < m_rows) {
        // 设置新起点单元格状态
        GridCell* cell = getCell(pos);
        if (cell) cell->setType(GridCell::Start);

        // 添加小车图标
        QPixmap carPixmap(":/icons/car.png"); // 使用资源路径
        const int iconSize = m_cellSize * 0.8; // 图标大小为单元格的80%
        carPixmap = carPixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio);
        m_startIcon = scene->addPixmap(carPixmap);
        // 计算中心点坐标
        qreal x = m_gridMargin + pos.x() * m_cellSize + (m_cellSize - iconSize)/2;
        qreal y = m_gridMargin + pos.y() * m_cellSize + (m_cellSize - iconSize)/2;
        m_startIcon->setPos(x, y);
        m_startIcon->setZValue(1); // 确保图标在单元格上方
    }
}

// 设置或更新终点位置并更新场景显示
void MainWindow::updateEndPos(QPoint pos)
{
    // 清除旧终点图标
    if (m_endIcon) {
        scene->removeItem(m_endIcon);
        delete m_endIcon;
        m_endIcon = nullptr;
    }

    // 清除旧终点单元格状态
    if (m_endPos != QPoint(-1, -1)) {
        GridCell* oldCell = getCell(m_endPos);
        if (oldCell) oldCell->setType(GridCell::Passable);
    }

    m_endPos = pos;
    if (pos.x() >=0 && pos.x() < m_cols && pos.y() >=0 && pos.y() < m_rows) {
        // 设置新终点单元格状态
        GridCell* cell = getCell(pos);
        if (cell) cell->setType(GridCell::End);

        // 添加旗帜图标
        QPixmap flagPixmap(":/icons/flag.png");
        const int iconSize = m_cellSize * 0.8;
        flagPixmap = flagPixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio);
        m_endIcon = scene->addPixmap(flagPixmap);
        qreal x = m_gridMargin + pos.x() * m_cellSize + (m_cellSize - iconSize)/2;
        qreal y = m_gridMargin + pos.y() * m_cellSize + (m_cellSize - iconSize)/2;
        m_endIcon->setPos(x, y);
        m_endIcon->setZValue(1);
    }
}

// 在指定位置设置障碍物单元格
void MainWindow::updateObstacle(QPoint pos) {
    if (pos.x() < 0 || pos.x() >= m_cols || pos.y() < 0 || pos.y() >= m_rows) return;

    GridCell* cell = getCell(pos);
    if (cell) {
        cell->setType(GridCell::Obstacle);
        m_gridData[pos.y()][pos.x()] = GridCell::Obstacle;
        updateCellVisual(pos.y(), pos.x());
    }
}

// 获取最近使用的文件目录路径
QString MainWindow::getLastFileDirectory() const
{
    QSettings settings;
    return settings.value("LastFileDirectory",
                          QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
        .toString();
}

// 更新最近使用的文件目录路径
void MainWindow::updateLastFileDirectory(const QString &path)
{
    if (!path.isEmpty()) {
        QFileInfo fileInfo(path);
        QSettings settings;
        settings.setValue("LastFileDirectory", fileInfo.absolutePath());
    }
}

// GridCell 构造函数
GridCell::GridCell(int row, int col, qreal size, MainWindow* mainWindow, QGraphicsItem* parent)
    : QGraphicsRectItem(0, 0, size, size, parent),
    m_row(row), m_col(col), m_mainWindow(mainWindow)
{
    setType(Passable);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    // 添加有效性检查
    Q_ASSERT_X(m_mainWindow, "GridCell", "MainWindow pointer cannot be null");
}

// 设置单元格类型并触发界面更新
void GridCell::setType(CellType type)
{
    if (m_type != type) {
        m_type = type;
        updateColor();
        // 通知主窗口更新数据
        if (m_mainWindow) {
            m_mainWindow->updateGridData(m_row, m_col, m_type);
        }
    }
}

// 根据当前类型更新单元格显示颜色
void GridCell::updateColor()
{
    switch (m_type) {
    case Passable:
        setBrush(Qt::white);
        break;
    case Obstacle:
        setBrush(Qt::darkGray);
        break;
    case Start:
        setBrush(Qt::green);
        break;
    case End:
        setBrush(Qt::red);
        break;
    }
}

// 右键菜单事件处理，提供单元格类型设置选项
void GridCell::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;

    // 获取当前单元格位置
    QPoint currentPos(col(), row());

    QAction* setStartAction = menu.addAction("设为起点");
    QAction* setEndAction = menu.addAction("设为终点");
    QAction* setObstacleAction = menu.addAction("设为障碍");
    QAction* clearAction = menu.addAction("清除");

    QAction* selectedAction = menu.exec(event->screenPos());

    if (selectedAction == setStartAction) {
        setType(Start);
        m_mainWindow->updateStartPos(QPoint(m_col, m_row));
    } else if (selectedAction == setEndAction) {
        setType(End);
        m_mainWindow->updateEndPos(QPoint(m_col, m_row));
    } else if (selectedAction == setObstacleAction) {
        setType(Obstacle);
    } else if (selectedAction == clearAction) {
        setType(Passable);
    }

    QGraphicsRectItem::contextMenuEvent(event);
}
