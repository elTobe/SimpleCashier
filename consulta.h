#ifndef CONSULTA_H
#define CONSULTA_H

#include <QWidget>
#include <caja.h>

namespace Ui {
class Consulta;
}

class Consulta : public QWidget
{
    Q_OBJECT

public:
    explicit Consulta(QWidget *parent = nullptr);
    void showEvent( QShowEvent* event );
    ~Consulta();
    Caja* caja;

private slots:
    void on_input_returnPressed();

    void on_input_textChanged(const QString &arg1);

private:
    void keyPressEvent(QKeyEvent *event);

    Ui::Consulta *ui;
};

#endif // CONSULTA_H
