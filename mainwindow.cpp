#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;

    //connect(this, SIGNAL(dataIsAvaliableSignal(int,int)), this, SLOT(receiveMeasData(int, int)));

}

void MainWindow::on_init_Button_clicked()
{
    unsigned initNumber = AVS_Init(0);
    if (initNumber == 0)
    {
        ui->plainTextEdit->appendPlainText("No devices found");
        return;
    }
    ui->plainTextEdit->appendPlainText("Number of devices found: " + QString::number(initNumber));
    on_update_Button_2_clicked();
    getListOfDevices();
}


void MainWindow::on_update_Button_2_clicked()
{
    auto number = AVS_UpdateUSBDevices();
    qDebug() <<  number;
}

void MainWindow::getListOfDevices()
{
    unsigned int         l_Size = 0;
    unsigned int         l_RequiredSize = 0;
    int                  l_NrDevices;
    char*                l_pDataDiscoDevs = NULL; // Data pointer for list of discovered devices
    char*				 l_pDataIds = NULL; // Data pointer for list of IDs
    AvsIdentityType*     l_pId;

    l_NrDevices = AVS_GetList(0, &l_RequiredSize, NULL );
    l_pDataIds = new char[l_RequiredSize];
    l_pId = (AvsIdentityType*)l_pDataIds;
    l_Size = l_RequiredSize;
    l_NrDevices = AVS_GetList(l_Size, &l_RequiredSize, l_pId);

    for (int i = 0; i < l_NrDevices; i++)
    {
        QString deviceInfo = QString(l_pId->SerialNumber);
        ui->listWidget->addItem(deviceInfo);
        ui->plainTextEdit->appendPlainText(QString("Device #%1: ").arg(i+1) + l_pId->UserFriendlyName + ", " + l_pId->SerialNumber);
    }

    delete[] l_pDataDiscoDevs;
    delete[] l_pDataIds;
}


void MainWindow::on_connect_Button_clicked()
{
    if (ui->listWidget->selectedItems().isEmpty()) return;
    if (ui->listWidget->selectedItems().size() > 1) return;

    AvsIdentityType l_Active;
    l_Active.UserFriendlyName[0] = 0;
    AvsHandle l_hDevice;

    strcpy(l_Active.SerialNumber, ui->listWidget->selectedItems().at(0)->text().toLocal8Bit().data());
    l_Active.SerialNumber[AVS_SERIAL_LEN - 1] = 0;
    l_hDevice = AVS_Activate(&l_Active);

    if (INVALID_AVS_HANDLE_VALUE == l_hDevice)
    {
        ui->plainTextEdit->appendPlainText(QString("Device %1 wasn't connected").arg(l_Active.SerialNumber));
        return;
    }

    ui->plainTextEdit->appendPlainText(QString("Device %1 has been connected!").arg(l_Active.SerialNumber));
    m_DeviceHandle = l_hDevice;
    m_Identity = l_Active;

    DeviceConfigType  l_DeviceData;
    unsigned int       l_Size;
    l_Size = sizeof(DeviceConfigType);
    int l_Res = AVS_GetParameter(m_DeviceHandle, l_Size, &l_Size, &l_DeviceData);
    if (ERR_SUCCESS != l_Res)
    {
        ui->plainTextEdit->appendPlainText(QString("AVS_GetParameter failed (code %1)").arg(l_Res));
        return;
    }

    mAckDisabled = l_DeviceData.m_MessageAckDisable;
    mIncludeCRC = l_DeviceData.m_IncludeCRC;

    qDebug() << "mIncludeCRC" << mIncludeCRC;
}


void MainWindow::on_singleMeasure_Button_clicked()
{
    short l_NrOfScans = ui->measurments_spinBox->value();
    int measureTime = ui->intergationTime_spinBox->value();
    int numberOfAverages = ui->averages_spinBox->value();
    int dataReceiverDelay = measureTime * numberOfAverages * l_NrOfScans + 100;

    prepareMeasure();
    measure(l_NrOfScans, dataReceiverDelay);


}

void MainWindow::on_pushButton_clicked()
{
    prepareMeasure();
    getLambdas();
}

void MainWindow::getLambdas()
{
    int getLambdasRes = AVS_GetLambda(m_DeviceHandle, lambdas);
    if (getLambdasRes == ERR_SUCCESS)
        ui->plainTextEdit->appendPlainText(QString("Lambda values (%1 points) has been obtained!").arg(sizeof(lambdas)/sizeof(double)));
    else
        ui->plainTextEdit->appendPlainText("Error occured while lambda values obtaining" + QString(" (%1 code)").arg(getLambdasRes));

    ui->data_plainTextEdit->clear();
    //ui->data_plainTextEdit->appendPlainText("Lambda values:");
    for (int i = 0; i < sizeof(lambdas)/sizeof(double); i++)
        ui->data_plainTextEdit->appendPlainText(QString::number(lambdas[i]));
}

void MainWindow::callbackFunction(AvsHandle *handle, int *result)
{
}

void MainWindow::receiveMeasData()
{
    //unsigned long endTime = std::clock();
    //qDebug() << endTime - startTime;
    unsigned short avg;
    uint32 crc;
    uint16 nrPixels;
    uint32 tempSpectrum[NUMBER_OF_PYXELS];
    int l_Res = 0;
    unsigned timeLabel;

    if (mIncludeCRC)
    {
        l_Res = AVS_GetRawScopeDataCRC(m_DeviceHandle, &m_Time, &avg, &crc, &nrPixels, tempSpectrum);
        if (l_Res != ERR_SUCCESS)
        {
            ui->plainTextEdit->appendPlainText("Error while getting data");
            return;
        }
        for (int i = 0; i < NUMBER_OF_PYXELS; i++) {
            specData[i] = tempSpectrum[i] / avg;  // avg cannot be 0, tested in AVS_PrepareMeasure
        }
    }
    else
    {
        l_Res = AVS_GetScopeData(m_DeviceHandle, &timeLabel, specData);
        if (l_Res != ERR_SUCCESS)
        {
            ui->plainTextEdit->appendPlainText("Error while getting data");
            return;
        }
    }
    QFile file(QString("D:\\Avaspec\\%1.txt").arg(timeLabel));
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream out(&file);
        out << QString::number(ui->integrationDelay_spinBox->value()) << endl;
        for (int i = 0; i < NUMBER_OF_PYXELS; i++)
            out << QString::number(specData[i]) << endl;

    }
    file.close();

//    ui->data_plainTextEdit->clear();
//    for (int i = 0; i < NUMBER_OF_PYXELS; i++)
//    {
//        QString str = QString::number(specData[i]);
//        ui->data_plainTextEdit->appendPlainText(str);
//    }
}

void MainWindow::prepareMeasure()
{
    int l_Res = AVS_UseHighResAdc(m_DeviceHandle, 1);
    if  (l_Res != ERR_SUCCESS)
    {
        ui->plainTextEdit->appendPlainText(QString("AVS_UseHighResAdc failed (code %1)").arg(l_Res));
        return;
    }

    MeasConfigType l_PrepareMeasData;
    l_PrepareMeasData.m_StartPixel = 0;
    l_PrepareMeasData.m_StopPixel = NUMBER_OF_PYXELS-1;
    QLocale::setDefault(QLocale::Dutch);
    l_PrepareMeasData.m_IntegrationTime = ui->intergationTime_spinBox->value();
    l_PrepareMeasData.m_IntegrationDelay = ui->integrationDelay_spinBox->value();
    l_PrepareMeasData.m_NrAverages = ui->averages_spinBox->value();
    l_PrepareMeasData.m_CorDynDark.m_Enable = 0;
    l_PrepareMeasData.m_CorDynDark.m_ForgetPercentage = 0;
    l_PrepareMeasData.m_Smoothing.m_SmoothPix = 0;
    l_PrepareMeasData.m_Smoothing.m_SmoothModel = 0;
    l_PrepareMeasData.m_SaturationDetection = 0;
    l_PrepareMeasData.m_Trigger.m_Mode = 0;
    l_PrepareMeasData.m_Trigger.m_Source = 0;
    l_PrepareMeasData.m_Trigger.m_SourceType = 0;
    l_PrepareMeasData.m_Control.m_StrobeControl = 0;
    l_PrepareMeasData.m_Control.m_LaserDelay = 0;
    l_PrepareMeasData.m_Control.m_LaserWidth = 0;
    l_PrepareMeasData.m_Control.m_LaserWaveLength = 0;
    l_PrepareMeasData.m_Control.m_StoreToRam = 0;

    l_Res = AVS_UseHighResAdc(m_DeviceHandle,1);
    if (ERR_SUCCESS != l_Res)
    {
        ui->plainTextEdit->appendPlainText("AVS_UseHighResAdc Error!");
        return;
    }

    l_Res = AVS_PrepareMeasure(m_DeviceHandle, &l_PrepareMeasData);
    if (ERR_SUCCESS != l_Res)
    {
        ui->plainTextEdit->appendPlainText("AVS_PrepareMeasure Error!");
        return;
    }

    //ui->plainTextEdit->appendPlainText("Detector has been configured successfully");
}

void MainWindow::measure(int numberOfMeasures, int dataReceiverDelay)
{
    int l_Res = AVS_Measure(m_DeviceHandle, 0, numberOfMeasures);
    QTimer::singleShot(dataReceiverDelay, this, &MainWindow::receiveMeasData);
    //startTime = std::clock();

    if (l_Res != ERR_SUCCESS)
    {
        ui->plainTextEdit->appendPlainText("Error while measuring");
        return;
    }
}

void MainWindow::ledOff()
{
    AVS_SetDigOut(m_DeviceHandle, 0, 0);
    qDebug() << "pin 11 off";
}

void MainWindow::ledOn()
{
    AVS_SetDigOut(m_DeviceHandle, 0, 1);
    qDebug() << "pin 11 on";
}

void MainWindow::on_LED_pushButton_clicked()
{
    if (ui->LED_pushButton->isChecked())
    {
        AVS_SetDigOut(m_DeviceHandle, 0, 1);
    }
    else
    {
        AVS_SetDigOut(m_DeviceHandle, 0, 0);
    }
}


void MainWindow::on_ledOff_measure_pushButton_clicked()
{
    short l_NrOfScans = ui->measurments_spinBox->value();
    int measureTime = ui->intergationTime_spinBox->value();
    int numberOfAverages = ui->averages_spinBox->value();
    int dataReceiverDelay = measureTime * numberOfAverages * l_NrOfScans + 100;
    int ledOffDelay = ui->ledOff_delay_spinBox->value();

    prepareMeasure();

    QElapsedTimer timer;
    timer.start();
    QTimer::singleShot(ledOffDelay, this, &MainWindow::ledOff);
    //ledOff();
    measure(l_NrOfScans, dataReceiverDelay);
    qDebug() << "The foo() function took" << timer.nsecsElapsed() << "nanoseconds";
}


void MainWindow::on_program1_pushButton_clicked()
{
    warmUp(500);
    ledOn();

    int integrationDelay = 25000;
    const int delayIncrement = 10; // ticks
    //const int ledOffDelay = 1;
    int repeatCounter = 0;
    const int repeatNumber = 5;

    unsigned long timer = std::clock();
    const int deltaTime = 150; //milliseconds
    unsigned timeLabel = 0;


    QElapsedTimer measureTimer;
    QElapsedTimer highPinTimer;
    int ledOffDelay = 0;
    int highPinDelay = 0;
    measureTimer.start();
    highPinTimer.start();

    while(integrationDelay < 75000)
    {
        if (std::clock() - timer > deltaTime)
        {
            timer = std::clock();
            prepareMeasure();

            AVS_Measure(m_DeviceHandle, 0, 1);
            AVS_SetDigOut(m_DeviceHandle, 0, 0); // led off

            QThread::msleep(60);

            AVS_GetScopeData(m_DeviceHandle, &timeLabel, specData);
            AVS_SetDigOut(m_DeviceHandle, 0, 1); // led on
            //ledOn();

            QFile file(QString("D:\\Avaspec\\%1_%2.txt").arg(integrationDelay).arg(repeatCounter));
            if (file.open(QIODevice::WriteOnly))
            {
                QTextStream out(&file);
                out << QString::number(ui->integrationDelay_spinBox->value()) << Qt::endl;
                // out << QString::number(ledOffDelay) << Qt::endl;
                // out << QString::number(highPinDelay) << Qt::endl;
                for (int i = 0; i < NUMBER_OF_PYXELS; i++)
                    out << QString::number(specData[i]) << Qt::endl;
            }
            file.close();
            //QTimer::singleShot(150, this, &MainWindow::ledOn);

            qDebug() << std::clock() - timer;

            if (repeatCounter == repeatNumber - 1 )
            {
                integrationDelay += delayIncrement;
                ui->integrationDelay_spinBox->setValue(integrationDelay);
                repeatCounter = 0;
            }
            else
                repeatCounter++;
        }
    }

    AVS_SetDigOut(m_DeviceHandle, 0, 0); // led off
}

void MainWindow::warmUp(const int numberOfCycles)
{
    int i = 0;
    while (i < numberOfCycles)
    {
        AVS_SetDigOut(m_DeviceHandle, 0, 0); // led off
        QThread::msleep(100);
        AVS_SetDigOut(m_DeviceHandle, 0, 1); // led on
        QThread::msleep(300);
        i++;
    }
}


void MainWindow::on_program2_pushButton_clicked() // Phosphorescence
{
    const int initialIntegrationDelay = ui->initialDelay_spinBox->value(); // ms
    const int finalIntegrationDelay = ui->finalDelay_spinBox->value(); // ms
    int integrationDelay = initialIntegrationDelay; //ms
    const int delayIncrement = ui->delayIncrement_spinBox->value(); // ms
    const int irradiationTime = ui->exitationTime_spinBox_phos->value(); // ms

    int repeatCounter = 0;
    int filesCounter = 0;
    const int repeatNumber = ui->repeatNumber_spinBox->value();

    const int integrationTime = ui->integrationTime_spinBox_phos->value(); // ms

    unsigned timeLabel = 0;
    ui->intergationTime_spinBox->setValue(integrationTime);

    while(integrationDelay <= finalIntegrationDelay)
    {
        prepareMeasure();

        AVS_SetDigOut(m_DeviceHandle, 0, 1); // led on
        QThread::msleep(irradiationTime);

        AVS_SetDigOut(m_DeviceHandle, 0, 0); // led off
        QThread::msleep(integrationDelay);
        AVS_Measure(m_DeviceHandle, 0, 1);

        QThread::msleep(integrationTime + 50);
        AVS_GetScopeData(m_DeviceHandle, &timeLabel, specData);

        QFile file(QString("D:\\Avaspec\\phosphorescence\\%1_%2.txt").arg(integrationDelay).arg(repeatCounter));
        if (file.open(QIODevice::WriteOnly))
        {
            QTextStream out(&file);
            out << integrationDelay << Qt::endl;
            for (int i = 0; i < NUMBER_OF_PYXELS; i++)
                out << QString::number(specData[i]) << Qt::endl;
        }
        file.close();
        filesCounter++;

        if (repeatCounter == repeatNumber - 1)
        {
            integrationDelay += delayIncrement;
            repeatCounter = 0;
        }
        else
            repeatCounter++;

        QThread::msleep(50);
    }

    ui->plainTextEdit->appendPlainText(QString("\n* \"Phosphorescence lifetime\" subprogram finished. %1 files has been created in D:\\Avaspec\\phosphorescence\\").arg(filesCounter));

    AVS_SetDigOut(m_DeviceHandle, 0, 0); // led off
}

