#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
SSH连接测试脚本
使用paramiko库直接处理SSH连接，支持密码和密钥认证
"""

import sys
import os
import argparse
import paramiko
import socket
from pathlib import Path

def test_ssh_connection(hostname, port, username, password=None, key_file=None):
    """
    测试SSH连接
    
    Args:
        hostname: 服务器IP地址
        port: SSH端口
        username: 用户名
        password: 密码（可选）
        key_file: SSH私钥文件路径（可选）
    
    Returns:
        bool: 连接是否成功
    """
    try:
        print(f"[INFO] 正在测试连接到 {username}@{hostname}:{port}")
        
        # 创建SSH客户端
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        
        auth_methods = []
        
        # 尝试不同的认证方法
        auth_success = False
        
        # 方法1: 密钥认证（如果提供了密钥文件）
        if key_file and os.path.exists(key_file):
            try:
                print(f"[INFO] 尝试SSH密钥认证...")
                ssh.connect(
                    hostname=hostname,
                    port=int(port),
                    username=username,
                    key_filename=key_file,
                    timeout=10,
                    allow_agent=False,
                    look_for_keys=False
                )
                auth_success = True
                auth_methods.append("公钥认证")
                print("[SUCCESS] SSH密钥认证成功")
            except paramiko.AuthenticationException as e:
                print(f"[INFO] SSH密钥认证失败: {str(e)}")
            except Exception as e:
                print(f"[WARNING] SSH密钥认证出错: {str(e)}")
        
        # 方法2: 密码认证（如果密钥认证失败且提供了密码）
        if not auth_success and password:
            try:
                print(f"[INFO] 尝试密码认证...")
                if auth_success:  # 如果之前的连接还在，先关闭
                    ssh.close()
                    ssh = paramiko.SSHClient()
                    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
                
                ssh.connect(
                    hostname=hostname,
                    port=int(port),
                    username=username,
                    password=password,
                    timeout=10,
                    allow_agent=False,
                    look_for_keys=False
                )
                auth_success = True
                auth_methods.append("密码认证")
                print("[SUCCESS] 密码认证成功")
            except paramiko.AuthenticationException as e:
                print(f"[ERROR] 密码认证失败: {str(e)}")
            except Exception as e:
                print(f"[ERROR] 密码认证出错: {str(e)}")
        
        if not auth_success:
            print("[ERROR] 所有认证方法都失败")
            return False
        
        print("[INFO] SSH连接成功")
        
        # 执行一个简单的命令来验证连接
        print("[INFO] 执行测试命令...")
        stdin, stdout, stderr = ssh.exec_command('echo "SSH连接测试成功"')
        exit_code = stdout.channel.recv_exit_status()
        
        if exit_code == 0:
            output = stdout.read().decode('utf-8').strip()
            print(f"[SUCCESS] 测试命令执行成功: {output}")
            print(f"[INFO] 使用的认证方法: {', '.join(auth_methods)}")
        else:
            error_msg = stderr.read().decode('utf-8')
            print(f"[WARNING] 测试命令执行失败: {error_msg}")
        
        ssh.close()
        return True
        
    except socket.gaierror as e:
        print(f"[ERROR] DNS解析失败: {str(e)}")
        print("[HINT] 请检查服务器IP地址是否正确")
        return False
    except socket.timeout as e:
        print(f"[ERROR] 连接超时: {str(e)}")
        print("[HINT] 请检查网络连接和服务器状态")
        return False
    except socket.error as e:
        print(f"[ERROR] 网络连接错误: {str(e)}")
        print("[HINT] 请检查服务器IP地址和端口是否正确")
        return False
    except paramiko.SSHException as e:
        print(f"[ERROR] SSH连接错误: {str(e)}")
        print("[HINT] 可能的原因:")
        print("       1. SSH服务未运行")
        print("       2. 防火墙阻止连接")
        print("       3. SSH端口不正确")
        return False
    except Exception as e:
        print(f"[ERROR] 未知错误: {str(e)}")
        return False

def main():
    parser = argparse.ArgumentParser(description='SSH连接测试工具')
    parser.add_argument('--host', required=True, help='服务器IP地址')
    parser.add_argument('--port', default=22, type=int, help='SSH端口')
    parser.add_argument('--user', required=True, help='用户名')
    parser.add_argument('--password', help='密码（可选）')
    parser.add_argument('--key-file', help='SSH私钥文件路径（可选）')
    
    args = parser.parse_args()
    
    print(f"[INFO] SSH连接测试工具启动")
    print(f"[INFO] 目标服务器: {args.user}@{args.host}:{args.port}")
    
    if args.key_file:
        print(f"[INFO] SSH私钥文件: {args.key_file}")
    if args.password:
        print(f"[INFO] 提供了密码，长度: {len(args.password)} 字符")
    
    print("")
    
    # 测试SSH连接
    success = test_ssh_connection(
        hostname=args.host,
        port=args.port,
        username=args.user,
        password=args.password,
        key_file=args.key_file
    )
    
    if success:
        print("[SUCCESS] SSH连接测试通过")
        sys.exit(0)
    else:
        print("[FAILED] SSH连接测试失败")
        sys.exit(1)

if __name__ == '__main__':
    main() 