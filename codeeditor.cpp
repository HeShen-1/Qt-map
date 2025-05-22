// codeeditor.cpp
#include "codeeditor.h"
#include "syntaxhighlighter.h"

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {
    lineNumberArea = new LineNumberArea(this);
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged,this, &CodeEditor::highlightCurrentLine);
    updateLineNumberAreaWidth(0);

    // 初始化语法高亮
    new SyntaxHighlighter(document());
}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    return 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::updateLineNumberAreaWidth(int) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
}

void CodeEditor::lineNumberPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    // 绘制行号文本
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                            Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

// 当前行高亮实现
void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        // 设置高亮样式
        QColor lineColor = QColor(255, 255, 196);  // 浅黄色背景
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);

        // 获取当前光标位置
        QTextCursor cursor = textCursor();
        cursor.clearSelection();
        selection.cursor = cursor;

        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

// 在CodeEditor类中添加错误提示
void CodeEditor::markError(int line) {
    QTextBlock block = document()->findBlockByLineNumber(line-1);
    QTextCursor cursor(block);

    QTextCharFormat errorFormat;
    errorFormat.setUnderlineColor(Qt::red);
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);

    QList<QTextEdit::ExtraSelection> extras = extraSelections();
    QTextEdit::ExtraSelection selection;
    selection.cursor = cursor;
    selection.format = errorFormat;
    extras.append(selection);

    setExtraSelections(extras);
}
