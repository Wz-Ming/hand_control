#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    autorun_obj=new autoRun();
    autorun_obj->duoji_select_id=-1;
    set_shift_value=false;
    autorun_obj->run_serial=false;
    autorun_obj->run_loop=false;
    autorun_obj->auto_start=false;
    autorun_obj->send_busy=false;
    autorun_obj->scan_id=false;
    init_param();
    open_serialport();
    autorun_obj->sms=-1;
    autorun_thread=new QThread();
    autorun_obj->moveToThread(autorun_thread);
    connect(autorun_thread,SIGNAL(started()),autorun_obj,SLOT(mainloop()));
    connect(autorun_thread,SIGNAL(finished()),autorun_obj,SLOT(Mainloop_stop()));
    connect(autorun_obj,SIGNAL(update_control_signal(int)),this,SLOT(update_control_fun(int)));
    autorun_thread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete autorun_obj;
    delete autorun_thread;
}

void MainWindow::init_param()
{
    int currenRow=-1;
    if(QFile::exists("/home/left_hand_pos.txt"))
    {
        QFile file("/home/left_hand_pos.txt");
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&file);
        while(!in.atEnd())
        {
          currenRow=ui->pos_widget->currentRow();
          QString line=in.readLine();
          ui->pos_widget->insertItem(currenRow+1,line);
          ui->pos_widget->setCurrentRow(currenRow+1);
          QString motion="G01 P"+QString::number(currenRow+1)+" 500 1000";
          ui->motion_widget->insertItem(currenRow+1,motion);
          ui->motion_widget->setCurrentRow(currenRow+1);
        }
        file.close();
    }
}

void MainWindow::init_tablewidget()
{
    ui->tableWidget->clear();
    QList<QString>addr_explain;
    QList<uchar>reg_addr;
    QList<int>addr_num;
    addr_explain.clear();
    reg_addr.clear();
    addr_num.clear();
    QTableWidgetItem *headerItem;
    QStringList widgethead={"内存","数值","存储区域","读写"};
    ui->tableWidget->setColumnCount(widgethead.size());
    for(int i=0;i<ui->tableWidget->columnCount();i++)
    {
        headerItem=new QTableWidgetItem(widgethead.at(i));
        ui->tableWidget->setHorizontalHeaderItem(i,headerItem);
    }
    if(autorun_obj->sms==1)
    {

        addr_explain=sms_addr_explain;
        reg_addr=sms_reg_addr;
        addr_num=sms_addr_num;
    }
    else
    {
        addr_explain=scs_addr_explain;
        reg_addr=scs_reg_addr;
        addr_num=scs_addr_num;
    }
    ui->tableWidget->setColumnWidth(0,150);
    ui->tableWidget->setColumnWidth(1,100);
    ui->tableWidget->setColumnWidth(2,80);
    ui->tableWidget->setColumnWidth(3,70);
    ui->tableWidget->setRowCount(addr_explain.size());
    for(int i=0;i<reg_addr.length();i++)
    {
        headerItem=new QTableWidgetItem(addr_explain.at(i));
        ui->tableWidget->setItem(i,0,headerItem);
        if((reg_addr.at(i) & 0x000000ff)<40)
        {
            headerItem=new QTableWidgetItem("EPROM");
            ui->tableWidget->setItem(i,2,headerItem);
        }
        else
        {
            headerItem=new QTableWidgetItem("SRAM");
            ui->tableWidget->setItem(i,2,headerItem);
        }
        if((reg_addr.at(i) & 0x000000ff)>4 && (reg_addr.at(i) & 0x000000ff)<56)
        {
            headerItem=new QTableWidgetItem("读写");
            ui->tableWidget->setItem(i,3,headerItem);
        }
        else
        {
            headerItem=new QTableWidgetItem("只读");
            ui->tableWidget->setItem(i,3,headerItem);
        }
    }
}

void MainWindow::update_control_fun(int flag)
{   
    if(flag==1)
    {
        if(autorun_obj->duoji_wrong!=-1)
        {
            ui->textEdit->append(QString::number(autorun_obj->duoji_wrong)+"号电机运行失败！");
            autorun_obj->run_loop=false;
            autorun_obj->auto_start=false;
        }
        else
        {
            if(autorun_obj->run_serial)
            {
                if(autorun_obj->move_row==ui->motion_widget->count()-1)
                {
                    autorun_obj->move_row=0;
                    ui->motion_widget->setCurrentRow(autorun_obj->move_row);
                }
                else
                {
                    autorun_obj->move_row+=1;
                    ui->motion_widget->setCurrentRow(autorun_obj->move_row);
                }
                QStringList motion_mes=ui->motion_widget->item(autorun_obj->move_row)->text().split(" ",QString::SkipEmptyParts);
                autorun_obj->motion_cmd=motion_mes.at(0);
                autorun_obj->motion_point=motion_mes.at(1);
                autorun_obj->motion_point=autorun_obj->motion_point.split("P",QString::SkipEmptyParts).at(0);
                autorun_obj->motion_time=motion_mes.at(2);
                autorun_obj->motion_spd=motion_mes.at(3);
                autorun_obj->pos_list.clear();
                QString pos_mes=ui->pos_widget->item(autorun_obj->motion_point.toInt())->text().split(" ",QString::SkipEmptyParts).at(1);
                QStringList pos_list=pos_mes.split(",",QString::SkipEmptyParts);
                for(int i=0;i<pos_list.length();i++)
                {
                    QString single_pos=pos_list.at(i);
                    autorun_obj->pos_list.append(single_pos.toInt());
                }
                autorun_obj->run_loop=true;
            }
            else
            {
                if(autorun_obj->move_row<ui->motion_widget->count()-1)
                {
                    autorun_obj->move_row+=1;
                    ui->motion_widget->setCurrentRow(autorun_obj->move_row);
                }
                else
                {
                    autorun_obj->move_row=0;
                    ui->motion_widget->setCurrentRow(autorun_obj->move_row);
                }
            }
        }
    }
    else if(flag==0 && autorun_obj->duoji_select_id!=-1)
    {
        while(autorun_obj->send_busy)
        {
            ;
        }
        autorun_obj->send_busy=true;
        ideal_pos=autorun_obj->read_bytes(autorun_obj->duoji_select_id,reg_ideal_pos,2,autorun_obj->sms);
        current_pos=autorun_obj->read_bytes(autorun_obj->duoji_select_id,reg_current_pos,2,autorun_obj->sms);
        current_spd=autorun_obj->read_bytes(autorun_obj->duoji_select_id,reg_current_spd,2,autorun_obj->sms);
        current_load=autorun_obj->read_bytes(autorun_obj->duoji_select_id,reg_current_load,2,autorun_obj->sms);
        current_voltage=autorun_obj->read_bytes(autorun_obj->duoji_select_id,reg_current_voltage,1,autorun_obj->sms);
        current_temp=autorun_obj->read_bytes(autorun_obj->duoji_select_id,reg_current_temp,1,autorun_obj->sms);
        current_state=autorun_obj->read_bytes(autorun_obj->duoji_select_id,reg_current_state,1,autorun_obj->sms);
        move_state=autorun_obj->read_bytes(autorun_obj->duoji_select_id,reg_move_state,1,autorun_obj->sms);
        autorun_obj->send_busy=false;
        if(ideal_pos!=-1)ui->label_target->setText(QString::number(ideal_pos));
        if(current_pos!=-1)ui->label_position->setText(QString::number(current_pos));
        if(current_spd!=-1)
        {
            if(current_spd>=32768)current_spd=32768-current_spd;
            ui->label_speed->setText(QString::number(current_spd));
        }
        if(current_load!=-1)ui->label_torque->setText(QString::number(current_load*0.1)+"%");
        if(current_voltage!=-1)ui->label_volatage->setText(QString::number(current_voltage*0.1)+"V");
        if(current_temp!=-1)ui->label_temperature->setText(QString::number(current_temp)+"°C");
        if(current_state!=-1)
        {
            if(current_state==0)ui->label_status->setText("工作正常!");
            else if(current_state==1)ui->label_status->setText("电压报警!");
            else if(current_state==4)ui->label_status->setText("温度报警!");
            else if(current_state==32)ui->label_status->setText("过载报警!");
            else ui->label_status->setText("异常!");
        }

        if(move_state!=-1)ui->label_movement->setText(QString::number(move_state));
    }
    else if(flag==2)
    {
        int currenRow = ui->listWidget->currentRow();
        ui->listWidget->insertItem(currenRow+1,autorun_obj->duoji_list);
        ui->listWidget->setCurrentRow(currenRow+1);
        autorun_obj->scan_stop=false;
    }
    else if(flag==3)ui->search_id->setText("搜索");
    else if(flag==4)ui->label_status->setText("搜索:"+autorun_obj->duoji_list);
}

void MainWindow::open_serialport()
{
    if(autorun_obj->serial_port_sms->isOpen())autorun_obj->serial_port_sms->close();
    autorun_obj->serial_port_sms->setBaudRate(QSerialPort::Baud115200);//  Baud19200);QSerialPort::Baud115200
    autorun_obj->serial_port_sms->setDataBits(QSerialPort::Data8);
    autorun_obj->serial_port_sms->setParity(QSerialPort::NoParity);
    autorun_obj->serial_port_sms->setStopBits(QSerialPort::OneStop);//QSerialPort::OneStop
    autorun_obj->serial_port_sms->setFlowControl(QSerialPort::NoFlowControl);
    autorun_obj->serial_port_sms->setReadBufferSize(2048);
    bool open_state=false;
    open_state=autorun_obj->serial_port_sms->open(QIODevice::ReadWrite);
    if(open_state)
    {
        ui->textEdit->append("sms_dudoji open successful!");
//        ui->textEdit->append(QString::number(autorun_obj->serial_port_sms->baudRate()));
    }
    else
    {
        ui->textEdit->append("sms_duoji open failed!");
    }

    if(autorun_obj->serial_port_scs->isOpen())autorun_obj->serial_port_scs->close();
    autorun_obj->serial_port_scs->setBaudRate(QSerialPort::Baud115200);//  Baud19200);QSerialPort::Baud115200
    autorun_obj->serial_port_scs->setDataBits(QSerialPort::Data8);
    autorun_obj->serial_port_scs->setParity(QSerialPort::NoParity);
    autorun_obj->serial_port_scs->setStopBits(QSerialPort::OneStop);
    autorun_obj->serial_port_scs->setFlowControl(QSerialPort::NoFlowControl);
    autorun_obj->serial_port_scs->setReadBufferSize(2048);
    open_state=false;
    open_state=autorun_obj->serial_port_scs->open(QIODevice::ReadWrite);
    if(open_state)
    {
        ui->textEdit->append("scs_duoji open successful!");
    }
    else
    {
        ui->textEdit->append("scs_duoji open failed!");
    }
}

void MainWindow::on_start_move_clicked()
{
    int duoji_pos=ui->duoji_pos->text().trimmed().toInt();
    int duoji_spd=ui->duoji_spd->text().trimmed().toInt();
    int duoji_time=ui->duoji_time->text().trimmed().toInt();
    while(autorun_obj->send_busy)
    {
        ;
    }
    autorun_obj->send_busy=true;
    autorun_obj->set_move(autorun_obj->duoji_select_id,duoji_pos,duoji_spd,duoji_time,autorun_obj->sms);
    autorun_obj->send_busy=false;
}

void MainWindow::on_clear_text_clicked()
{
    ui->textEdit->clear();
}

void MainWindow::on_shift_pos_valueChanged(int value)
{
    if(set_shift_value==false)
    {
        int duoji_spd=ui->duoji_spd->text().trimmed().toInt();
        int duoji_time=ui->duoji_time->text().trimmed().toInt();
        int duoji_pos=value;
        while(autorun_obj->send_busy)
        {
            ;
        }
        autorun_obj->send_busy=true;
        autorun_obj->set_move(autorun_obj->duoji_select_id,duoji_pos,duoji_spd,duoji_time,autorun_obj->sms);
        autorun_obj->send_busy=false;
    }
}

void MainWindow::on_act_save_file_triggered()
{
    QString dlgTitle="Question MessageBox";
    QString strInfo="文件已被修改，是否保存?";
    QMessageBox::StandardButton defaultbtn=QMessageBox::NoButton;
    QMessageBox::StandardButton result;
    result=QMessageBox::question(this,dlgTitle,strInfo,QMessageBox::Yes|QMessageBox::No,defaultbtn);
    if(result==QMessageBox::Yes)
    {
        QFile file("/home/left_hand_pos.txt");
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
            QTextStream out(&file);
            if(ui->pos_widget->count()>0)
            {
                for(int i=0;i<ui->pos_widget->count();i++)
                {
                    QString pos_message=ui->pos_widget->item(i)->text();
                    out<<QString("%1").arg(pos_message)<<"\r\n";
                }
            }
            file.close();
            ui->textEdit->append("已保存！");
        }
        else ui->textEdit->append("保存失败！");
    }
    else
    {
        ui->textEdit->append("已取消保存！");
    }
}


void MainWindow::on_search_id_clicked()
{   
    if(autorun_obj->scan_id)
    {
        while(autorun_obj->send_busy)
        {
            ;
        }
        autorun_obj->scan_id=false;
        ui->search_id->setText("搜索");
    }
    else
    {
        ui->listWidget->clear();
        autorun_obj->scan_id=true;
        ui->search_id->setText("停止");
    }
}

void MainWindow::on_listWidget_clicked(const QModelIndex &index)
{
    QString duoji_message=ui->listWidget->currentItem()->text();
    if(duoji_message.isEmpty()==false)
    {
        QString duoji_num=duoji_message.split(":",QString::SkipEmptyParts).at(0);
        autorun_obj->duoji_select_id=duoji_num.toInt();
        if(duoji_num.toInt()==107 || duoji_num.toInt()==108)
        {
            ui->shift_pos->setMaximum(4095);
            autorun_obj->sms=1;
        }
        else
        {
            ui->shift_pos->setMaximum(4095);
            autorun_obj->sms=0;
        }
        set_shift_value=true;
        while(autorun_obj->send_busy)
        {
            ;
        }
        autorun_obj->send_busy=true;
        int read_data=autorun_obj->read_bytes(autorun_obj->duoji_select_id,(uchar)(0x38),2,autorun_obj->sms);
        if(read_data!=-1)ui->shift_pos->setValue(read_data);
        autorun_obj->send_busy=false;
        set_shift_value=false;
        on_read_par_clicked();
    }
}

void MainWindow::on_act_exit_triggered()
{
     while(autorun_obj->send_busy)
     {
         ;
     }
     autorun_obj->run_var=false;
     QThread::msleep(50);
     autorun_thread->exit();
     close();
}

void MainWindow::on_update_pos_clicked()
{
    int read_data=-1;
    bool update_en=true;
    QString update_pos="P"+QString::number(ui->pos_widget->currentRow())+" ";
    uchar reg_pos;
    if(ui->ideal_pos->checkState()==Qt::Checked)reg_pos=uchar(0x2a);
    else reg_pos=uchar(0x38);
    while(autorun_obj->send_busy)
    {
        ;
    }
    autorun_obj->send_busy=true;
    for(int i=0;i<autorun_obj->duoji_cnt;i++)
    {
        read_data=-1;
        if(i==autorun_obj->sms_duoji_pos[0] || i==autorun_obj->sms_duoji_pos[1])read_data=autorun_obj->read_bytes(i+autorun_obj->duoji_begin_pos,reg_pos,2,1);
        else read_data=autorun_obj->read_bytes(i+autorun_obj->duoji_begin_pos,uchar(0x38),2,0);
        if(read_data==-1)
        {
            ui->textEdit->append(QString::number(i+autorun_obj->duoji_begin_pos)+"号舵机读取位置失败!");
            update_en=false;
            break;
        }
        else
        {
            update_pos+=QString::number(read_data);
            if(i!=7) update_pos+=",";
        }
    }
    autorun_obj->send_busy=false;
    if(update_en)
    {
        int currenRow = ui->pos_widget->currentRow();
        ui->pos_widget->insertItem(currenRow+1,update_pos);
        ui->pos_widget->takeItem(currenRow);
    }
}

void MainWindow::on_auto_start_clicked()
{
    if(ui->motion_widget->currentRow() == -1)return ;
    autorun_obj->delay_time=ui->delay_time->text().trimmed().toInt();
    autorun_obj->move_row=ui->motion_widget->currentRow();
    QStringList motion_mes=ui->motion_widget->item(autorun_obj->move_row)->text().split(" ",QString::SkipEmptyParts);
    autorun_obj->motion_cmd=motion_mes.at(0);
    if(autorun_obj->motion_cmd=="G01")
    {
        autorun_obj->motion_point=motion_mes.at(1);
        autorun_obj->motion_point=autorun_obj->motion_point.split("P",QString::SkipEmptyParts).at(0);
        autorun_obj->motion_time=motion_mes.at(2);
        autorun_obj->motion_spd=motion_mes.at(3);
        autorun_obj->pos_list.clear();
        QString pos_mes=ui->pos_widget->item(autorun_obj->motion_point.toInt())->text().split(" ",QString::SkipEmptyParts).at(1);
        QStringList pos_list=pos_mes.split(",",QString::SkipEmptyParts);
        for(int i=0;i<pos_list.length();i++)
        {
            QString single_pos=pos_list.at(i);
            autorun_obj->pos_list.append(single_pos.toInt());
        }
        autorun_obj->auto_start=true;
        autorun_obj->run_loop=true;
        if(autorun_obj->run_serial)
        {
            ui->debug_box->setEnabled(false);
            ui->act_exit->setEnabled(false);
            ui->act_save_file->setEnabled(false);
            ui->update_pos->setEnabled(false);
            ui->delete_pos->setEnabled(false);
            ui->add_pos->setEnabled(false);
            ui->ideal_pos->setEnabled(false);
            ui->auto_start->setEnabled(false);
            ui->read_par->setEnabled(false);
            ui->write_par->setEnabled(false);
        }
    }
    else ui->textEdit->append("指令错误！");
}

void MainWindow::on_add_pos_clicked()
{
    int currenRow=ui->pos_widget->count();
    int read_data=-1;
    bool update_en=true;
    QString update_pos="P"+QString::number(currenRow)+" ";
    uchar reg_pos;
    if(ui->ideal_pos->checkState()==Qt::Checked)reg_pos=uchar(0x2a);
    else reg_pos=uchar(0x38);
    for(int i=0;i<autorun_obj->duoji_cnt;i++)
    {
        read_data=-1;
        if(i==autorun_obj->sms_duoji_pos[0] || i==autorun_obj->sms_duoji_pos[1])read_data=autorun_obj->read_bytes(i+autorun_obj->duoji_begin_pos,reg_pos,2,1);
        else read_data=autorun_obj->read_bytes(i+autorun_obj->duoji_begin_pos,uchar(0x38),2,0);
        if(read_data==-1)
        {
            ui->textEdit->append(QString::number(i+autorun_obj->duoji_begin_pos)+"号舵机读取位置失败!");
            update_en=false;
            break;
        }
        else
        {
            update_pos+=QString::number(read_data);
            if(i!=7) update_pos+=",";
        }
    }
    if(update_en)
    {
        ui->pos_widget->insertItem(currenRow,update_pos);
        ui->pos_widget->setCurrentRow(currenRow);
    }
}

void MainWindow::on_delete_pos_clicked()
{
    int currenRow = ui->pos_widget->currentRow();
    ui->pos_widget->takeItem(currenRow);
}

void MainWindow::on_text_to_widget_clicked()
{
    ui->motion_widget->clear();
    QStringList text=ui->textEdit->toPlainText().split("\n",QString::SkipEmptyParts);
    for(int i=0;i<text.length();i++)
    {
        ui->motion_widget->insertItem(i,text.at(i));
        if(i<text.length()-1)ui->motion_widget->setCurrentRow(i+1);
    }
}

void MainWindow::on_widget_to_text_clicked()
{
    int item_num=ui->motion_widget->count();
    ui->textEdit->clear();
    for(int i=0;i<item_num;i++)
    {
        ui->textEdit->append(ui->motion_widget->item(i)->text());
    }
}

void MainWindow::on_motion_serial_clicked()
{
    if(ui->motion_serial->checkState()==Qt::Checked)
    {
        autorun_obj->run_serial=true;
    }
    else
    {
        while(autorun_obj->send_busy)
        {
            ;
        }
        autorun_obj->run_serial=false;
        autorun_obj->auto_start=false;
        autorun_obj->run_loop=false;
        ui->debug_box->setEnabled(true);
        ui->act_exit->setEnabled(true);
        ui->act_save_file->setEnabled(true);
        ui->update_pos->setEnabled(true);
        ui->delete_pos->setEnabled(true);
        ui->add_pos->setEnabled(true);
        ui->ideal_pos->setEnabled(true);
        ui->auto_start->setEnabled(true);
        ui->read_par->setEnabled(true);
        ui->write_par->setEnabled(true);
    }
}

void MainWindow::on_read_par_clicked()
{
    int read_data=-1;
    init_tablewidget();
    QTableWidgetItem  *headerItem;
    while(autorun_obj->send_busy)
    {
        ;
    }
    autorun_obj->send_busy=true;
    if(autorun_obj->sms==1)
    {
        for(int i=0;i<sms_reg_addr.length();i++)
        {
            read_data=autorun_obj->read_bytes(autorun_obj->duoji_select_id,sms_reg_addr.at(i),sms_addr_num.at(i),autorun_obj->sms);
            if(read_data==-1)
            {
                ui->textEdit->append(sms_addr_explain.at(i)+" 读取失败!");
                break;
            }
            else
            {
                headerItem=new QTableWidgetItem(QString::number(read_data));
                ui->tableWidget->setItem(i,1,headerItem);
            }
        }
    }
    else if(autorun_obj->sms==0)
    {
        for(int i=0;i<scs_reg_addr.length();i++)
        {
            read_data=autorun_obj->read_bytes(autorun_obj->duoji_select_id,scs_reg_addr.at(i),scs_addr_num.at(i),autorun_obj->sms);
            if(read_data==-1)
            {
                ui->textEdit->append(scs_addr_explain.at(i)+" 读取失败!");
                break;
            }
            else
            {
                headerItem=new QTableWidgetItem(QString::number(read_data));
                ui->tableWidget->setItem(i,1,headerItem);
            }
        }
    }
    autorun_obj->send_busy=false;
}

void MainWindow::on_write_par_clicked()
{
    QString dlgTitle="Question MessageBox";
    QString strInfo="是否保存修改?";
    QMessageBox::StandardButton defaultbtn=QMessageBox::NoButton;
    QMessageBox::StandardButton result;
    result=QMessageBox::question(this,dlgTitle,strInfo,QMessageBox::Yes|QMessageBox::No,defaultbtn);
    if(result==QMessageBox::Yes)
    {
        QTableWidgetItem  *headerItem;
        while(autorun_obj->send_busy)
        {
            ;
        }
        autorun_obj->send_busy=true;
        if(autorun_obj->sms==1)
        {
            for(int i=0;i<sms_reg_addr.length();i++)
            {
                headerItem=ui->tableWidget->item(i,1);
                int read_data=autorun_obj->write_bytes(autorun_obj->duoji_select_id,sms_reg_addr.at(i),headerItem->text().trimmed().toInt(),sms_addr_num.at(i),autorun_obj->sms);
                if(read_data==-1)
                {
                    headerItem=ui->tableWidget->item(i,0);
                    ui->textEdit->append(headerItem->text()+":写入失败!");
                    break;
                }
            }
        }
        else if(autorun_obj->sms==0)
        {
            for(int i=0;i<scs_reg_addr.length();i++)
            {
                headerItem=ui->tableWidget->item(i,1);
                int read_data=autorun_obj->write_bytes(autorun_obj->duoji_select_id,scs_reg_addr.at(i),headerItem->text().trimmed().toInt(),scs_addr_num.at(i),autorun_obj->sms);
                if(read_data==-1)
                {
                    headerItem=ui->tableWidget->item(i,0);
                    ui->textEdit->append(headerItem->text()+":写入失败!");
                    break;
                }
            }
        }
        autorun_obj->send_busy=false;
    }
}
