/**
 * @File Name: crypto_utils.h
 * @brief  680图像机软件加密工具类头文件，实现数字矩阵加字母罗盘随机算法
 * @Author : chency email:121888719@qq.com
 * @Version : 1.0
 * @Creat Date : 2025
 *
 */

#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <QString>
#include <QByteArray>

class CryptoUtils
{
public:
    // 加密函数 - 使用数字矩阵加字母罗盘随机算法
    static QString encryptMachineCode(const QString &machineCode);
    
    // 解密函数
    static QString decryptMachineCode(const QString &encryptedData);
    
private:
    // 私钥
    static const QString PRIVATE_KEY;
    
    // 数字矩阵变换
    static QByteArray applyDigitalMatrix(const QByteArray &data, bool encrypt = true);
    
    // 字母罗盘变换
    static QByteArray applyLetterCompass(const QByteArray &data, bool encrypt = true);
    
    // 私钥混合
    static QByteArray mixWithPrivateKey(const QByteArray &data, bool encrypt = true);
    
    // 生成伪随机序列
    static QList<int> generateRandomSequence(int length, const QString &seed);
    
    // 十六进制转换
    static QString toHexString(const QByteArray &data);
    static QByteArray fromHexString(const QString &hexStr);
};

#endif // CRYPTO_UTILS_H 