// syntaxhighlighter.cpp
#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // ================== 1. VS Code风格颜色配置 ==================
    // 关键字（柔蓝色）
    keywordFormat.setForeground(QColor(86, 156, 214));
    keywordFormat.setFontWeight(QFont::Bold);

    // 函数名
    functionFormat.setForeground(QColor(60, 19, 227));

    // 头文件（紫色）
    includeFormat.setForeground(QColor(184, 134, 219));
    includeFormat.setFontItalic(true);

    // 数字
    numberFormat.setForeground(QColor(225, 140, 0));

    // 类型（青蓝色，仅匹配带命名空间）
    typeFormat.setForeground(QColor(78, 201, 176));

    // 预处理指令（灰色）
    preprocessorFormat.setForeground(QColor(155, 155, 155));

    // 注释（绿色）
    singleLineCommentFormat.setForeground(QColor(87, 166, 74));
    multiLineCommentFormat.setForeground(QColor(87, 166, 74));

    // 字符串（棕橙色）
    quotationFormat.setForeground(QColor(206, 145, 120));

    // ================== 2. 调整规则顺序 ==================
    // 预处理指令（最先处理）
    addRule(R"(^\s*#\s*\w+)", preprocessorFormat);
    addRule(R"(#\s*include\s*(<[^>]+>|\"[^\"]+\"))", includeFormat);

    // 类型（带命名空间的类型，在关键字前添加）
    addRule(R"(\b(std::|boost::|qt::)[A-Za-z_]\w*(<.*>)?\b)", typeFormat);

    // 关键字（具体规则后添加以覆盖类型）
    const QStringList keywordPatterns = {/* 原有关键字列表 */};
    for (const auto& pattern : keywordPatterns) {
        addRule(pattern, keywordFormat);
    }

    // 函数名（在关键字之后）
    addRule(R"(\b(?!(?:if|while|for|switch|return|catch)\b)[A-Za-z_]\w*(?=\s*\())", functionFormat);

    // 数字（在函数名之后）
    addRule(R"(\b\d+\.?\d*([eE][+-]?\d+)?\b)", numberFormat);
    addRule(R"(\b0x[\dA-Fa-f]+\b)", numberFormat);
    addRule(R"(\b0b[01]+\b)", numberFormat);

    // 注释和字符串（最后处理）
    addRule(R"(//[^\n]*)", singleLineCommentFormat);
    addRule(R"("(\\.|[^"\\])*")", quotationFormat);
    addRule(R"('(\\.|[^'\\])')", quotationFormat);

    // 多行注释界定符
    commentStartExpression = QRegularExpression(R"(/\*)");
    commentEndExpression = QRegularExpression(R"(\*/)");

    // 多行字符串界定符（原始字符串）
    stringStartExpression = QRegularExpression(R"RX(R"([^()]*)RX");
    stringEndExpression = QRegularExpression(R"RX(\s*\))RX");
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // 处理多行字符串（原始字符串）
    // handleMultiline(text, stringStartExpression, QRegularExpression(R"(")"), quotationFormat, 2);
    // 修正处理逻辑，携带定界符信息
    handleMultiline(text,
                    stringStartExpression,
                    stringEndExpression,  // 使用新的结束表达式
                    quotationFormat,
                    2);  // 状态标识

    // 处理多行注释
    handleMultiline(text, commentStartExpression, commentEndExpression, multiLineCommentFormat, 1);

    // 应用其他高亮规则
    for (const auto& rule : highlightingRules) {
        auto matches = rule.pattern.globalMatch(text);
        while (matches.hasNext()) {
            auto match = matches.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

void SyntaxHighlighter::addRule(const QString &pattern, const QTextCharFormat &format) {
    HighlightRule rule;
    rule.pattern = QRegularExpression(pattern);
    rule.format = format;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::handleMultiline(const QString &text,
                                        const QRegularExpression &startPattern,
                                        const QRegularExpression &endPattern,
                                        const QTextCharFormat &format,
                                        int stateFlag)
{
    int startIndex = 0;
    int add = 0;

    // 如果上一行是未闭合状态
    if (previousBlockState() == stateFlag) {
        startIndex = 0;
        add = 1;
    } else {
        startIndex = text.indexOf(startPattern);
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch;
        int endIndex = text.indexOf(endPattern, startIndex + add, &endMatch);
        int length;

        if (endIndex == -1) { // 未找到结束符
            setCurrentBlockState(stateFlag);
            length = text.length() - startIndex;
        } else { // 找到结束符
            length = endIndex - startIndex + endMatch.capturedLength();
        }

        setFormat(startIndex, length, format);
        startIndex = text.indexOf(startPattern, startIndex + length);
        add = 0;
    }
}
