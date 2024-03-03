#include "caja.h"
#include "consulta.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Caja w;
    Consulta c;
    w.consulta = &c;
    c.caja = &w;
    w.show();
    return a.exec();
}
