#ifndef AUTORUN_H
#define AUTORUN_H
#include <QObject>
#include <QGraphicsView>
#include <math.h>
#include<QByteArray>
#include<QList>
#include "QtSerialPort/qserialport.h"
#include<QThread>
#include<QFile>
#include<QTextStream>
#include<QMessageBox>
#include<QStringList>
#include<QTime>
using namespace std;
class autoRun : public QObject
{
    Q_OBJECT
public:
    explicit autoRun(QObject *parent = 0);
    ~autoRun();
    bool run_var;
    bool auto_start;
    bool run_loop;
    bool run_serial;
    bool scan_id;
    bool scan_stop;
    int duoji_select_id;
    bool send_busy;
    int sms;
    int move_row;
    int delay_time;
    int duoji_wrong;
    QString duoji_list;
    QList<int>pos_list;
    QString motion_cmd,motion_time,motion_spd,motion_point;
    QSerialPort *serial_port_sms;
    QSerialPort *serial_port_scs;
    int set_move(int duoji_num, int duoji_pos, int duoji_spd, int duoji_time, int sms);
    int read_bytes(int id, uchar addr, int by_len, int sms);
    int write_bytes(int id, uchar addr, int data, int by_len, int sms);
    int search_sms_duoji_id(int id,uchar addr,int by_len);
    int search_scs_duoji_id(int id,uchar addr,int by_len);

    int duoji_begin_pos=101;
    int duoji_cnt=8;
    int sms_duoji_pos[2]={6,7};
signals:
    void update_control_signal(int);

public slots:
    void mainloop();

    void Mainloop_stop();

    int sms_receivedata(int byte_cnt);

    int scs_receivedata(int byte_cnt);
private:
    QTime time;

};

#endif // AUTORUN_H
