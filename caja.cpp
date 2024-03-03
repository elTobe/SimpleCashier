#include "caja.h"
#include "ui_caja.h"
#include <QtPrintSupport>
#include <QPrinterInfo>
#include <QPrinter>
#include <QImage>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFile>

Caja::Caja(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Caja)
{
    ui->setupUi(this);
    this->setEnabled(true);
    this->setFocus();

    ui->listWidget->setFont(QFont("Lucida Console",16,QFont::Normal,false));

    QFile file("departamentos.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox messageBox;
        messageBox.critical(0,"Error","No se ha encontrado el archivo de departamentos !");
        messageBox.setFixedSize(500,200);
    }

    QTextStream in(&file);
    QString line;
    while (!in.atEnd()) {
        line = in.readLine();
        this->departamentos.append(line);
    }
    this->departamentos.append(line);
    file.close();

    int folio = get_folio();
    ui->folio_venta->setText(QVariant(folio+1).toString());
}

int Caja::aumentar_folio(bool aumentar){

    QFile file("folio.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return -1;
    }
    QTextStream in(&file);
    int folio = QVariant(in.readLine()).toInt();
    file.close();

    QFile file2("folio.txt");
    if (!file2.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
        return -1;
    }
    QTextStream out(&file2);

    if(aumentar){
        folio++;
    }else{
        folio = 0;
    }

    ui->folio_venta->setText(QVariant(folio+1).toString());
    out << folio;
    file2.close();
    return folio;
}

int Caja::get_folio(){

    QFile file("folio.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return -1;
    }
    QTextStream in(&file);
    int folio = QVariant(in.readLine()).toInt();

    file.close();
    return folio;
}

Caja::~Caja()
{
    delete ui;
}

void Caja::insertar(){
        QString num;
        num.setNum(this->resultado_consulta,'f',2);
        this->ui->listWidget->addItem(this->departamentos.at(this->departamento_consulta).leftJustified(13,' ',true)
                                            + "| $" + num.rightJustified(11,' ',true) );
        num.setNum(ui->etiqueta_total->text().toFloat() + this->resultado_consulta,'f',2);
        this->num_articulos++;
        ui->cantidad_art->setText(QVariant(num_articulos).toString());

        ui->listWidget->scrollToBottom();

        ui->etiqueta_total->setText(num);
}

void Caja::cobrar(){
    ///imprimir venta actual

    int folio = this->aumentar_folio(true);

    QFile file("ultima_venta.txt");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox messageBox;
        messageBox.critical(0,"Error","No se pudo escribir el archivo de nueva venta !");
        messageBox.setFixedSize(500,200);
    }
    QTextStream out(&file);

    QPrinterInfo impresora_info = QPrinterInfo::defaultPrinter();
    QPrinter impresora = QPrinter(impresora_info,
                                  QPrinter::PrinterMode::ScreenResolution);
    int ancho= impresora_info.defaultPageSize().rectPixels(impresora.resolution()).width();
    int borde = 15;

    QFontDatabase::addApplicationFont(":/fonts/LucidaTypewriterRegular.ttf");
    QFont fuente = QFont("Lucida Sans Typewriter",10,QFont::Normal,false);
    fuente.setStretch(QFont::Condensed);
    QFontMetrics fuente_metricas = QFontMetrics(fuente);
    int alto_letra = fuente_metricas.height();
    int vspace = 1;
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime =  "Fecha :        " + date.toString("dd/MM/yyyy       hh:mm:ss");
    QString numstring;
    if(this->num_articulos == 1){
        numstring = QVariant(num_articulos).toString() + " artículo";
    }else{
        numstring = QVariant(num_articulos).toString() + " artículos";
    }
    numstring = numstring.rightJustified(20,' ',true);
    QString foliostring = "Folio : " + QVariant(folio).toString();
    foliostring = foliostring.leftJustified(20,' ',true);
    QString num_y_folio = foliostring + numstring;
    int fila = 3;
    QPainter documento;

    if(ui->check_tickets->isChecked()){
        QImage logo = QImage(":/images/small_elegant_logo.png");
        QPoint punto = QPoint( (ancho-logo.width())/2 ,0);

        documento.begin(&impresora);
        documento.setFont(fuente);
        documento.drawImage(punto,logo);
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "El Sauz Alto, Pedro Escobedo, Querétaro."); fila++;
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "                                        "); fila++;
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, num_y_folio); fila++;
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, formattedTime); fila++;
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "========================================"); fila++;
    }

    if(file.isOpen()){
        out << "El Sauz Alto, Pedro Escobedo, Querétaro." << "\n";
        out << "                                        " << "\n";
        out << num_y_folio << "\n";
        out << formattedTime << "\n";
        out << "========================================" << "\n";
    }

    ///Articulos Aqui
    int lim = ui->listWidget->count();
    QString s;
    for(int i = 0 ; i < lim; i++){
        s = "   " + ui->listWidget->takeItem(0)->text();
        if(s.contains("(")){
            s.replace("("," ");
            s.replace(")"," ");
            s.replace(16,1,"   ");
            s = s + "        ";
        }else{
            s.replace(16,1,"       ");
            s = s + "    ";
        }

        if(ui->check_tickets->isChecked()){documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, s); fila++;}
        if(file.isOpen()){out << s << "\n";}
    }

    QString total = QString("TOTAL : $" + ui->etiqueta_total->text().rightJustified(10,' ',false) + "   ").rightJustified(25,' ',true);
    QString pago = QString("Pago con : $" + ui->etiqueta_pago->text().rightJustified(10,' ',false) + "   ").rightJustified(40,' ',true);
    QString cambio = QString("Cambio : $" + ui->etiqueta_cambio->text().rightJustified(10,' ',false) + "   ").rightJustified(40,' ',true);

    if(ui->check_tickets->isChecked()){
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "========================================"); fila++;
        fuente.setWeight(QFont::ExtraBold);
        fuente.setPointSize(15);
        documento.setFont(fuente);
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde+10*vspace, total ); fila++;
        fila++;
        fila++;
        fuente.setWeight(QFont::Normal);
        fuente.setPointSize(10);
        documento.setFont(fuente);
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, pago ); fila++;
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, cambio); fila++;
        fila++;
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "        ¡Gracias por visitarnos!        "); fila++;
            documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "         ¡Que tenga un buen dia!        "); fila++;
            documento.end();
    }

    if(file.isOpen()){
        out << "========================================" << "\n";
        out << total << "\n";
        out << "                                        " << "\n";
        out << "                                        " << "\n";
        out << pago << "\n";
        out << cambio << "\n";
        out << "                                        " << "\n";
        out << "        ¡Gracias por visitarnos!        " << "\n";
            out << "         ¡Que tenga un buen dia!        " << "\n";
               file.close();

        QString filename =  "venta" + date.toString("_dd_MM_yyyy_hh_mm_ss") + ".txt";
        QFile::copy("ultima_venta.txt","ventas/"+filename);
    }
    this->num_articulos = 0;
    ui->cantidad_art->setText(QVariant(num_articulos).toString());
}

void Caja::keyPressEvent(QKeyEvent *event){
    int tecla = event->key();
    //qDebug() << this->num_articulos << "\n";

    do{
        if(tecla == Qt::Key_Period){
            tecla = Qt::Key_0;
        }else if(tecla == Qt::Key_0 && !redo){
            this->redo = true;
            tecla = Qt::Key_0;
        }else if(this->redo){
            this->redo = false;
        }

        if(this->cambio){
            ui->etiqueta_precio->setStyleSheet("border : 5px solid black");
            ui->etiqueta_cambio->setStyleSheet("border : 0px solid green");
            ui->etiqueta_dep->setText("--");
            ui->etiqueta_multi->setText("");
            ui->etiqueta_precio->setText("     0.00");
            ui->etiqueta_total->setText("       0.00");
            ui->etiqueta_pago->setText("       -.--");
            ui->etiqueta_cambio->setText("       -.--");
            this->cambio = false;
        }

        if( tecla > 47 && tecla < 58){

            if(tecla != 48){
                this->count_zeros = true;
            }
            /// ------------------------ NUMEROS ---------------------------
            if( (tecla == 48 && count_zeros == true) || tecla != 48 ){
                QString text;
                switch(this->state){
                case 1 :
                    text = ui->etiqueta_precio->text();
                    if(this->position_counter < 9){
                        text.remove(text.length()-3,1);
                        if(this->position_counter < 4){
                            text.remove(text.indexOf('0'),1);
                        }else{
                            text.removeFirst();
                        }
                        text.append(char(tecla));
                        text.insert(text.length()-2,'.');
                    }
                    ui->etiqueta_precio->setText(text);
                    break;
                case 2 :
                    text = ui->etiqueta_dep->text();
                    if(this->position_counter < 3){
                        if(this->position_counter == 1){
                            text.remove(0,2);
                            text.append(' ');
                            text.append(char(tecla));
                        }else{
                            text.removeFirst();
                            text.append(char(tecla));
                        }
                    }
                    ui->etiqueta_dep->setText(text);
                    break;
                case 3:
                    text = ui->etiqueta_pago->text();
                    if(this->position_counter < 11){
                        text.remove(text.length()-3,1);
                        if(this->position_counter < 4){
                            text.remove(text.indexOf('-'),1);
                        }else{
                            text.removeFirst();
                        }
                        text.append(char(tecla));
                        text.insert(text.length()-2,'.');
                    }
                    ui->etiqueta_pago->setText(text);
                    break;
                case 4 :
                    break;
                default:
                    break;
                }
                this->position_counter++;
            }
        }else if(tecla == Qt::Key_Asterisk){
            /// ------------------------ MULTIPLICACION --------------------------

            int m;
            switch(this->state){
            case 1:
                m = ui->etiqueta_precio->text().remove(".",Qt::CaseInsensitive).toInt();
                if(m <= 99 && m >=1 ){
                    this->multiplicador = m;
                    ui->etiqueta_precio->setText("     0.00");
                    this->position_counter = 1;
                    ui->etiqueta_multi->setText(QVariant(multiplicador).toString() + " x");
                }
                break;
            default:
                break;
            }

        }else if(tecla == Qt::Key_Return || tecla == Qt::Key_Enter){
            /// ------------------------ ENTER ---------------------------

            switch(this->state){
            case 1:
                if( ui->etiqueta_precio->text().toFloat() > 0){
                    this->state++;
                    ui->etiqueta_precio->setStyleSheet("border : 0px solid black");
                    ui->etiqueta_dep->setStyleSheet("border : 5px solid black");
                    this->position_counter = 1;
                }else{
                    ui->etiqueta_precio->setStyleSheet("border : 5px solid red");
                    this->position_counter = 1;
                }
                break;
            case 2:
                if( ui->etiqueta_dep->text().toInt() < departamentos.size()
                    && ui->etiqueta_dep->text().toInt() > 0){

                    QString num;
                    if(multiplicador != 1){
                        ui->listWidget->addItem(this->departamentos.at(ui->etiqueta_dep->text().toInt() - 1).leftJustified(13,' ',true)
                                                + "|($" + ui->etiqueta_precio->text().rightJustified(10,' ',true) + ")" );
                        ui->listWidget->addItem("   x " + QVariant(this->multiplicador).toString().leftJustified(8,' ',true)
                                                + "| $" + num.setNum(ui->etiqueta_precio->text().toFloat() * this->multiplicador,'f',2).rightJustified(11,' ',true));
                        num.setNum(ui->etiqueta_total->text().toFloat() + ui->etiqueta_precio->text().toFloat() * this->multiplicador,'f',2);
                        this->num_articulos = this->num_articulos + multiplicador;
                        ui->etiqueta_total->setText(num);
                        this->multiplicador=1;
                    }else{
                        ui->listWidget->addItem(this->departamentos.at(ui->etiqueta_dep->text().toInt() - 1).leftJustified(13,' ',true)
                                                + "| $" + ui->etiqueta_precio->text().rightJustified(11,' ',true) );
                        num.setNum(ui->etiqueta_total->text().toFloat() + ui->etiqueta_precio->text().toFloat(),'f',2);
                        ui->etiqueta_total->setText(num);
                        this->num_articulos++;
                    }

                    ui->cantidad_art->setText(QVariant(num_articulos).toString());

                    ui->listWidget->scrollToBottom();

                    this->state--;
                    ui->etiqueta_precio->setStyleSheet("border : 5px solid black");
                    ui->etiqueta_dep->setStyleSheet("border : 0px solid black");
                    ui->etiqueta_precio->setText("     0.00");
                    ui->etiqueta_dep->setText("--");
                    ui->etiqueta_multi->setText("");
                    this->position_counter = 1;

                    this->count_zeros = false;
                }else{
                    ui->etiqueta_dep->setStyleSheet("border : 5px solid red");
                    ui->etiqueta_dep->setText("--");
                    this->position_counter = 1;
                    this->multiplicador = 1;
                }

                break;
            case 3:
                if( ui->etiqueta_total->text().toFloat() <= ui->etiqueta_pago->text().toFloat() && ui->etiqueta_total->text().toFloat() != 0 ){
                    ui->etiqueta_pago->setStyleSheet("border : 0px solid black");
                    QString num;
                    num.setNum(ui->etiqueta_pago->text().toFloat()-ui->etiqueta_total->text().toFloat(),'f',2);
                    ui->etiqueta_cambio->setText(num);
                    ui->etiqueta_pago->setStyleSheet("border : 0px solid black");
                    ui->etiqueta_cambio->setStyleSheet("border : 5px solid green");
                    ui->etiqueta_precio->setStyleSheet("border : 5px solid black");
                    this->cambio = true;
                    this->state = 1;
                    this->position_counter = 1;
                    this->multiplicador = 1;
                    this->cobrar();

                }else{
                    ui->etiqueta_pago->setStyleSheet("border : 5px solid red");
                    this->position_counter = 1;
                    ui->etiqueta_pago->setText("       -.--");
                    tecla = -1;
                }
                break;
            default:
                break;
            }

        }else if(tecla == Qt::Key_Backspace){
            /// ------------------------ SUPRIMIR ---------------------------
            if(tecla_anterior == Qt::Key_Backspace){
                tecla = -1;
                switch(this->state){
                case 1:
                    this->multiplicador = 1;
                    ui->etiqueta_multi->setText("");
                    ui->etiqueta_precio->setText("     0.00");
                    break;
                case 2:
                    this->state--;
                    ui->etiqueta_precio->setText("     0.00");
                    ui->etiqueta_dep->setStyleSheet("border : 0px solid black");
                    ui->etiqueta_precio->setStyleSheet("border : 5px solid black");
                    ui->etiqueta_dep->setText("--");
                    break;
                case 3:
                    this->state=1;
                    ui->etiqueta_pago->setStyleSheet("border : 0px solid black");
                    ui->etiqueta_precio->setStyleSheet("border : 5px solid black");
                    ui->etiqueta_pago->setText("     -.--");
                    ui->etiqueta_precio->setText("     0.00");
                    break;
                }
            }else{
                switch(this->state){
                case 1:
                    ui->etiqueta_precio->setText("     0.00");
                    break;
                case 2:
                    ui->etiqueta_dep->setText("--");
                    break;
                case 3:
                    ui->etiqueta_pago->setText("     -.--");
                    break;
                default:
                    break;
                }
                this->position_counter = 1;
            }
            this->count_zeros = false;

        }else if(tecla == Qt::Key_Space){
            /// ------------------------ FIN ---------------------------

            switch (this->state) {
            case 1:
                this->state = 3;
                ui->etiqueta_precio->setStyleSheet("border : 0px solid black");
                ui->etiqueta_pago->setStyleSheet("border : 5px solid black");
                this->position_counter = 1;
                tecla = -1;
                break;
            case 2:
                this->state = 3;
                ui->etiqueta_dep->setStyleSheet("border : 0px solid black");
                ui->etiqueta_pago->setStyleSheet("border : 5px solid black");
                this->position_counter = 1;
                tecla = -1;
                break;
            case 3:
                if(ui->etiqueta_pago->text().trimmed() == "-.--" && tecla_anterior == Qt::Key_Space && ui->etiqueta_total->text().toFloat() != 0 ){
                    ui->etiqueta_pago->setText(ui->etiqueta_total->text());
                    ui->etiqueta_pago->setStyleSheet("border : 0px solid black");
                    QString num;
                    num.setNum(ui->etiqueta_pago->text().toFloat()-ui->etiqueta_total->text().toFloat(),'f',2);
                    ui->etiqueta_cambio->setText(num);
                    ui->etiqueta_pago->setStyleSheet("border : 0px solid black");
                    ui->etiqueta_cambio->setStyleSheet("border : 5px solid green");
                    ui->etiqueta_precio->setStyleSheet("border : 5px solid black");
                    this->cambio = true;
                    this->state = 1;
                    this->position_counter = 1;
                    this->multiplicador = 1;
                    this->cobrar();

                }
                break;
            default:
                break;
            }

        }else if(tecla == Qt::Key_Delete){
            /// ------------------------ BORRAR ---------------------------
            if(this->tecla_anterior == Qt::Key_Delete){
                tecla = -1;

                if(ui->listWidget->count()>0){
                    QString item = ui->listWidget->takeItem(ui->listWidget->count()-1)->text().trimmed();
                    QString num;
                    num.setNum(ui->etiqueta_total->text().toFloat() - item.sliced(item.indexOf("$")+1).toFloat(),'f',2);
                    ui->etiqueta_total->setText(num);
                    this->num_articulos--;
                    if(item.at(0) == 'x'){
                        int cant = QVariant(item.mid(1,5).trimmed()).toInt();
                        this->num_articulos = this->num_articulos - cant + 1;
                        ui->listWidget->takeItem(ui->listWidget->count()-1);
                    }
                    ui->cantidad_art->setText(QVariant(num_articulos).toString());
                }
            }else{
                switch(this->state){
                case 1:
                    if( ui->etiqueta_precio->text().toFloat() > 0){
                        this->state++;
                        ui->etiqueta_precio->setStyleSheet("border : 5px solid blue");
                        ui->etiqueta_dep->setStyleSheet("border : 5px solid black");
                        this->position_counter = 1;
                        QString num;
                        ui->etiqueta_precio->setText(num.setNum(ui->etiqueta_precio->text().toFloat() * -1,'f',2));
                    }else{
                        ui->etiqueta_precio->setStyleSheet("border : 5px solid red");
                        this->position_counter = 1;
                    }
                    break;
                default:
                    break;
                }
            }
        }else if(tecla == Qt::Key_F12){
            /// ------------------------ CONSULTA ---------------------------

            this->consulta->show();

        }else if(tecla == Qt::Key_Insert){
            this->on_reimpresion_clicked();

        }else if(tecla == Qt::Key_F3){
            this->on_boton_corte_x_clicked();

        }else if(tecla == Qt::Key_F6){
            this->on_boton_corte_z_clicked();

        }else if(tecla == Qt::Key_F9){
            this->on_boton_borrar_todo_clicked();

        }else if(tecla == Qt::Key_Shift){
            this->on_pushButton_clicked();

        }

        this->tecla_anterior = tecla;
    }while(this->redo);
}

void Caja::on_reimpresion_clicked()
{
    if(ui->listWidget->count()==0){
        ///imprimir ticket previo
        QFile file("ultima_venta.txt");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox messageBox;
            messageBox.critical(0,"Error","No se ha encontrado el archivo de ultima venta !");
            messageBox.setFixedSize(500,200);
        }

        QTextStream in(&file);
        if(file.isOpen()){
            QPrinterInfo impresora_info = QPrinterInfo::defaultPrinter();
            QPrinter impresora = QPrinter(impresora_info,
                                          QPrinter::PrinterMode::ScreenResolution);
            int ancho= impresora_info.defaultPageSize().rectPixels(impresora.resolution()).width();
            int borde = 15;

            QFontDatabase::addApplicationFont(":/fonts/LucidaTypewriterRegular.ttf");
            QFont fuente = QFont("Lucida Sans Typewriter",10,QFont::Normal,false);
            fuente.setStretch(QFont::Condensed);
            QFontMetrics fuente_metricas = QFontMetrics(fuente);
            int alto_letra = fuente_metricas.height();
            int vspace = 1;

            QImage logo = QImage(":/images/small_elegant_logo.png");
            QPoint punto = QPoint( (ancho-logo.width())/2 ,0);

            QPainter documento;
            documento.begin(&impresora);
            documento.setFont(fuente);
            /********************************/
            documento.drawImage(punto,logo);
            int fila = 3;

            ///TICKET
            while (!in.atEnd()) {
                QString line = in.readLine();
                if( line.indexOf("TOTAL") != -1){
                    fuente.setWeight(QFont::ExtraBold);
                    fuente.setPointSize(15);
                    documento.setFont(fuente);
                    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde+10*vspace, line ); fila++;

                    fila++;

                    fuente.setWeight(QFont::Normal);
                    fuente.setPointSize(10);
                    documento.setFont(fuente);
                }else{
                    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, line); fila++;
                }
            }

            documento.end();
            file.close();
        }
        this->setFocus();
    }
}


void Caja::on_boton_borrar_todo_clicked()
{
    int lim = ui->listWidget->count();
    for(int i = 0 ; i < lim; i++){
        ui->listWidget->takeItem(0)->text();
    }
    this->num_articulos = 0;
    ui->cantidad_art->setText(QVariant(num_articulos).toString());
    ui->etiqueta_precio->setStyleSheet("border : 5px solid black");
    ui->etiqueta_dep->setStyleSheet("border : 0px solid green");
    ui->etiqueta_total->setStyleSheet("border : 0px solid green");
    ui->etiqueta_cambio->setStyleSheet("border : 0px solid green");
    ui->etiqueta_pago->setStyleSheet("border : 0px solid green");
    ui->etiqueta_dep->setText("--");
    ui->etiqueta_multi->setText("");
    ui->etiqueta_precio->setText("     0.00");
    ui->etiqueta_total->setText("       0.00");
    ui->etiqueta_pago->setText("       -.--");
    ui->etiqueta_cambio->setText("       -.--");
    this->multiplicador = 1;
    this->state = 1;
    this->position_counter = 1;
    this->cambio = false;
    this->count_zeros = false;
    this->setFocus();
}


void Caja::on_boton_corte_x_clicked()
{
    float totales_dep[this->departamentos.size() + 1] = {0.0};
    int folio = get_folio();

    QDirIterator it("ventas", QStringList() << "*.txt", QDir::Files);
    while (it.hasNext()){
        it.next();
        QFile file("ventas/" + it.fileName());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox messageBox;
            messageBox.critical(0,"Error","Error abriendo los archivos de ventas");
            messageBox.setFixedSize(500,200);
            break;
        }
        QTextStream in(&file);
        QString pline = "",nline = "";
        while (!in.atEnd()) {

            pline = nline;
            nline = in.readLine().trimmed();

            if(pline.contains("$")){
                QString monto = "";
                QString dep = "";
                if(nline.contains("x")){
                    monto = nline.sliced(nline.indexOf("$")+1).trimmed();
                    dep = pline.sliced(0,pline.size()-pline.indexOf("$")+1).trimmed();
                    totales_dep[this->departamentos.indexOf(dep)] += monto.toFloat();
                    nline = in.readLine().trimmed();

                }else if(pline.contains("TOTAL")){
                    monto = pline.sliced(pline.indexOf("$")+1).trimmed();
                    totales_dep[this->departamentos.size()] += monto.toFloat();
                    break;

                }else{
                    monto = pline.sliced(pline.indexOf("$")+1).trimmed();
                    dep = pline.sliced(0,pline.size()-pline.indexOf("$")).trimmed();
                    totales_dep[this->departamentos.indexOf(dep)] += monto.toFloat();

                }
            }
        }
        file.close();
    }

    QPrinterInfo impresora_info = QPrinterInfo::defaultPrinter();
    QPrinter impresora = QPrinter(impresora_info,
                                  QPrinter::PrinterMode::ScreenResolution);
    int ancho= impresora_info.defaultPageSize().rectPixels(impresora.resolution()).width();
    int borde = 15;
    QFontDatabase::addApplicationFont(":/fonts/LucidaTypewriterRegular.ttf");
    QFont fuente = QFont("Lucida Sans Typewriter",10,QFont::Normal,false);
    fuente.setStretch(QFont::Condensed);
    QFontMetrics fuente_metricas = QFontMetrics(fuente);
    int alto_letra = fuente_metricas.height();
    int vspace = 1;
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime =  "Fecha :        " + date.toString("dd/MM/yyyy       hh:mm:ss");
    int fila = 3;
    QPainter documento;
    QImage logo = QImage(":/images/small_elegant_logo.png");
    QPoint punto = QPoint( (ancho-logo.width())/2 ,0);
    documento.begin(&impresora);
    documento.setFont(fuente);
    documento.drawImage(punto,logo);
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "         Tipo de Reporte : CORTE X          "); fila++;
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "                                        "); fila++;
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, formattedTime); fila++;
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "========================================"); fila++;

    ///Articulos Aqui
    QString temp_dep;
    QString temp_total;
    QString resumen = "";
    for(int i = 0; i < this->departamentos.size()-1; i++){
        temp_dep = this->departamentos.at(i).leftJustified(15,' ',true);
        temp_total.setNum(totales_dep[i],'f',2);
        temp_total = temp_total.rightJustified(21,' ',false);
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde+10*vspace, temp_dep + " = $" + temp_total); fila++;
    }
    fila++;
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "========================================"); fila++;
    QString total_ventas = "Total ventas : " + QVariant(folio).toString();
    total_ventas = total_ventas.leftJustified(40,' ',true);
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, total_ventas); fila++;
    fuente.setWeight(QFont::ExtraBold);
    fuente.setPointSize(15);
    documento.setFont(fuente);
    temp_dep = QString("TOTAL = $").leftJustified(14,' ',true);
    temp_total.setNum(totales_dep[this->departamentos.size()],'f',2);
    temp_total = temp_total.rightJustified(10,' ',false);
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde+10*vspace, temp_dep + temp_total);
    documento.end();
    this->setFocus();
}


void Caja::on_boton_corte_z_clicked()
{
    float totales_dep[this->departamentos.size() + 1] = {0.0};
    int folio = get_folio();
    aumentar_folio(false);

    QDirIterator it("ventas", QStringList() << "*.txt", QDir::Files);
    while (it.hasNext()){
        it.next();
        QFile file("ventas/" + it.fileName());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox messageBox;
            messageBox.critical(0,"Error","Error abriendo los archivos de ventas");
            messageBox.setFixedSize(500,200);
            break;
        }
        QTextStream in(&file);
        QString pline = "",nline = "";
        while (!in.atEnd()) {

            pline = nline;
            nline = in.readLine().trimmed();

            if(pline.contains("$")){
                QString monto = "";
                QString dep = "";
                if(nline.contains("x")){
                    monto = nline.sliced(nline.indexOf("$")+1).trimmed();
                    dep = pline.sliced(0,pline.size()-pline.indexOf("$")+1).trimmed();
                    totales_dep[this->departamentos.indexOf(dep)] += monto.toFloat();
                    nline = in.readLine().trimmed();

                }else if(pline.contains("TOTAL")){
                    monto = pline.sliced(pline.indexOf("$")+1).trimmed();
                    totales_dep[this->departamentos.size()] += monto.toFloat();
                    break;

                }else{
                    monto = pline.sliced(pline.indexOf("$")+1).trimmed();
                    dep = pline.sliced(0,pline.size()-pline.indexOf("$")).trimmed();
                    totales_dep[this->departamentos.indexOf(dep)] += monto.toFloat();

                }
            }
        }
        file.close();
        QFile::remove("ventas/" + it.fileName());
    }

    QPrinterInfo impresora_info = QPrinterInfo::defaultPrinter();
    QPrinter impresora = QPrinter(impresora_info,
                                  QPrinter::PrinterMode::ScreenResolution);
    int ancho= impresora_info.defaultPageSize().rectPixels(impresora.resolution()).width();
    int borde = 15;
    QFontDatabase::addApplicationFont(":/fonts/LucidaTypewriterRegular.ttf");
    QFont fuente = QFont("Lucida Sans Typewriter",10,QFont::Normal,false);
    fuente.setStretch(QFont::Condensed);
    QFontMetrics fuente_metricas = QFontMetrics(fuente);
    int alto_letra = fuente_metricas.height();
    int vspace = 1;
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime =  "Fecha :        " + date.toString("dd/MM/yyyy       hh:mm:ss");
    int fila = 3;
    QPainter documento;
    QImage logo = QImage(":/images/small_elegant_logo.png");
    QPoint punto = QPoint( (ancho-logo.width())/2 ,0);
    documento.begin(&impresora);
    documento.setFont(fuente);
    documento.drawImage(punto,logo);
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "        Tipo de Reporte : CORTE Z       "); fila++;
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "                                        "); fila++;
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, formattedTime); fila++;
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "========================================"); fila++;

    ///Articulos Aqui
    QString temp_dep;
    QString temp_total;
    QString resumen = "";
    for(int i = 0; i < this->departamentos.size()-1; i++){
        temp_dep = this->departamentos.at(i).leftJustified(15,' ',true);
        temp_total.setNum(totales_dep[i],'f',2);
        temp_total = temp_total.rightJustified(21,' ',false);
        documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde+10*vspace, temp_dep + " = $" + temp_total); fila++;
    }
    fila++;
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, "========================================"); fila++;
    QString total_ventas = "Total ventas : " + QVariant(folio).toString();
    total_ventas = total_ventas.leftJustified(40,' ',true);
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde, total_ventas); fila++;
    fuente.setWeight(QFont::ExtraBold);
    fuente.setPointSize(15);
    documento.setFont(fuente);
    temp_dep = QString("TOTAL = $").leftJustified(14,' ',true);
    temp_total.setNum(totales_dep[this->departamentos.size()],'f',2);
    temp_total = temp_total.rightJustified(10,' ',false);
    documento.drawText(borde, (fila*alto_letra)+(fila*vspace)+borde+10*vspace, temp_dep + temp_total);
    documento.end();
    this->setFocus();
}


void Caja::on_check_tickets_clicked()
{
    this->setFocus();
}


void Caja::on_pushButton_clicked()
{
    QPrinterInfo impresora_info = QPrinterInfo::defaultPrinter();
    QPrinter impresora = QPrinter(impresora_info, QPrinter::PrinterMode::ScreenResolution);

    QPainter documento;
    documento.begin(&impresora);
    documento.end();
    this->setFocus();
}

