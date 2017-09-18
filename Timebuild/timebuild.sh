#!/bin/bash
#编译环境打包文件解压出来应为一个文件夹，文件夹里面有compile.sh文件，比如
#打包文件cen_debug.tar解压出来：
#--文件夹cen_debug (和打包文件名一致)
#     --文件compile.sh (编译脚本,可自动编出最终的文件xxx.vmdk)
#     --文件夹 PUBLIC
#

#此脚本需定时执行
#在/etc/rc.d/rc.local这个脚本的末尾加上：  /sbin/service crond start
#以jenkins用户的运行crontab -e
#例如要在每天 12:02的时候进行TimeBuild
#编辑文本，加上 2 12 * * * /home/jenkins/build/timebuild.sh
#              分 时 日 月 周 脚本路径

#===============================需要修改的地方==============================
#最终版本的存放路径
PATH_VERSION=/home/share/Version
#编译环境打包文件路径
PATH_TAR=/home/svti/jenkins
#Git远程拷贝代码的地址
GIT_CLONE_ADDRESS=ssh://git@192.168.11.22/SVTI/Code
#编译路径
PATH_WORKSPACE=/home/jenkins/TimeBuild
#还要修改tar_all函数和每个编译函数(如cen_debug)的最后一句cp -rf ... 
#===========================================================================
PATH_MYSELF=$(cd $(dirname $0) && pwd)
FILE_LOG=$PATH_MYSELF/timebuild.log
PATH_CODE=$PATH_WORKSPACE/$(basename $GIT_CLONE_ADDRESS)
PATH_VERSION_OBJ=$PATH_VERSION/$(date "+%Y%m%d_%H%M%S")
FILE_GIT_LOG=$PATH_VERSION_OBJ/Git_Log.txt
PATH_VERSION_LASTEST=$PATH_VERSION/Lastest
FILE_LASTEST_LOG=$PATH_VERSION_LASTEST/$(basename $FILE_GIT_LOG)
GIT_COMMIT_HASH_LEN=40

function ExitErr
{
    echo "================$*================"
    exit 1
}

function ExitMsg
{
    echo "----------------$*----------------"
    exit 0
}

#获取最新代码的Log，如果与Lastest目录下的Git_Log不一样则返回0
function Judge
{
    echo -e "\r\n\r\n"
    date "+%F %T"
    
    if [ "$(whoami)" == "root" ]
    then
        echo "================Do not run as Root================"
        return 1
    fi
    
    if [ ! -d "$PATH_CODE" ]
    then
        if [ ! -d "$PATH_WORKSPACE" ]
        then
            rm -rf $PATH_WORKSPACE
            mkdir $PATH_WORKSPACE
        fi
        
        cd $PATH_WORKSPACE
        git clone --branch master $GIT_CLONE_ADDRESS || ExitErr "Git clone Failed when Judge!"
    else
        #更新代码
        cd $PATH_CODE
        git pull --all || ExitErr "Judge:Git Pull Failed!"        
    fi    
    
    #获取最新代码的Hash与上次编译的Hash
    cd $PATH_CODE || ExitErr "No git code Found!"
    NewHash=$(git log -1 --format=%H)
    
    if [ ! -f "$FILE_LASTEST_LOG" ]
    then
        echo "----------------Start to build first time----------------"
        return 0
    fi
    OldHash=$(sed -rn "1s/.*commit (\\w+).*/\1/p" $FILE_LASTEST_LOG)
    
    if [ "${#NewHash}" -ne "$GIT_COMMIT_HASH_LEN" -o "${#OldHash}" -ne "$GIT_COMMIT_HASH_LEN" ]
    then
        ExitErr "Hash Len Error:${NewHash},${OldHash}"
    fi
    
    #Hash不相等 需要编译
    if [ "$NewHash" != "$OldHash" ] 
    then
        return 0
    fi
    
    return 1
}

function Init
{
    echo -e "\r\n----------------------------------------------------------------------------"
    StartTime=$(date "+%s")
    mkdir -p $PATH_VERSION_OBJ
    pushd . > /dev/null
}

function Fini
{
    EndTime=$(date "+%s")
    let "ElapseTime=EndTime-StartTime"
    echo "--------------------TimeBuild Elapsed $ElapseTime Seconds in all--------------------"
    echo -e "\r\n\r\n\r\n\r\n"
    popd > /dev/null
}

function tar_all
{  
    #清除原来的工作空间
    rm -rf $PATH_WORKSPACE 2> /dev/null
    mkdir $PATH_WORKSPACE
    
    cd $PATH_WORKSPACE    
    tar xf $PATH_TAR/cen_debug.tar
    tar xf $PATH_TAR/mpu_debug.tar
    tar xf $PATH_TAR/mpu_release.tar
    
    TarTime=$(date "+%s")
    let "TarTime=TarTime-StartTime"
    echo "--------------------Tar All Elapsed $TarTime Seconds--------------------"
}

#获取远程代码，保存Git_Log
function fetch_code
{
    cd $PATH_WORKSPACE
    rm -rf $PATH_CODE 2> /dev/null
    git clone --branch master $GIT_CLONE_ADDRESS || ExitErr "Git clone Failed when Fetch!"
    
    #保存Git_Log
    cd $PATH_CODE
    git log -3 --stat > $FILE_GIT_LOG
    unix2dos $FILE_GIT_LOG
}

function cen_debug
{    
    local VersionName=cen_debug
    cd $PATH_WORKSPACE/$VersionName || return 1
    
    #复制代码
    yes | cp -rf $PATH_CODE/* . || return 1
    
    #编译并生成最终版本
    ./compile.sh || return 1
        
    #拷贝最终版本到指定目录
    local VerDir=$PATH_VERSION_OBJ/$VersionName
    mkdir -p -m 755 $VerDir
    #-------------------------------需要根据情况修改----------------------------------------
    cp -rf $PATH_WORKSPACE/$VersionName/PUBLIC/product/simware7/version/debug/*.vmdk $VerDir    
}

function mpu_debug
{    
    local VersionName=mpu_debug
    cd $PATH_WORKSPACE/$VersionName || return 1
    
    #复制代码
    yes | cp -rf $PATH_CODE/* . || return 1
    
    #编译并生成最终版本
    ./compile.sh || return 1
        
    #拷贝最终版本到指定目录
    local VerDir=$PATH_VERSION_OBJ/$VersionName
    mkdir -p -m 755 $VerDir
    #-------------------------------需要根据情况修改----------------------------------------
    cp -rf $PATH_WORKSPACE/$VersionName/PUBLIC/product/simware7/version/debug/*.vmdk $VerDir    
}

function mpu_release
{    
    local VersionName=mpu_release
    cd $PATH_WORKSPACE/$VersionName || return 1
    
    #复制代码
    yes | cp -rf $PATH_CODE/* . || return 1
    
    #编译并生成最终版本
    ./compile.sh || return 1
        
    #拷贝最终版本到指定目录
    local VerDir=$PATH_VERSION_OBJ/$VersionName
    mkdir -p -m 755 $VerDir
    #-------------------------------需要根据情况修改----------------------------------------
    cp -rf $PATH_WORKSPACE/$VersionName/PUBLIC/product/simware7/version/release/*.vmdk $VerDir    
}

function copy2lastest
{
    rm -rf $PATH_VERSION_LASTEST
    cp -rf $PATH_VERSION_OBJ $PATH_VERSION_LASTEST
}

#定时执行此函数即可
function TryBuild
{
    #判断是否需要Build
    Judge || ExitMsg "No Need to Build"
    
    Init
    
    tar_all
    fetch_code
    
    cen_debug    || echo "================Make cen_debug Failed!================"
    mpu_debug    || echo "================Make mpu_debug Failed!================"
    mpu_release  || echo "================Make mpu_release Failed!================"    
    
    copy2lastest
    
    Fini
}

TryBuild 2>&1 | tee -a $FILE_LOG


