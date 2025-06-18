/**
 * @File Name: crypto_utils.cpp
 * @brief  680图像机软件加密工具实现，使用数字矩阵、字母罗盘和私钥混合的三重加密算法
 * @Author : chency email:121888719@qq.com
 * @Version : 1.0
 * @Creat Date : 2025
 *
 */

#include "crypto_utils.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>

// 私钥定义
const QString CryptoUtils::PRIVATE_KEY = "ChencyUe2025";

QString CryptoUtils::encryptMachineCode(const QString &machineCode)
{
    if (machineCode.isEmpty()) {
        return QString();
    }
    
    // 转换为字节数组
    QByteArray data = machineCode.toUtf8();
    
    // 第一步：私钥混合
    data = mixWithPrivateKey(data, true);
    
    // 第二步：字母罗盘变换
    data = applyLetterCompass(data, true);
    
    // 第三步：数字矩阵变换
    data = applyDigitalMatrix(data, true);
    
    // 转换为十六进制字符串
    return toHexString(data);
}

QString CryptoUtils::decryptMachineCode(const QString &encryptedData)
{
    if (encryptedData.isEmpty()) {
        return QString();
    }
    
    // 从十六进制字符串转换
    QByteArray data = fromHexString(encryptedData);
    if (data.isEmpty()) {
        return QString();
    }
    
    // 第一步：数字矩阵逆变换
    data = applyDigitalMatrix(data, false);
    
    // 第二步：字母罗盘逆变换
    data = applyLetterCompass(data, false);
    
    // 第三步：私钥混合逆变换
    data = mixWithPrivateKey(data, false);
    
    return QString::fromUtf8(data);
}

QByteArray CryptoUtils::applyDigitalMatrix(const QByteArray &data, bool encrypt)
{
    QByteArray result = data;
    
    // 简化的数字矩阵变换 - 使用位运算和移位，确保可逆
    for (int i = 0; i < result.size(); i++) {
        unsigned char byte = result[i];
        
        if (encrypt) {
            // 加密：按位循环左移3位，然后异或位置索引
            byte = ((byte << 3) | (byte >> 5)) ^ (i % 256);
        } else {
            // 解密：先异或位置索引，然后按位循环右移3位
            byte = byte ^ (i % 256);
            byte = ((byte >> 3) | (byte << 5));
        }
        
        result[i] = byte;
    }
    
    return result;
}

QByteArray CryptoUtils::applyLetterCompass(const QByteArray &data, bool encrypt)
{
    QByteArray result = data;
    
    // 双圈字母罗盘设计（简化版本，确保完全可逆）
    // 内圈大小: 13，外圈大小: 13
    const int INNER_SIZE = 13;
    const int OUTER_SIZE = 13;
    
    // 生成基于私钥的随机位移序列
    QList<int> shifts = generateRandomSequence(result.size(), PRIVATE_KEY);
    
    for (int i = 0; i < result.size(); i++) {
        unsigned char byte = result[i];
        
        // 根据位置确定使用内圈还是外圈（避免依赖字节值，确保加密解密一致）
        bool useInnerCompass = (i % 2 == 0);
        
        // 选择罗盘参数
        int compassSize = useInnerCompass ? INNER_SIZE : OUTER_SIZE;
        int baseShift = useInnerCompass ? 7 : 11;
        
        // 计算位移量
        int shift = shifts[i % shifts.size()] % compassSize;
        int totalShift = (shift + baseShift) % compassSize;
        
        if (encrypt) {
            // 加密：双圈罗盘变换
            int transformed = (byte + totalShift) % 256;
            
            // 内外圈交互：简单异或操作，完全可逆
            if (useInnerCompass) {
                transformed = transformed ^ (i % 7);  // 内圈异或
            } else {
                transformed = transformed ^ (i % 11); // 外圈异或
            }
            
            result[i] = (unsigned char)transformed;
        } else {
            // 解密：严格按加密的逆序操作
            int transformed = byte;
            
            // 先逆向内外圈交互
            if (useInnerCompass) {
                transformed = transformed ^ (i % 7);  // 逆向内圈异或
            } else {
                transformed = transformed ^ (i % 11); // 逆向外圈异或
            }
            
            // 然后逆向罗盘变换
            result[i] = (unsigned char)((transformed - totalShift + 256) % 256);
        }
    }
    
    return result;
}

QByteArray CryptoUtils::mixWithPrivateKey(const QByteArray &data, bool encrypt)
{
    QByteArray result = data;
    QByteArray keyBytes = PRIVATE_KEY.toUtf8();
    
    // 计算私钥的MD5哈希作为混合密钥
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(keyBytes);
    QByteArray keyHash = hash.result();
    
    for (int i = 0; i < result.size(); i++) {
        unsigned char dataByte = result[i];
        unsigned char keyByte = keyHash[i % keyHash.size()];
        
        if (encrypt) {
            // 加密：异或后再加法混合
            result[i] = ((dataByte ^ keyByte) + keyByte) % 256;
        } else {
            // 解密：减法混合后再异或
            int temp = (dataByte - keyByte + 256) % 256;
            result[i] = temp ^ keyByte;
        }
    }
    
    return result;
}

QList<int> CryptoUtils::generateRandomSequence(int length, const QString &seed)
{
    QList<int> sequence;
    
    // 使用私钥和当前数据生成伪随机序列
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(seed.toUtf8());
    QByteArray hashBytes = hash.result();
    
    for (int i = 0; i < length; i++) {
        // 使用哈希字节生成伪随机数
        int index = i % hashBytes.size();
        int value = ((unsigned char)hashBytes[index] + i) % 100;
        sequence.append(value);
    }
    
    return sequence;
}

QString CryptoUtils::toHexString(const QByteArray &data)
{
    return data.toHex().toUpper();
}

QByteArray CryptoUtils::fromHexString(const QString &hexStr)
{
    return QByteArray::fromHex(hexStr.toUtf8());
} 