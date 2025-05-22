#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QTextBlock>
#include <QPainter>

class LineNumberArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    int lineNumberAreaWidth();
    // 添加行号绘制函数声明
    void lineNumberPaintEvent(QPaintEvent *event);
    void markError(int line);

    class LineNumberArea : public QWidget {
    public:
        LineNumberArea(CodeEditor *editor) : QWidget(editor), editor(editor) {}
        QSize sizeHint() const override {
            return QSize(editor->lineNumberAreaWidth(), 0);
        }

    protected:
        void paintEvent(QPaintEvent *event) override {
            editor->lineNumberPaintEvent(event);
        }

    private:
        CodeEditor *editor;
    };

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();

private:
    QWidget *lineNumberArea;
    friend class LineNumberArea; // 声明友元类
    void updateLineHighlight();  // 更新高亮
};

#endif // CODEEDITOR_H
