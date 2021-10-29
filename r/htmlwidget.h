#ifndef HTMLWIDGET_H
#define HTMLWIDGET_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QTextBrowser>

class htmlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit htmlWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setLayout(&background_layout);
        background_layout.addWidget(&browser);

        setFixedWidth( 480 );
        setFixedHeight( 640 );
        showMaximized();
    }

private:
    QTextBrowser browser;
    QGridLayout background_layout;

signals:

};

#endif // HTMLWIDGET_H
