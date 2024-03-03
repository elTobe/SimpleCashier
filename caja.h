#ifndef CAJA_H
#define CAJA_H

#include <QMainWindow>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class Caja; }
QT_END_NAMESPACE

class Caja : public QMainWindow
{
    Q_OBJECT

public:
    Caja(QWidget *parent = nullptr);
    ~Caja();
    QWidget* consulta;
    float resultado_consulta = 0.0;
    int departamento_consulta = 0;
    QVector<QString> departamentos;
    void insertar();

private slots:
    void on_reimpresion_clicked();

    void keyPressEvent(QKeyEvent *event);

    void on_boton_borrar_todo_clicked();

    void on_boton_corte_x_clicked();

    void on_boton_corte_z_clicked();

    void on_check_tickets_clicked();

    void on_pushButton_clicked();

    void cobrar();

    int aumentar_folio(bool aumentar);  ///false para resetear, regresa el folio al que se aumentó

    int get_folio();

private:
    bool count_zeros = false;
    int state = 1;
    int position_counter = 1;
    int multiplicador = 1;
    int tecla_anterior = -1;
    bool cambio = false;
    bool redo = false;
    int num_articulos = 0;

    QListWidgetItem titulo;
    Ui::Caja *ui;
};
#endif // CAJA_H
