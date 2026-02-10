#ifndef CONTEXTPANNEL_H
#define CONTEXTPANNEL_H

#include <QHBoxLayout>
#include <QPushButton>
#include <QMenuBar>
#include <QLineEdit>
#include <QLabel>

class ContextPannel : public QWidget
{
    Q_OBJECT

public:
    ContextPannel(QWidget* parent = nullptr);
    const QHBoxLayout& getLayout() const;

private:
    QMenuBar* menu_bar;
    QHBoxLayout* context_pannel_layout;
};

#endif // CONTEXTPANNEL_H
