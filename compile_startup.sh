#!/bin/bash
#编译需要开机启动的脚本,以root身份登录
#============根据项目的不通需要修改的地方===================
#jenkins登录编译服务器的账户名
ACCOUT_JENKINS=jenkins
#编译环境的打包文件        eg:由PUBLIC目录和VPN目录组成
FILE_COMPILE_ENV=/home/$ACCOUT_JENKINS/build/compile.tar
#总工作空间
PATH_WORKSPACE=/home/$ACCOUT_JENKINS/WORKSPACE
#===========================================================
FILE_LOCK_COMPILE=/tmp/jenkins_compile_lock
PATH_COMPILE=$PATH_WORKSPACE/Compile #编译所在文件夹
FILE_FIFO=/home/jenkins/build/compile_fifo
MSG_WORKSPACE_OK="WorkspaceIsOk"
MSG_PREPARE="PleasePrepareWorkspace"

function make_fifo
{
    rm -f $FILE_FIFO 2> /dev/null
    # 删除所有的锁
    rm -rf $FILE_LOCK_COMPILE 2> /dev/null
    mkfifo $FILE_FIFO
    chown $ACCOUT_JENKINS:$ACCOUT_JENKINS $FILE_FIFO
}

#重新解压编译环境
function prepare_workspace
{
    if [ -d "$PATH_WORKSPACE" ]
    then
        rm -rf $PATH_WORKSPACE/*
    fi
    
    mkdir -p $PATH_COMPILE
    cd $PATH_COMPILE
    tar xf $FILE_COMPILE_ENV
    chown -R $ACCOUT_JENKINS:$ACCOUT_JENKINS $PATH_WORKSPACE
}

#等待收到重新准备环境的消息
function wait_prepare_msg
{
    while :
    do
        if [ ! -p "$FILE_FIFO" ]
        then
            make_fifo
        fi
        f=$(cat $FILE_FIFO)        
        if [ "$f" == "$MSG_PREPARE" ]
        then
            break;
        fi
    done
}


make_fifo

while :
do
    #重新准备编译环境
    prepare_workspace
    echo $MSG_WORKSPACE_OK > $FILE_FIFO
    wait_prepare_msg
done

