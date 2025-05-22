
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QScroller>      // 惯性滚动支持
#include <QScrollBar>      // 滚动条操作
#include <QSplitter>
#include <QPlainTextEdit>

#include "codeeditor.h"
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>


class QGraphicsLineItem;  // 前向声明
class CodeEditor;  // 前置声明
class MainWindow; // 前向声明

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class GridCell : public QGraphicsRectItem {
public:
    enum CellType { Passable, Obstacle, Start, End };

    GridCell(int row, int col, qreal size, MainWindow* mainWindow, QGraphicsItem* parent = nullptr);

    void setType(CellType type);
    int row() const { return m_row; }
    int col() const { return m_col; }

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override; // 仅声明

private:
    void updateColor();
    int m_row;
    int m_col;
    CellType m_type = Passable;
    MainWindow* m_mainWindow; // 添加MainWindow指针
};


class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class GridCell; // 允许GridCell访问私有成员

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:  // 关键slot声明
    void onSaveFileTriggered();  // 添加这一行
    void onOpenFileTriggered(const QString &fileName = QString());  // 打开文件函数
    void onNewMapTriggered();  // 新建地图槽函数
    void onNewWindowTriggered(); // 新建窗口槽函数
    void updateGridData(int row, int col, GridCell::CellType type);

    // 读入地图
    void onLoadMapTriggered();
    // 从json文件加载地图
    void loadMapFromFile(const QString &filePath);

private:
    Ui::MainWindow *ui;

    void createMenus();     // 创建菜单栏
    void createGrid(int rows, int cols, int cellSize = 0);  // 创建网格函数
    void loadAndDisplayFile(const QString &filePath);  // 打开文件函数

    QGraphicsScene *scene;  // 添加场景成员
    QGraphicsView *view;    // 添加视图成员

    QSplitter *mainSplitter;
    CodeEditor *fileContentWidget;

    void saveMapToFile(const QString &filePath);  // 添加这一行
    void updateCellVisual(int row, int col);      // 添加这一行

    // 更新起点、终点、障碍的坐标
    void updateStartPos(QPoint pos);
    void updateEndPos(QPoint pos);
    void updateObstacle(QPoint pos);

    int m_rows = 0;
    int m_cols = 0;
    int m_cellSize = 40;
    QVector<QVector<GridCell::CellType>> m_gridData; // 0-普通 1-障碍 2-起点 3-终点
    QPoint m_startPos;
    QPoint m_endPos;

    // 插入小车和旗帜的图标
    QGraphicsPixmapItem* m_startIcon = nullptr;
    QGraphicsPixmapItem* m_endIcon = nullptr;
    const int m_gridMargin = 10; // 与createGrid中的margin一致

    // 实现栅格属性设置的快捷键
    QPoint m_lastMousePos;      // 记录鼠标位置

private:
    enum DetailLevel { LowDetail, NormalDetail, HighDetail };

    void updateGridDetailLevel(DetailLevel level) {
        foreach (QGraphicsItem *item, scene->items()) {
            if (QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(item)) {
                QPen pen = line->pen();
                pen.setWidthF(level == HighDetail ? 1.5 : 0.8);
                line->setPen(pen);
            }
        }
    }

private:
    GridCell* getCell(const QPoint& pos) {
        int row = pos.y();
        int col = pos.x();
        if (row >=0 && row < m_rows && col >=0 && col < m_cols) {
            foreach (QGraphicsItem* item, scene->items()) {
                if (GridCell* cell = dynamic_cast<GridCell*>(item)) {
                    if (cell->row() == row && cell->col() == col)
                        return cell;
                }
            }
        }
        return nullptr;
    }

// 文件打开、保存记忆化
private:
    QString getLastFileDirectory() const;
    void updateLastFileDirectory(const QString &path);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override
    {
        // 根据缩放级别调整细节
        qreal scaleFactor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
        view->scale(scaleFactor, scaleFactor);

        // 缩放超过200%时显示精细网格
        if (view->transform().m11() > 2.0) {
            updateGridDetailLevel(HighDetail);
        }
        // 缩放小于50%时简化显示
        else if (view->transform().m11() < 0.5) {
            updateGridDetailLevel(LowDetail);
        }
        // 按住Ctrl时使用滚轮缩放
        if (event->modifiers() & Qt::ControlModifier) {
            const double scaleFactor = 1.1;
            if (event->angleDelta().y() > 0) {
                view->scale(scaleFactor, scaleFactor);
            } else {
                view->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
            }
            event->accept();
        } else {
            QMainWindow::wheelEvent(event);
        }
    }

    // 添加鼠标移动事件重写声明
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // MAINWINDOW_H
