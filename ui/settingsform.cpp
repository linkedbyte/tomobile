#include "settingsform.h"
#include "ui_settingsform.h"
#include <QMap>
#include <QString>

SettingsForm::SettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsForm)
{
    ui->setupUi(this);
    ui->videoCodecList->addItem("h264");
    ui->videoCodecList->addItem("h265");
    ui->videoCodecList->addItem("av1");

    ui->videoSourceList->addItem("屏幕");
    ui->videoSourceList->addItem("摄像头");

    ui->audioCodecList->addItem("opus");
    ui->audioCodecList->addItem("aac");
    ui->audioCodecList->addItem("raw");

    ui->audioSourceList->addItem("麦克风");
    ui->audioSourceList->addItem("应用输出");

    QStringList videoBits;
    videoBits<<QString::number(704000)
             <<QString::number(896000)
             <<QString::number(1216000)
             <<QString::number(1536000)
             <<QString::number(1856000)
              <<QString::number(5632000);

    ui->videoBitRateList->addItems(videoBits);

    QStringList audioBits;
    audioBits<<QString::number(64000)
              <<QString::number(96000)
              <<QString::number(128000)
              <<QString::number(320000);
    ui->audioBitRateList->addItems(audioBits);

    QStringList frameRates;
    frameRates<<QString::number(25)<<QString::number(30)<<QString::number(60);
    ui->frameRateList->addItems(frameRates);

}

SettingsForm::~SettingsForm()
{
    delete ui;
}
