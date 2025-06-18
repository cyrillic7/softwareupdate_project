/**
 * @File Name: main.cpp
 * @brief  680图像机软件主程序入口，包含机器授权验证和主窗口启动
 * @Author : chency email:121888719@qq.com
 * @Version : 1.0
 * @Creat Date : 2025
 *
 */

#include <QApplication>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QSysInfo>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include "mainwindow.h"
#include "crypto_utils.h"

// 获取机器码
QString getMachineCode()
{
    // 获取CPU信息
    QString cpuInfo = QSysInfo::currentCpuArchitecture();
    
    // 获取机器名
    QString machineName = QSysInfo::machineHostName();
    
    // 获取MAC地址
    QString macAddress;
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
        if (!(interface.flags() & QNetworkInterface::IsLoopBack)) {
            macAddress = interface.hardwareAddress();
            if (!macAddress.isEmpty()) {
                break;
            }
        }
    }
    
    // 组合机器码信息
    QString combinedInfo = QString("%1-%2-%3")
                          .arg(cpuInfo)
                          .arg(machineName)
                          .arg(macAddress);
    
    // 计算MD5哈希作为机器码
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(combinedInfo.toUtf8());
    return hash.result().toHex().toUpper();
}

// 检查机器授权
bool checkMachineAuthorization()
{
    QString currentMachineCode = getMachineCode();
    QString authFile = QApplication::applicationDirPath() + "/machine_auth.key";
    
    // 如果授权文件不存在，拒绝访问
    if (!QFile::exists(authFile)) {
        return false;
    }
    
    // 读取授权文件中的加密数据
    QFile file(authFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString encryptedData = stream.readAll().trimmed();
    file.close();
    
    // 解密授权文件数据
    QString authorizedMachineCode = CryptoUtils::decryptMachineCode(encryptedData);
    
    // 验证机器码是否匹配
    return (currentMachineCode == authorizedMachineCode);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 启动时检查机器授权
    if (!checkMachineAuthorization()) {
        QString machineCode = getMachineCode();
        QMessageBox::critical(nullptr, "权限验证失败", 
            QString("抱歉，您没有权限使用此软件。\n\n"
                   "请按以下步骤操作：\n"
                   "1. 运行 680_AuthTool.exe 授权工具\n"
                   "2. 复制当前机器码：%1\n"
                   "3. 生成授权文件 machine_auth.key\n"
                   "4. 将授权文件放置在软件目录下\n\n"
                   "或联系软件提供商获取授权文件。").arg(machineCode));
        return 1;  // 直接退出，返回错误码
    }
    
    // 加载QSS样式文件
    QFile file(":/styles/main.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        app.setStyleSheet(styleSheet);
        file.close();
    }
    
    MainWindow window;
    window.show();
    
    return app.exec();
} 