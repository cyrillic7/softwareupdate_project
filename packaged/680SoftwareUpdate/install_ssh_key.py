#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
SSH公钥安装脚本
使用paramiko库直接处理SSH连接，避免Windows命令行问题
"""

import sys
import os
import argparse
import paramiko
from pathlib import Path

def install_ssh_key(hostname, port, username, password, public_key_content):
    """
    安装SSH公钥到远程服务器
    
    Args:
        hostname: 服务器IP地址
        port: SSH端口
        username: 用户名
        password: 密码
        public_key_content: 公钥内容
    
    Returns:
        bool: 安装是否成功
    """
    try:
        print(f"[INFO] 正在连接到 {username}@{hostname}:{port}")
        
        # 创建SSH客户端
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        
        # 连接到服务器
        ssh.connect(
            hostname=hostname,
            port=int(port),
            username=username,
            password=password,
            timeout=30
        )
        
        print("[INFO] SSH连接成功")
        
        # 创建.ssh目录
        stdin, stdout, stderr = ssh.exec_command('mkdir -p ~/.ssh')
        exit_code = stdout.channel.recv_exit_status()
        if exit_code != 0:
            error_msg = stderr.read().decode('utf-8')
            print(f"[ERROR] 创建.ssh目录失败: {error_msg}")
            return False
        
        print("[INFO] .ssh目录创建成功")
        
        # 检查公钥是否已存在
        stdin, stdout, stderr = ssh.exec_command('cat ~/.ssh/authorized_keys 2>/dev/null || echo ""')
        existing_keys = stdout.read().decode('utf-8').strip()
        
        # 检查公钥是否已经存在
        public_key_content = public_key_content.strip()
        if public_key_content in existing_keys:
            print("[INFO] 公钥已存在，无需重复安装")
            ssh.close()
            return True
        
        # 添加公钥到authorized_keys
        command = f'echo "{public_key_content}" >> ~/.ssh/authorized_keys'
        stdin, stdout, stderr = ssh.exec_command(command)
        exit_code = stdout.channel.recv_exit_status()
        if exit_code != 0:
            error_msg = stderr.read().decode('utf-8')
            print(f"[ERROR] 添加公钥失败: {error_msg}")
            return False
        
        print("[INFO] 公钥添加成功")
        
        # 设置正确的权限
        stdin, stdout, stderr = ssh.exec_command('chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys')
        exit_code = stdout.channel.recv_exit_status()
        if exit_code != 0:
            error_msg = stderr.read().decode('utf-8')
            print(f"[WARNING] 设置权限失败: {error_msg}")
            # 权限设置失败不影响整体结果
        
        print("[INFO] 权限设置完成")
        
        # 验证安装结果
        stdin, stdout, stderr = ssh.exec_command('cat ~/.ssh/authorized_keys | grep -c "ssh-rsa\\|ssh-ed25519\\|ecdsa-sha2"')
        key_count = stdout.read().decode('utf-8').strip()
        print(f"[INFO] authorized_keys文件中共有 {key_count} 个公钥")
        
        ssh.close()
        print("[SUCCESS] SSH公钥安装完成")
        return True
        
    except paramiko.AuthenticationException:
        print("[ERROR] SSH认证失败，请检查用户名和密码")
        return False
    except paramiko.SSHException as e:
        print(f"[ERROR] SSH连接错误: {str(e)}")
        return False
    except Exception as e:
        print(f"[ERROR] 未知错误: {str(e)}")
        return False

def main():
    parser = argparse.ArgumentParser(description='SSH公钥安装工具')
    parser.add_argument('--host', required=True, help='服务器IP地址')
    parser.add_argument('--port', default=22, type=int, help='SSH端口')
    parser.add_argument('--user', required=True, help='用户名')
    parser.add_argument('--password', required=True, help='密码')
    parser.add_argument('--key-file', required=True, help='公钥文件路径')
    
    args = parser.parse_args()
    
    # 读取公钥文件
    try:
        with open(args.key_file, 'r', encoding='utf-8') as f:
            public_key_content = f.read().strip()
        
        if not public_key_content:
            print("[ERROR] 公钥文件为空")
            sys.exit(1)
        
        print(f"[INFO] 公钥文件读取成功，长度: {len(public_key_content)} 字符")
        
    except FileNotFoundError:
        print(f"[ERROR] 公钥文件不存在: {args.key_file}")
        sys.exit(1)
    except Exception as e:
        print(f"[ERROR] 读取公钥文件失败: {str(e)}")
        sys.exit(1)
    
    # 安装SSH公钥
    success = install_ssh_key(
        hostname=args.host,
        port=args.port,
        username=args.user,
        password=args.password,
        public_key_content=public_key_content
    )
    
    if success:
        print("[SUCCESS] SSH公钥安装成功")
        sys.exit(0)
    else:
        print("[FAILED] SSH公钥安装失败")
        sys.exit(1)

if __name__ == '__main__':
    main() 