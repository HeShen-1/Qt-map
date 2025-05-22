#ifndef MAPSIZEDIALOG_H
#define MAPSIZEDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

class MapSizeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MapSizeDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        // 创建控件
        QLabel *rowsLabel = new QLabel("行数:", this);
        rowsSpin = new QSpinBox(this);
        rowsSpin->setMinimum(1);
        rowsSpin->setMaximum(9999);
        rowsSpin->setValue(-1);

        QLabel *colsLabel = new QLabel("列数:", this);
        colsSpin = new QSpinBox(this);
        colsSpin->setMinimum(1);
        colsSpin->setMaximum(9999);
        colsSpin->setValue(-1);

        // 设置数值输入框宽度
        rowsSpin->setMinimumWidth(100);
        colsSpin->setMinimumWidth(100);

        QDialogButtonBox *buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

        // 布局
        QHBoxLayout *inputLayout = new QHBoxLayout();
        QVBoxLayout *rowsLayout = new QVBoxLayout();
        rowsLayout->addWidget(rowsLabel);
        rowsLayout->addWidget(rowsSpin);

        QVBoxLayout *colsLayout = new QVBoxLayout();
        colsLayout->addWidget(colsLabel);
        colsLayout->addWidget(colsSpin);

        inputLayout->addLayout(rowsLayout);
        inputLayout->addLayout(colsLayout);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addLayout(inputLayout);
        mainLayout->addWidget(buttons);

        // 信号连接
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    int getRows() const { return rowsSpin->value(); }
    int getCols() const { return colsSpin->value(); }

private:
    QSpinBox *rowsSpin;
    QSpinBox *colsSpin;
};


#endif // MAPSIZEDIALOG_H
