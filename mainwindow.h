#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define NUMBER_OF_PYXELS 1900

#include <QMainWindow>
#include <QDebug>
#include <QTimer>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <QThread>

#include "avaspec.h"
#include <chrono>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_init_Button_clicked();

    void on_update_Button_2_clicked();
    void getListOfDevices();

    void on_connect_Button_clicked();

    void on_singleMeasure_Button_clicked();

    void on_pushButton_clicked();
    void getLambdas();
    static void callbackFunction(AvsHandle *handle, int *result);
    void receiveMeasData();
    void prepareMeasure();
    void measure(int numberOfMeasures, int dataReceiverDelay);
    void ledOff();
    void ledOn();

    void on_LED_pushButton_clicked();

    void on_ledOff_measure_pushButton_clicked();

    void on_program1_pushButton_clicked();
    void warmUp(const int numberOfCycles);

    void on_program2_pushButton_clicked();

signals:
    void dataIsAvaliableSignal(int, int);

private:
    long m_DeviceHandle;
    AvsIdentityType m_Identity;
    double lambdas[NUMBER_OF_PYXELS] = {0};
    double specData[NUMBER_OF_PYXELS] = {0};
    unsigned int m_Time;

    unsigned long startTime;

    bool mIncludeCRC;
    bool mAckDisabled;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
