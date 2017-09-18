#!/bin/bash
#=============================================================
#根据项目具体情况修改
#jenkins登录编译服务器的账户名
ACCOUT_JENKINS=jenkins
#总工作空间
PATH_WORKSPACE=/home/$ACCOUT_JENKINS/WORKSPACE
#编译开机启动的脚本名
FILE_COMPILE_STARTUP=compile_startup.sh
#根据实际情况修改clt_run函数
#==============================================================
PATH_COMPILE=$PATH_WORKSPACE/Compile #编译所在文件夹
FILE_LOCK_COMPILE=/tmp/jenkins_compile_lock
FILE_FIFO=/home/jenkins/build/compile_fifo
FILE_CLT_RESULT=$PATH_WORKSPACE/clt_result.txt
FILE_CLT_MAKE_ERROR=$PATH_WORKSPACE/clt_make_err.txt
FILE_RESULT=$PATH_WORKSPACE/result.txt #编译保存的结果供CLT时查询
MSG_WORKSPACE_OK="WorkspaceIsOk"
MSG_PREPARE="PleasePrepareWorkspace"
RESULT_SUCCESS=Success
RESULT_FAILED=Failed

#出错退出
function exit_with_error
{
    ret=${1:-1}    
    rm -rf $PATH_WORKSPACE
    prepare_next
    exit $ret
}

#通知startup脚本准备下一次编译环境
function prepare_next
{
    if [ "$(ps -ef | grep $FILE_COMPILE_STARTUP | wc -l)" -eq 2 ]
    then
        echo "Send signal to prepare next Workspace."
        echo "$MSG_PREPARE" > $FILE_FIFO &
        #释放编译的锁
        rm -rf $FILE_LOCK_COMPILE 2>/dev/null
    else
        echo "Error!!!The compile startup process has not started."
    fi
    
}

#根据编译结果决定是否要CLT
function get_compile_result
{
	if [ ! -f "$FILE_RESULT" ]
	then
		echo "Error!Can not find compile result."
        exit_with_error
	fi
	
	if [ "$(head -n 1 $FILE_RESULT)" != "$RESULT_SUCCESS" ]
	then
		echo "Compile Failed, so the CLT will not start."
		exit_with_error 5555
	fi
	yes | rm -f $FILE_RESULT
}

#运行CLT与结果判断 并实时输出
function clt_run_sub
{
    CLT_NAME="$1"
    echo
    if [ "$#" -ne 4 ]
    then
        echo "$1 CLT Arguments Error."
        return 1
    fi
    
    cd "$2" || return 1
    chmod a+x -R "$2" || return 1
    #MAKE
    sh $3 2> $FILE_CLT_MAKE_ERROR
    ret=$?
    cat $FILE_CLT_MAKE_ERROR
    c=$(cat $FILE_CLT_MAKE_ERROR)
    if [[ "$ret" -ne 0 || "$c" = *error* || "$c" = *Error* || "$c" = *make*Stop* ]]
    then
        echo "=========${CLT_NAME} Make Failed!=========="
        exit_with_error
    fi
    echo -e "${CLT_NAME} CLT Make SUCCESS!\r\n\r\n"
    #运行CLT
    sh $4 2>&1 | tee $FILE_CLT_RESULT
    #判断结果
    r=$(cat $FILE_CLT_RESULT)
    if [[ "$r" = *leaked* || "$r" = *FAILED* || "$r" = *Failure* || ! "$r" = *PASSED* ]]
    then
        echo "Could not pass ${CLT_NAME} CLT."
        ret=1
    else
        echo "${CLT_NAME} CLT PASSED."
        ret=0
    fi
    echo "------------------------------------------------------------------------------------------------------------------------------------"
    
    return $ret
}

#运行CLT，返回结果 根据实际情况修改
function clt_run
{
    #              模块名                       ut脚本路径                   make脚本与参数   run_clt脚本及参数
    clt_run_sub "SVTI_vpn"      "${PATH_COMPILE}/VPN/ut/sbin/ut/vti/gm"      "q.sh"           "run_ut.sh mpu"    || return 1
    clt_run_sub "SVTI_kvpn"     "${PATH_COMPILE}/VPN/ut/kernel/ut/vti"       "q.sh"           "run_ut.sh mpu"    || return 1
    clt_run_sub "SVTI_ktunnel"  "${PATH_COMPILE}/NETFWD/ut/sbin/tunnel/vti/gm" "q.sh"         "run_ut.sh mpu"    || return 1
}

get_compile_result
echo "Prepare to run CLT."
echo "===================================================================================================================================="
#运行CLT
clt_run
#获取返回值
ret=$?
echo "===================================================================================================================================="
prepare_next
echo "CLT Script exit with code $ret"
exit $ret


