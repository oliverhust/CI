#!/usr/bin/python
# -*- coding: utf-8 -*-
#Last modify time 2015/6/28 0:10 Oliver

#Git_AutoCA模块的ip地址和端口
IP_GIT_AUTOCA = "127.0.0.1"
PORT_GIT_AUTOCA = 7777

#---------------------------------------------------------------------------

#"下一个构建的号码"文件名
FILE_NEXT_BUILD = "nextBuildNumber"

#是否调试
_DEBUG = False

#发送消息头
SEND_HEAD_SUCCESS = "[Success]"
SEND_HEAD_FAILED  = "[Failure]"
SEND_HEAD_BUSY    = "[Busying]"

SEND_HEAD_ALL_JOBS = "[All jobs Finished]"
SEND_HEAD_CA_RESULT_PATH = "[What is CA Result and Path]"
SEND_HEAD_FINI_PY_PATH = "[Give me the Path of Fini_py]"

MSG_SUCCESS = "Success"
MSG_FAILURE = "Failure"

RECV_LEN_MAX = 8000

#===========================================================================
import os,subprocess,socket,time
if _DEBUG == True: 
    import pdb 
    pdb.set_trace() 

###########################################################################
#                              执行系统命令
#输入：系统命令
#返回值：输出结果
###########################################################################
def os_shell(cmd):
    
    print("Run", cmd)
    
    #获取输出
    out = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    
    #读取输出
    lines = out.stdout.readlines()
    
    #编码转换
    result = ""
    for line in lines:
        try:
            result = result + line.decode('gbk')
        except:
            continue
    
    return result

############################################################################
#
#                            Socket Init
#
############################################################################
def Socket_Init():
    
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except:
        print('Failed to create socket.')
        exit(1)
    
    Times = 30
    while Times > 0:
        Err = 0
        try:
            s.connect((IP_GIT_AUTOCA, PORT_GIT_AUTOCA))
        except:
            print("Could not Connect to the Git_AutoCA.")
            Err = 1
        if Err == 0:
            break
        Times = Times - 1
        time.sleep(1)
    
    if Times == 0:
        print("Failed to Connect to the Git_AutoCA.")
        exit(1)
    
    return s

    
############################################################################
#
#                                  main
#
############################################################################
if __name__ == '__main__':
       
    s = Socket_Init()    
    
    #等待CA结束:不断查询是否结束
    i = 0
    while True:
        s.sendall(SEND_HEAD_CA_RESULT_PATH.encode())
        if i == 0:
            print("Send to CA server:", SEND_HEAD_CA_RESULT_PATH)
        recv_CA = s.recv(RECV_LEN_MAX).decode()
        if i == 0:
            print("Receive from CA server:", recv_CA)
        else:    
            print(".", end = '')
        if recv_CA[:len(SEND_HEAD_SUCCESS)] == SEND_HEAD_SUCCESS or \
           recv_CA[:len(SEND_HEAD_FAILED)] == SEND_HEAD_FAILED:
            print("\nReceive from CA server:", recv_CA)
            break;
        else:
            time.sleep(1)
            i += 1
    
    #获取Fini.py的路径及参数并执行
    s.sendall(SEND_HEAD_FINI_PY_PATH.encode())
    print("Send to CA server:", SEND_HEAD_FINI_PY_PATH)
    recv_msg = s.recv(RECV_LEN_MAX).decode()
    print("Receive from CA server:", recv_msg)
    #运行脚本
    print("\n================================Fini.py======================================")
    #print(os_shell("python " + recv_msg), end = '')
	#如果出现'python' 不是内部或外部命令，也不是可运行的程序 则注释上以后换用下面这行
    print(os_shell(recv_msg), end = '')
    #ret = os.system("python " + recv_msg)
    #if ret != 0:
    #    print("Error Code" , ret)
    #    exit(1)
    print("=============================================================================\n")
    
    #获取CA_Lint文件路径
    Path_CALint = recv_CA[recv_CA.find("]") + 1:]
    print("Path of CA_Lint:", Path_CALint)
    #打印CA结果
    print("\n")
    try:
        f_CALint = open(Path_CALint)
        CALint = f_CALint.readlines(65535)
    except:
        f_CALint.close()
        print("Open and read CA File Error")
        exit(1)
    f_CALint.close()
    
    for i in CALint:
        print(i, end = '')
    
    #发送信号：所有任务已完成
    send_msg = SEND_HEAD_ALL_JOBS
    s.sendall(send_msg.encode())
    print("Send to CA server:", send_msg)
    recv_msg = s.recv(RECV_LEN_MAX).decode()
    print("Receive from CA server:", recv_msg)
    if recv_msg != SEND_HEAD_SUCCESS:
        print("Send all jobs finished message to Git_AutoCA Error")
        exit(1)  
    
    s.close()
    
    #返回结果给jenkins
    if recv_CA[:len(SEND_HEAD_SUCCESS)] != SEND_HEAD_SUCCESS:
        exit(5555)
    
    exit(0)
    
    
