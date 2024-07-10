#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

// This class implements syntax highlighting for the text editor
class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    // Constructor: initializes the syntax highlighter
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    // This method is called automatically by Qt to highlight a block of text
    void highlightBlock(const QString &text) override;

private:
    // Structure to hold highlighting rules
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    // Text formats for different syntax elements
    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;

    // Method to set up the highlighting rules
    void setupHighlightingRules();
};

#endif // SYNTAXHIGHLIGHTER_H