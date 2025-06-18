/**
 * @File Name: test_double_compass.cpp
 * @brief  测试双圈罗盘加密算法的加密解密功能
 * @Author : chency email:121888719@qq.com
 * @Version : 1.0
 * @Creat Date : 2025
 *
 */

#include <QCoreApplication>
#include <QDebug>
#include <QSysInfo>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QFile>
#include <QTextStream>
#include "crypto_utils.h"

// 获取机器码
QString getMachineCode()
{
    QString cpuInfo = QSysInfo::currentCpuArchitecture();
    QString machineName = QSysInfo::machineHostName();
    
    QString macAddress;
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
        if (!(interface.flags() & QNetworkInterface::IsLoopBack)) {
            macAddress = interface.hardwareAddress();
            if (!macAddress.isEmpty()) {
                break;
            }
        }
    }
    
    QString combinedInfo = QString("%1-%2-%3")
                          .arg(cpuInfo)
                          .arg(machineName)
                          .arg(macAddress);
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(combinedInfo.toUtf8());
    return hash.result().toHex().toUpper();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "========================================";
    qDebug() << "双圈罗盘加密算法测试";
    qDebug() << "========================================";
    
    // 获取当前机器码
    QString originalMachineCode = getMachineCode();
    qDebug() << "Original Machine Code:" << originalMachineCode;
    qDebug() << "Machine Code Length:" << originalMachineCode.length();
    
    // 测试多次加密解密
    bool allTestsPassed = true;
    for (int test = 1; test <= 5; test++) {
        qDebug() << "\n--- Test Round" << test << "---";
        
        // 加密机器码
        QString encryptedData = CryptoUtils::encryptMachineCode(originalMachineCode);
        qDebug() << "Encrypted Data:" << encryptedData;
        qDebug() << "Encrypted Length:" << encryptedData.length();
        
        // 解密数据
        QString decryptedMachineCode = CryptoUtils::decryptMachineCode(encryptedData);
        qDebug() << "Decrypted Machine Code:" << decryptedMachineCode;
        
        // 验证是否匹配
        bool isMatch = (originalMachineCode == decryptedMachineCode);
        qDebug() << "Test Result:" << (isMatch ? "PASS" : "FAIL");
        
        if (!isMatch) {
            allTestsPassed = false;
            qDebug() << "ERROR: Decryption failed!";
            qDebug() << "Expected:" << originalMachineCode;
            qDebug() << "Got:     " << decryptedMachineCode;
        }
    }
    
    qDebug() << "\n========================================";
    qDebug() << "Overall Test Result:" << (allTestsPassed ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    qDebug() << "========================================";
    
    if (allTestsPassed) {
        // 自动生成授权文件
        QString encryptedData = CryptoUtils::encryptMachineCode(originalMachineCode);
        QFile file("machine_auth.key");
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            stream << encryptedData;
            file.close();
            qDebug() << "Authorization file created: machine_auth.key";
            qDebug() << "File contains encrypted data:" << encryptedData;
        } else {
            qDebug() << "Failed to create authorization file!";
        }
    }
    
    return 0;
} 