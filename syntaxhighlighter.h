#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

struct HighlightRule {
    QRegularExpression pattern;  // 改用QRegularExpression
    QTextCharFormat format;
};

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void addRule(const QString &pattern, const QTextCharFormat &format);
    void handleMultiline(const QString &text,
                         const QRegularExpression &startPattern,
                         const QRegularExpression &endPattern,
                         const QTextCharFormat &format,
                         int stateFlag);

    QVector<HighlightRule> highlightingRules;

    // 格式定义
    QTextCharFormat keywordFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat includeFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;

    // 多行处理正则
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
    QRegularExpression stringStartExpression;
    QRegularExpression stringEndExpression; // 新增
};

#endif // SYNTAXHIGHLIGHTER_H
