#include "consulta.h"
#include "ui_consulta.h"
#include <QKeyEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QFile>


Consulta::Consulta(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Consulta)
{
    ui->setupUi(this);

    ui->etiqueta_precio2->hide();
    ui->etiqueta_precio2_signo->hide();
    ui->etiqueta_precio2_texto->hide();
}

Consulta::~Consulta()
{
    delete ui;
}

void Consulta::showEvent( QShowEvent* event ) {
    QWidget::showEvent( event );
    this->ui->input->setFocus();
    ui->input->selectAll();
}

void Consulta::keyPressEvent(QKeyEvent *event){
    int tecla = event->key();

    if(tecla == Qt::Key_F12 || tecla == Qt::Key_Escape){
        this->hide();
    }

    ui->input->setFocus();
}

void Consulta::on_input_returnPressed()
{
    ui->input->selectAll();

    /////// -------------- INSERTAR -------------- ///////
    if(ui->input->text().length() <= 2){
        bool ok;
        int n = ui->input->text().toInt(&ok,10);
        if(ui->etiqueta_precio1->text() != "--.--" && ok && n>0 && n<this->caja->departamentos.size()){
            this->caja->resultado_consulta = ui->etiqueta_precio1->text().toFloat();
            this->caja->departamento_consulta = n - 1;
            this->caja->insertar();
            this->hide();
        }
    /////// -------------- BUSCAR -------------- ///////
    }else{
        //CONEXION POR DEFAULT
        QString ip = "192.168.0.105";
        QString port = "3306";
        QString base = "sicar";
        QString username = "consultas";
        QString password = "123456";

        QFile file("ip_server.txt");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&file);
            ip = in.readLine();
            port = in.readLine();
            base = in.readLine();
            username = in.readLine();
            password = in.readLine();
            file.close();
        }

        QSqlDatabase sicardb = QSqlDatabase::addDatabase("QMYSQL");
        sicardb.setHostName(ip);
        sicardb.setPort(port.toInt());
        sicardb.setDatabaseName(base);
        sicardb.setUserName(username);
        sicardb.setPassword(password);

        if(!sicardb.open()){
            QMessageBox messageBox;
            messageBox.critical(0,"Error","No se pudo conectar a la base de datos !");
            messageBox.setFixedSize(500,200);
            return;
        }
        QSqlQuery consulta;

        bool forzar_precio = false;
        float precio_forzado = 0;
        QString busqueda = ui->input->text().toUpper();
        if( busqueda.contains("/") ){
            forzar_precio = true;
            precio_forzado = busqueda.left( busqueda.indexOf("/") ).toFloat();
            busqueda = busqueda.right( busqueda.length() - busqueda.indexOf("/")-1 );
        }
        if( busqueda.contains("-") ){
            forzar_precio = true;
            precio_forzado = busqueda.left( busqueda.indexOf("-") ).toFloat();
            busqueda = busqueda.right( busqueda.length() - busqueda.indexOf("-")-1 );
        }
        qDebug() << "precioForzado = " << precio_forzado;
        qDebug() << "query = " << busqueda;

        /////// -------------- POR CLAVE -------------- ///////
        consulta.exec("SELECT descripcion,precio1,precio2 FROM articulo WHERE (clave='" + busqueda + "' OR claveAlterna='" + busqueda + "') AND status=1");
        if( consulta.next() ){
            ui->etiqueta_imagen->setPixmap(QPixmap(":/images/ok.png"));

            QString temp;
            ui->etiqueta_nombre->setText( consulta.value("descripcion").toString() );

            if(forzar_precio){
                temp.setNum( precio_forzado, 'f', 2 );
                ui->etiqueta_precio1->setText(temp);
            }else{
                temp.setNum( consulta.value("precio1").toFloat(), 'f', 2 );
                ui->etiqueta_precio1->setText(temp);
            }

            if( consulta.value("precio2").toFloat() > 0 ){
                temp.setNum( consulta.value("precio2").toFloat(), 'f', 2 );
                ui->etiqueta_precio2->setText(temp);
                ui->etiqueta_precio2->show();
                ui->etiqueta_precio2_signo->show();
                ui->etiqueta_precio2_texto->show();
            }else{
                ui->etiqueta_precio2->hide();
                ui->etiqueta_precio2_signo->hide();
                ui->etiqueta_precio2_texto->hide();
            }
            return;

        }

        /////// -------------- LIKE CLAVE -------------- ///////
        consulta.exec("SELECT descripcion,precio1,precio2 FROM articulo WHERE clave LIKE '%" + busqueda + "' AND status=1");
        if( consulta.next() ) {
            ui->etiqueta_imagen->setPixmap(QPixmap(":/images/ok.png"));
            QString temp;
            ui->etiqueta_nombre->setText( consulta.value("descripcion").toString() );

            if(forzar_precio){
                temp.setNum( precio_forzado, 'f', 2 );
                ui->etiqueta_precio1->setText(temp);
            }else{
                temp.setNum( consulta.value("precio1").toFloat(), 'f', 2 );
                ui->etiqueta_precio1->setText(temp);
            }

            if( consulta.value("precio2").toFloat() > 0 ){
                temp.setNum( consulta.value("precio2").toFloat(), 'f', 2 );
                ui->etiqueta_precio2->setText(temp);
                ui->etiqueta_precio2->show();
                ui->etiqueta_precio2_signo->show();
                ui->etiqueta_precio2_texto->show();
            }else{
                ui->etiqueta_precio2->hide();
                ui->etiqueta_precio2_signo->hide();
                ui->etiqueta_precio2_texto->hide();
            }
            return;
        }

        /////// -------------- LIKE CLAVE ALTERNA -------------- ///////
        consulta.exec("SELECT descripcion,precio1,precio2 FROM articulo WHERE claveAlterna LIKE '%" + busqueda + "' AND status=1");
        if( consulta.next() ) {
            ui->etiqueta_imagen->setPixmap(QPixmap(":/images/ok.png"));
            QString temp;
            ui->etiqueta_nombre->setText( consulta.value("descripcion").toString() );

            if(forzar_precio){
                temp.setNum( precio_forzado, 'f', 2 );
                ui->etiqueta_precio1->setText(temp);
            }else{
                temp.setNum( consulta.value("precio1").toFloat(), 'f', 2 );
                ui->etiqueta_precio1->setText(temp);
            }

            if( consulta.value("precio2").toFloat() > 0 ){
                temp.setNum( consulta.value("precio2").toFloat(), 'f', 2 );
                ui->etiqueta_precio2->setText(temp);
                ui->etiqueta_precio2->show();
                ui->etiqueta_precio2_signo->show();
                ui->etiqueta_precio2_texto->show();
            }else{
                ui->etiqueta_precio2->hide();
                ui->etiqueta_precio2_signo->hide();
                ui->etiqueta_precio2_texto->hide();
            }
            return;
        }

        /////// -------------- ERROR -------------- ///////
        ui->etiqueta_nombre->setText("NO ENCONTRADO");
        ui->etiqueta_precio1->setText("--.--");
        ui->etiqueta_precio2->setText("--.--");
        ui->etiqueta_imagen->setPixmap(QPixmap(":/images/error.png"));
    }
}


void Consulta::on_input_textChanged(const QString &arg1)
{
    QString t = ui->input->text();

    if(t.length()>0){
        if( t.last(1) == "."){
            t.removeLast();
            ui->input->setText( t + "0" );
        }
    }
}

