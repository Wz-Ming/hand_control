#include"autorun.h"
autoRun::autoRun(QObject *parent) : QObject(parent)
{
    serial_port_sms=new QSerialPort("/dev/ttyPS4");
    serial_port_scs=new QSerialPort("/dev/ttyPS3");
}
autoRun::~autoRun()
{
    delete serial_port_sms;
    delete serial_port_scs;
}
void autoRun::Mainloop_stop()
{
    run_var=false;
}
void autoRun::mainloop()
{
    run_var=true;
    while(run_var)
    {
        if(auto_start)
        {
            if(run_serial)
            {
                time.restart();
                while(run_loop==false)
                {
                    if(time.elapsed()>50)
                    {
                       run_loop=true;
                       break;
                    }
                }
                QThread::msleep(delay_time);
                int receive_data=-1;
                duoji_wrong=-1;
                while(send_busy)
                {
                    ;
                }
                send_busy=true;
                for(int i=0;i<duoji_cnt;i++)
                {
                    if(i==sms_duoji_pos[0] || i==sms_duoji_pos[1])
                    {
                        int move_state=1;
                        while(move_state!=0)
                        {
                            move_state=read_bytes(i+duoji_begin_pos,uchar(0x42),1,1);
                        }
                        receive_data=set_move(i+duoji_begin_pos,pos_list.at(i),motion_spd.toInt(),motion_time.toInt(),1);
                    }
                    else
                    {
                        int move_state=1;
                        while(move_state!=0)
                        {
                            move_state=read_bytes(i+duoji_begin_pos,uchar(0x42),1,0);
                        }
                        receive_data=set_move(i+duoji_begin_pos,pos_list.at(i),motion_spd.toInt(),motion_time.toInt(),0);
                    }
                    if(receive_data==-1)
                    {
                        duoji_wrong=i+1;
                        break;
                    }
                }
                send_busy=false;
                update_control_signal(1);
                run_loop=false;
            }
            else
            {
                QThread::msleep(100);
                int receive_data=-1;
                duoji_wrong=-1;
                while(send_busy)
                {
                    ;
                }
                send_busy=true;
                for(int i=0;i<duoji_cnt;i++)
                {
                    if(i==sms_duoji_pos[0] || i==sms_duoji_pos[1])
                    {
                        int move_state=1;
                        while(move_state!=0)
                        {
                            move_state=read_bytes(i+duoji_begin_pos,uchar(0x42),1,1);
                        }
                        receive_data=set_move(i+duoji_begin_pos,pos_list.at(i),motion_spd.toInt(),motion_time.toInt(),1);
                    }
                    else
                    {
                        int move_state=1;
                        while(move_state!=0)
                        {
                            move_state=read_bytes(i+duoji_begin_pos,uchar(0x42),1,0);
                        }
                        receive_data=set_move(i+duoji_begin_pos,pos_list.at(i),motion_spd.toInt(),motion_time.toInt(),0);
                    }
                    if(receive_data==-1)
                    {
                        duoji_wrong=i+1;
                        break;
                    }
                }
                send_busy=false;
                update_control_signal(1);
                auto_start=false;
            }
        }
        else
        {
            if(scan_id)
            {
                int duoji_id=0;
                for(int i=100;i<=110;i++)
                {
                    if(scan_id==false)break;
                    duoji_id=-1;
                    while(send_busy)
                    {
                        ;
                    }
                    send_busy=true;
                    duoji_id=search_sms_duoji_id(i,(uchar)(0x05),1);
                    send_busy=false;
                    if(duoji_id!=-1)
                    {
                        if(duoji_id==1)duoji_list=QString::number(duoji_id)+":手腕X";
                        else if(duoji_id==2)duoji_list=QString::number(duoji_id)+":手腕Y";
                        else duoji_list=QString::number(duoji_id)+":SMS";
                        scan_stop=true;
                        update_control_signal(2);
                        QThread::msleep(10);
                        while(scan_stop)
                        {
                            ;
                        }
                    }

                    duoji_id=-1;
                    while(send_busy)
                    {
                        ;
                    }
                    send_busy=true;
                    duoji_id=search_scs_duoji_id(i,(uchar)(0x05),1);
                    send_busy=false;
                    if(duoji_id!=-1)
                    {
                        if(duoji_id==3)duoji_list=QString::number(duoji_id)+":中指";
                        else if(duoji_id==4)duoji_list=QString::number(duoji_id)+":无名指";
                        else if(duoji_id==5)duoji_list=QString::number(duoji_id)+":小拇指";
                        else if(duoji_id==6)duoji_list=QString::number(duoji_id)+":大拇指X";
                        else if(duoji_id==7)duoji_list=QString::number(duoji_id)+":食指";
                        else if(duoji_id==8)duoji_list=QString::number(duoji_id)+":大拇指Y";
                        else duoji_list=QString::number(duoji_id)+":SCS";
                        scan_stop=true;
                        update_control_signal(2);
                        QThread::msleep(10);
                        while(scan_stop)
                        {
                            ;
                        }
                    }
                    duoji_list=QString::number(i);
                    update_control_signal(4);
                }
                update_control_signal(3);
                scan_id=false;
            }
            else
            {
                QThread::msleep(100);
                update_control_signal(0);
            }
        }
    }
}

int autoRun::read_bytes(int id,uchar addr,int by_len,int sms)
{
    QByteArray send_data;
    QByteArray check_array;
    int check_test=-1;
    int read_data=-1;
    send_data.resize(8);
    check_test=id+4+2+(addr & 0x000000ff)+by_len;
    check_array.resize(1);
    check_array[0] = (uchar)(0x000000ff & check_test);
    check_array[0]=check_array[0]^0xff;

    send_data[0]=(uchar)(0xff);send_data[1]=(uchar)(0xff);
    send_data[2]=(uchar)(0x000000ff & id);
    send_data[3]=(uchar)(0x04);
    send_data[4]=(uchar)(0x02);send_data[5]=addr;
    if(by_len==1) send_data[6]=(uchar)(0x01);
    else if(by_len==2)send_data[6]=(uchar)(0x02);
    send_data[7]=check_array[0];
    if(sms==1)
    {
        serial_port_sms->write(send_data);
        serial_port_sms->waitForBytesWritten(10);
        bool receive_en=true;
        time.restart();
        while (serial_port_sms->bytesAvailable()<6+by_len)
        {
            serial_port_sms->waitForReadyRead(10);
            if(time.elapsed()>50){
                receive_en=false;
                break;
            }
        }
        if(receive_en)
        {
            read_data=sms_receivedata(by_len);
            return read_data;
        }
        else
        {
            serial_port_sms->readAll();
            return -1;
        }
    }
    else if(sms==0)
    {
        serial_port_scs->write(send_data);
        serial_port_scs->waitForBytesWritten(10);
        bool receive_en=true;
        time.restart();
        while (serial_port_scs->bytesAvailable()<6+by_len)
        {
            serial_port_scs->waitForReadyRead(10);
            if(time.elapsed()>50){
                receive_en=false;
                break;
            }
        }
        if(receive_en)
        {
            read_data=scs_receivedata(by_len);
            return read_data;
        }
        else
        {
            serial_port_scs->readAll();
            return -1;
        }
    }
    else return -1;
}

int autoRun::write_bytes(int id,uchar addr,int data,int by_len,int sms)
{

    QByteArray send_data;
    QByteArray check_array;
    int check_test=-1;
    int read_data=-1;
    if(by_len==1)
    {
        send_data.resize(8);
        check_test=id+4+3+(addr & 0x000000ff)+data;
        check_array.resize(1);
        check_array[0] = (uchar)(0x000000ff & check_test);
        check_array[0]=check_array[0]^0xff;
        send_data[0]=(uchar)(0xff);send_data[1]=(uchar)(0xff);
        send_data[2]=(uchar)(0x000000ff & id);
        send_data[3]=(uchar)(0x04);
        send_data[4]=(uchar)(0x03);
        send_data[5]=addr;
        send_data[6]=(uchar)(0x000000ff & data);
        send_data[7]=check_array[0];
    }
    else if(by_len==2)
    {
        send_data.resize(9);
        check_test=id+5+3+(addr & 0x000000ff)+(0x000000ff & data)+((0x0000ff00 & data)>>8);
        check_array.resize(1);
        check_array[0] = (uchar)(0x000000ff & check_test);
        check_array[0]=check_array[0]^0xff;
        send_data[0]=(uchar)(0xff);send_data[1]=(uchar)(0xff);
        send_data[2]=(uchar)(0x000000ff & id);
        send_data[3]=(uchar)(0x05);
        send_data[4]=(uchar)(0x03);
        send_data[5]=addr;
        if(sms==1)
        {
            send_data[6]=(uchar)(0x000000ff & data);
            send_data[7]=(uchar)((0x0000ff00 & data)>>8);
        }
        else if(sms==0)
        {
            send_data[6]=(uchar)((0x0000ff00 & data)>>8);
            send_data[7]=(uchar)(0x000000ff & data);
        }
        send_data[8]=check_array[0];
    }
    if(sms==1)
    {
        serial_port_sms->write(send_data);
        serial_port_sms->waitForBytesWritten(10);
        bool receive_en=true;
        time.restart();
        while (serial_port_sms->bytesAvailable()<6)
        {
            serial_port_sms->waitForReadyRead(10);
            if(time.elapsed()>50){
                receive_en=false;
                break;
            }
        }
        if(receive_en)
        {
            read_data=sms_receivedata(0);
            return read_data;
        }
        else
        {
            serial_port_scs->readAll();
            return -1;
        }
    }
    else if(sms==0)
    {
        serial_port_scs->write(send_data);
        serial_port_scs->waitForBytesWritten(10);
        bool receive_en=true;
        time.restart();
        while (serial_port_scs->bytesAvailable()<6)
        {
            serial_port_scs->waitForReadyRead(10);
            if(time.elapsed()>50){
                receive_en=false;
                break;
            }
        }
        if(receive_en)
        {
            read_data=scs_receivedata(0);
            return read_data;
        }
        else
        {
            serial_port_scs->readAll();
            return -1;
        }
    }
    else return -1;
}

int autoRun::sms_receivedata(int byte_cnt)
{
    QByteArray data_array=serial_port_sms->readAll();
    int data_sum=0;
    int data=0;
    int read_data=-1;
    for(int i=2;i<data_array.size()-1;i++)
    {
        data = data_array[i] & 0x000000FF;
        data_sum=data_sum+data;
    }
    data_sum=0x000000ff & data_sum;
    data_sum=data_sum ^ 0xff;
    data=data_array[data_array.size()-1] & 0x000000FF;
    if(data==data_sum)
    {
        if(byte_cnt==0)
        {
            return 0;
        }
        else if(byte_cnt==1)
        {
            read_data=data_array[data_array.size()-2] & 0x000000FF;
            return read_data;
        }
        else if(byte_cnt==2)
        {
            read_data = data_array[5] & 0x000000FF;
            read_data |= ((data_array[6] << 8) & 0x0000FF00);
            return read_data;
        }
        else return -1;
    }
    else
    {
        return -1;
    }
}

int autoRun::set_move(int duoji_num,int duoji_pos,int duoji_spd,int duoji_time,int sms)
{
    QByteArray send_data;
    int read_data=-1;
    send_data.resize(13);
    int check_test=duoji_num+9+3+42+
         (0x000000ff & duoji_pos)+((0x0000ff00 & duoji_pos)>>8)+
         (0x000000ff & duoji_spd)+((0x0000ff00 & duoji_spd)>>8)+
         (0x000000ff & duoji_time)+((0x0000ff00 & duoji_time)>>8);

    QByteArray duoji_array;
    duoji_array.resize(1);
    duoji_array[0] = (uchar)(0x000000ff & duoji_num);

    QByteArray pos_array;
    pos_array.resize(2);
    pos_array[0] = (uchar)(0x000000ff & duoji_pos);
    pos_array[1] = (uchar)((0x0000ff00 & duoji_pos)>>8);

    QByteArray time_array;
    time_array.resize(2);
    time_array[0] = (uchar)(0x000000ff & duoji_time);
    time_array[1] = (uchar)((0x0000ff00 & duoji_time)>>8);

    QByteArray spd_array;
    spd_array.resize(2);
    spd_array[0] = (uchar)(0x000000ff & duoji_spd);
    spd_array[1] = (uchar)((0x0000ff00 & duoji_spd)>>8);

    QByteArray check_array;
    check_array.resize(1);
    check_array[0] = (uchar)(0x000000ff & check_test);
    check_array[0]=check_array[0]^0xff;

    send_data[0]=(uchar)(0xff);send_data[1]=(uchar)(0xff);
    send_data[2]=duoji_array[0];
    send_data[3]=(uchar)(0x09);
    send_data[4]=(uchar)(0x03);
    send_data[5]=(uchar)(0x2a);
    send_data[12]=check_array[0];
    if(sms==1)
    {
        send_data[6]=pos_array[0];send_data[7]=pos_array[1];
        send_data[8]=time_array[0];send_data[9]=time_array[1];
        send_data[10]=spd_array[0];send_data[11]=spd_array[1];
        serial_port_sms->write(send_data);
        serial_port_sms->waitForBytesWritten(10);
        bool receive_en=true;
        time.restart();
        while (serial_port_sms->bytesAvailable()<6)
        {
            serial_port_sms->waitForReadyRead(10);
            if(time.elapsed()>50){
                receive_en=false;
                break;
            }
        }
        if(receive_en)
        {
            read_data=sms_receivedata(0);
            return read_data;
        }
        else
        {
            serial_port_sms->readAll();
            return -1;
        }
    }
    else if(sms==0)
    {
        send_data[6]=pos_array[0];send_data[7]=pos_array[1];
        send_data[8]=time_array[0];send_data[9]=time_array[1];
        send_data[10]=spd_array[0];send_data[11]=spd_array[1];
        serial_port_scs->write(send_data);
        serial_port_scs->waitForBytesWritten(10);
        bool receive_en=true;
        time.restart();
        while (serial_port_scs->bytesAvailable()<6)
        {
            serial_port_scs->waitForReadyRead(10);
            if(time.elapsed()>50){
                receive_en=false;
                break;
            }
        }
        if(receive_en)
        {
            read_data=scs_receivedata(0);
            return read_data;
        }
        else
        {
            serial_port_scs->readAll();
            return -1;
        }
    }
    else return -1;
}

int autoRun::scs_receivedata(int byte_cnt)
{
    QByteArray data_array=serial_port_scs->readAll();
    int data_sum=0;
    int data=0;
    int read_data=-1;
    for(int i=2;i<data_array.size()-1;i++)
    {
        data = data_array[i] & 0x000000FF;
        data_sum=data_sum+data;
    }
    data_sum=0x000000ff & data_sum;
    data_sum=data_sum ^ 0xff;
    data=data_array[data_array.size()-1] & 0x000000FF;
    if(data==data_sum)
    {
        if(byte_cnt==0)
        {
            return 0;
        }
        else if(byte_cnt==1)
        {
            read_data=data_array[data_array.size()-2] & 0x000000FF;
            return read_data;
        }
        else if(byte_cnt==2)
        {
            read_data = data_array[5] & 0x000000FF;
            read_data |= ((data_array[6] << 8) & 0x0000FF00);
            return read_data;
        }
        else return -1;
    }
    else
    {
        return -1;
    }
}

int autoRun::search_sms_duoji_id(int id,uchar addr,int by_len)
{
    QByteArray send_data;
    QByteArray check_array;
    int read_data=-1;
    int check_test=-1;
    send_data.resize(8);
    check_test=id+4+2+(addr & 0x000000ff)+by_len;
    check_array.resize(1);
    check_array[0] = (uchar)(0x000000ff & check_test);
    check_array[0]=check_array[0]^0xff;

    send_data[0]=(uchar)(0xff);send_data[1]=(uchar)(0xff);
    send_data[2]=(uchar)(0x000000ff & id);
    send_data[3]=(uchar)(0x04);
    send_data[4]=(uchar)(0x02);send_data[5]=addr;
    if(by_len==1) send_data[6]=(uchar)(0x01);
    else if(by_len==2)send_data[6]=(uchar)(0x02);
    send_data[7]=check_array[0];
    serial_port_sms->write(send_data);
    serial_port_sms->waitForBytesWritten(10);
    time.restart();
    bool receive_en=true;
    while (serial_port_sms->bytesAvailable()<6+by_len)
    {
        serial_port_sms->waitForReadyRead(10);
        if(time.elapsed()>50){
            receive_en=false;
            break;
        }
    }
    if(receive_en)
    {
        read_data=sms_receivedata(by_len);
        return read_data;
    }
    else
    {
        serial_port_sms->readAll();
        return -1;
    }
}

int autoRun::search_scs_duoji_id(int id,uchar addr,int by_len)
{
    QByteArray send_data;
    QByteArray check_array;
    int read_data=-1;
    int check_test=-1;
    send_data.resize(8);
    check_test=id+4+2+(addr & 0x000000ff)+by_len;
    check_array.resize(1);
    check_array[0] = (uchar)(0x000000ff & check_test);
    check_array[0]=check_array[0]^0xff;

    send_data[0]=(uchar)(0xff);send_data[1]=(uchar)(0xff);
    send_data[2]=(uchar)(0x000000ff & id);
    send_data[3]=(uchar)(0x04);
    send_data[4]=(uchar)(0x02);send_data[5]=addr;
    if(by_len==1) send_data[6]=(uchar)(0x01);
    else if(by_len==2)send_data[6]=(uchar)(0x02);
    send_data[7]=check_array[0];
    serial_port_scs->write(send_data);
    serial_port_scs->waitForBytesWritten(10);
    bool receive_en=true;
    time.restart();
    while (serial_port_scs->bytesAvailable()<6+by_len)
    {
        serial_port_scs->waitForReadyRead(10);
        if(time.elapsed()>50){
            receive_en=false;
            break;
        }
    }
    if(receive_en)
    {
        read_data=scs_receivedata(by_len);
        return read_data;
    }
    else
    {
        serial_port_scs->readAll();
        return -1;
    }
}
