#!/bin/bash
#第四版:只负责下载代码和编译，环境的准备由compile_startup.sh负责

#============根据项目的不通需要修改的地方===================
#主线分支
BRANCH_OFFCIAL=master
#上传分支
BRANCH_PUSH=work
#jenkins登录编译服务器的账户名
ACCOUT_JENKINS=jenkins
#编译环境的打包文件        eg:由PUBLIC目录和VPN目录组成
FILE_COMPILE_ENV=/home/$ACCOUT_JENKINS/build/compile.tar
#总工作空间
PATH_WORKSPACE=/home/$ACCOUT_JENKINS/WORKSPACE
#编版本存放路径 [需要用root权限建立并且chown jenkins:jenkins 文件夹名]
PATH_VMDKS=/home/share/jenkins
#Git远程拷贝代码的地址
GIT_CLONE_ADDRESS=ssh://git@192.168.11.22/SVTI/Code
#编译开机启动的脚本名
FILE_COMPILE_STARTUP=compile_startup.sh
#=========编译命令在compile函数中,按需修改==================
#=========生成VMDK文件
#===========================================================
FILE_LOCK_COMPILE=/tmp/jenkins_compile_lock
LOCK_SLEEP_TIME=1
PATH_COMPILE=$PATH_WORKSPACE/Compile #编译所在文件夹
PATH_CODE=$PATH_WORKSPACE/$(basename $GIT_CLONE_ADDRESS)
FILE_RESULT=$PATH_WORKSPACE/result.txt #保存结果供CLT时查询
FILE_GIT_LOG=$PATH_WORKSPACE/Git_Log.txt
PATH_VMDK_LASTEST=$PATH_VMDKS/Lastest
COMPILE_RESULT_TMP=$PATH_WORKSPACE/compile_result_tmp.txt
FILE_FIFO=/home/jenkins/build/compile_fifo
MSG_WORKSPACE_OK="WorkspaceIsOk"
MSG_PREPARE="PleasePrepareWorkspace"
RESULT_SUCCESS=Success
RESULT_FAILED=Failed


#上锁
function lock
{
    while :
    do
        #尝试解锁
        if mkdir $FILE_LOCK_COMPILE 2>/dev/null  
        then
            break
        else
            echo "Try to get lock of compile workspace..."
            
            #假定上锁了一定有用户登录在才属于正常情况
            if [ $(w | awk '{print $1}' | grep $ACCOUT_JENKINS | wc -w) -lt 2 ]
            then
                echo "Workspace exited abnormal last time."
                echo "Force to get lock and remove workspace."
                mkdir $FILE_LOCK_COMPILE 2>/dev/null
                #上次非正常退出的处理，认为clt中写fifo和unlock之间不会异常退出
                rm -rf $PATH_WORKSPACE
                echo $MSG_PREPARE > $FILE_FIFO
                break
            fi
            
            sleep $LOCK_SLEEP_TIME
        fi
    done
}

#出错退出
function exit_with_error
{
    ret=${1:-1}
    rm -rf $PATH_WORKSPACE
    exit $ret
}

#如果编译的开机脚本没有启动，则自己解压编译环境，若启动了则等待编译环境ok
function startup_shell_work_ok
{
    if [ ! "$(ps -ef | grep $FILE_COMPILE_STARTUP | wc -l)" -eq 2 ]
    then
        echo "Error!!!The compile startup process has not started."
        rm -rf $PATH_WORKSPACE 2> /dev/null        
        mkdir -p $PATH_COMPILE
        #解压编译环境到工作空间
        tar xf $FILE_COMPILE_ENV -C $PATH_COMPILE || exit_with_error
    else
        #等待startup脚本把环境准备好
        echo "Check if the workspace has been prepared..."
        msg=$(cat $FILE_FIFO)
        lisdir=$(ls $PATH_COMPILE)
        if [ "$msg" != "$MSG_WORKSPACE_OK" -o -z $lisdir ]
        then
            echo "Workspace error!!!Compile startup script is abnormal."
            exit 1
        fi
    fi
}

#获取远程代码并复制到编译文件夹
function code_push_compile
{
    cd $PATH_WORKSPACE
    rm -rf $PATH_CODE 2> /dev/null
    git clone --branch $BRANCH_PUSH $GIT_CLONE_ADDRESS || exit_with_error
    cd $PATH_CODE
    #jenkins控制台使用的不是UTF-8编码，加iconv解决乱码问题
    git log -1 | iconv -f UTF-8 -t GB18030//TRANSLIT
    #保存日志信息方便make VMDK
    git log -1 --stat > $FILE_GIT_LOG
    unix2dos $FILE_GIT_LOG
    echo
    yes | cp -rf $PATH_CODE/* $PATH_COMPILE || exit_with_error
    
}


#编译(返回值决定编译成功/失败)，将结果显示并输出到$COMPILE_RESULT_TMP
function compile
{
    chmod a+x -R $PATH_COMPILE/PUBLIC
    cd $PATH_COMPILE/PUBLIC/build
    #修复bug:没有将错误信息输出到tmp文件，应该使用 ./xx.sh 2>&1 | tee $COMPILE_RESULT_TMP
    ./simware7.sh -e 64sim7dis.ipe 2>&1 | tee $COMPILE_RESULT_TMP
}

#根据编译的输出$COMPILE_RESULT_TMP判断是否编译成功
function judge_result
{
    Ret=1
    FinalLine=$(tail -n 3 $COMPILE_RESULT_TMP)
    
    #根据编译输出的内容判断编译结果s
    if [[ "$FinalLine" = *===Elapsed*sec.\ for\ simware7* ]]
    then    
        Info=$(grep -i -e warn -e error $COMPILE_RESULT_TMP)
        if [ -z "$Info" ]
        then
            Ret=0
        else
            echo "========Compile Ouput has WARNING or ERROR !=========="
            echo "Error Content:${Info}"
        fi
    fi
    
    rm $COMPILE_RESULT_TMP
    echo "Judge Result Code:$Ret"
    
    return $Ret
}

#结果处理：标识成功失败
function result_proc
{
    if [ ! "$1" -eq 0 ]
    then
        echo $RESULT_FAILED > $FILE_RESULT
    else
        echo $RESULT_SUCCESS > $FILE_RESULT
    fi
    date >> $FILE_RESULT
}


#生成VMDK文件
function make_vmdk
{
    cd $PATH_COMPILE/PUBLIC/product/simware7/version 
    ./make_vmdk.sh
    
    rm -rf $PATH_VMDK_LASTEST
    mkdir -p -m 755 $PATH_VMDK_LASTEST
    cp -f debug/chassis_mpu64.vmdk $PATH_VMDK_LASTEST
    cp -f debug/chassis_lpu64.vmdk $PATH_VMDK_LASTEST
    cp -f $FILE_GIT_LOG $PATH_VMDK_LASTEST
    
    DateDir=$PATH_VMDKS/$(date "+%Y%m%d_%H%M%S")
    mkdir -m 755 $DateDir
    cp -f debug/chassis_mpu64.vmdk $DateDir
    cp -f debug/chassis_lpu64.vmdk $DateDir
    cp -f $FILE_GIT_LOG $DateDir
}


############################Start#################################

#将编译环境拷贝到工作空间(如果有则跳过)，更新代码，然后合入工作空间

echo "Prepare to compile."
lock

#如果编译的开机脚本没有启动(异常)，则自己解压编译环境，若启动了则等待编译环境ok
startup_shell_work_ok

#复制代码进去
code_push_compile

#代码已在工作空间准备好，开始编译
w
echo
echo "Start to compile."
echo "====================================================================="
compile
echo "====================================================================="
echo "Compile finished!"

#根据输出信息判断编译是否成功
judge_result
result=$?

#输出结果到文件
result_proc $result

if [ "$result" -eq 0 ]
then
    echo "Congratulations!Compile Success!"
    echo "Start to make VMDK."
    make_vmdk
else
    echo "Error!Compile Failed!"
fi
echo "Script exit with code $result"
exit $result




