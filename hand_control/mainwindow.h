#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <math.h>
#include<QByteArray>
#include<QList>
#include<QFile>
#include<QTextStream>
#include<QMessageBox>
#include<QStringList>
#include"autorun.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    autoRun *autorun_obj;
    void open_serialport();
    void init_param();
    void init_tablewidget();
    bool set_shift_value;
    int ideal_pos,current_pos,current_spd,current_load,current_voltage,current_temp,current_state,move_state;
    QList<QString>sms_addr_explain={"固件主版本号","固件次版本号","舵机主版本号","舵机次版本号","ID","波特率","返回延迟时间","应答状态级别",
                               "最小角度限制","最大角度限制","最高温度上限","最高输入电压","最低输入电压","最大扭矩","卸载条件","LED报警条件",
                               "P比例系数","D微分系数","I积分系数","最小启动力","顺时针不灵敏区","逆时针不灵敏区","保护电流","角度分辨率",
                               "位置校正","运行模式","保护扭力","保护时间","过载扭力","KFF1st系数","扭矩开关","加速度",
                               "目标位置","运行速度","转矩限制","锁标志","当前位置","当前速度","当前负载","当前电压",
                               "当前温度","移动标志","异步写标志"};
    QList<uchar>sms_reg_addr={uchar(0x00),uchar(0x01),uchar(0x03),uchar(0x04),uchar(0x05),uchar(0x06),uchar(0x07),uchar(0x08),
                             uchar(0x09),uchar(0x0b),uchar(0x0d),uchar(0x0e),uchar(0x0f),uchar(0x10),uchar(0x13),uchar(0x14),
                             uchar(0x15),uchar(0x16),uchar(0x17),uchar(0x18),uchar(0x1a),uchar(0x1b),uchar(0x1c),uchar(0x1e),
                             uchar(0x1f),uchar(0x21),uchar(0x22),uchar(0x23),uchar(0x24),uchar(0x25),uchar(0x28),uchar(0x29),
                             uchar(0x2a),uchar(0x2e),uchar(0x30),uchar(0x37),uchar(0x38),uchar(0x3a),uchar(0x3c),uchar(0x3e),
                             uchar(0x3f),uchar(0x42),uchar(0x45)};
    QList<int>sms_addr_num={1,1,1,1,1,1,1,1,
                             2,2,1,1,1,2,1,1,
                             1,1,1,2,1,1,2,1,
                             2,1,1,1,1,1,1,1,
                             2,2,2,1,2,2,2,1,
                             1,1,1};
    QList<QString>scs_addr_explain={"固件主版本号","固件次版本号","舵机主版本号","舵机次版本号","ID","波特率","返回延迟时间","应答状态级别",
                               "最小角度限制","最大角度限制","最高温度上限","最高输入电压","最低输入电压","最大扭矩","高压标志位","卸载条件",
                               "LED报警条件","P比例系数","D微分系数","I积分系数","最小PWM","顺时针不灵敏区","逆时针不灵敏区","积分限制",
                               "运行模式","保护扭力","保护时间","过载扭力","扭矩开关","目标位置","运行时间","运行速度","锁标志",
                               "当前位置","当前速度","当前负载","当前电压","当前温度","异步写标志"};

    QList<uchar>scs_reg_addr={uchar(0x00),uchar(0x01),uchar(0x03),uchar(0x04),uchar(0x05),uchar(0x06),uchar(0x07),uchar(0x08),
                              uchar(0x09),uchar(0x0b),uchar(0x0d),uchar(0x0e),uchar(0x0f),uchar(0x10),uchar(0x12),uchar(0x013),
                             uchar(0x14),uchar(0x15),uchar(0x16),uchar(0x17),uchar(0x18),uchar(0x1a),uchar(0x1b),uchar(0x1c),

                              uchar(0x21),uchar(0x25),uchar(0x26),uchar(0x27),uchar(0x28),uchar(0x2a),uchar(0x2c),uchar(0x2e),uchar(0x37),
                              uchar(0x38),uchar(0x3a),uchar(0x3c),uchar(0x3e),uchar(0x3f),uchar(0x40)};

//                             uchar(0x1e),uchar(0x28),uchar(0x2a),uchar(0x2c),uchar(0x2e),uchar(0x30),uchar(0x38),uchar(0x3a),
//                             uchar(0x3c),uchar(0x3e),uchar(0x3f),uchar(0x40)};
    QList<int>scs_addr_num={1,1,1,1,1,1,1,1,
                            2,2,1,1,1,2,1,1,
                             1,1,1,1,2,1,1,1,
                             1,1,1,1,2,2,2,1,1,
                             2,2,2,1,1,1};
private slots:
    void update_control_fun(int flag = 0);

    void on_start_move_clicked();

    void on_clear_text_clicked();

    void on_shift_pos_valueChanged(int value);

    void on_act_save_file_triggered();

    void on_search_id_clicked();

    void on_listWidget_clicked(const QModelIndex &index);

    void on_act_exit_triggered();

    void on_update_pos_clicked();

    void on_auto_start_clicked();

    void on_add_pos_clicked();

    void on_delete_pos_clicked();

    void on_text_to_widget_clicked();

    void on_widget_to_text_clicked();

    void on_motion_serial_clicked();

    void on_read_par_clicked();

    void on_write_par_clicked();

private:
    QThread *autorun_thread;
    const uchar reg_ideal_pos = 0x2a;
    const uchar reg_current_pos = 0x38;
    const uchar reg_current_spd = 0x3a;
    const uchar reg_current_load = 0x3c;
    const uchar reg_current_voltage = 0x3e;
    const uchar reg_current_temp = 0x3f;
    const uchar reg_current_state = 0x41;
    const uchar reg_move_state = 0x42;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
