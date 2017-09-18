#!/usr/bin/python
# -*- coding: utf-8 -*-
#由于服务器性能有限，为提高对整个工程的CA速度，于是写了这个脚本
#执行快速CA：根据用户上传的文件(.c .h)和文件的依赖关系确定需要对哪些文件CA
#要CA的文件=用户上传的文件+其依赖文件+自定义的文件
#====================根据不同项目修改=====================

PATH_PROJECT = r"E:\SVTI\Code"
BRANCH_OFFICAL = "master"
BRANCH_PUSH = "work"

#CA结果保存文件
CA_LINT_TXT = PATH_PROJECT + r"\.git\CA_Lint.txt"

#要CA的非PUBLIC文件夹目录
DIRS_CA_NOT_PUBLIC = [ \
PATH_PROJECT + r'\VPN',\
PATH_PROJECT + r'\NETFWD',\
]

##CA_EXE = r'E:\CATool\lintware\MCAnalyzer.bat'
CA_EXE = r'C:\CATool\lintware\MCA_Filter.exe'

#要执行的CA命令 最终的结果保存到特定的Lint文件中
CA_CMDS = [ \
CA_EXE + r' all ' + PATH_PROJECT + ' ipsec > ' + CA_LINT_TXT,\
CA_EXE + r' all ' + PATH_PROJECT + ' tunnel >> ' + CA_LINT_TXT,\
CA_EXE + r' all ' + PATH_PROJECT + ' tunn4 >> ' + CA_LINT_TXT,\
CA_EXE + r' all ' + PATH_PROJECT + ' tunn6 >> ' + CA_LINT_TXT,\
]

#是否根据头文件自动查找需要CA的C文件
CA_ADD_CFILE_BY_HFILE = True

#白名单：每次都要CA的特定文件(自定义的文件)
FILES_CA_PREDEFINE_WHITE = [ \
r'E:\SVTI\Code\VPN\src\sbin\ipsec\app\ipsec_main.c',\
r'E:\SVTI\Code\VPN\src\sbin\ipsec\app\ipsec_spdbase.c',\
]

#黑名单：每次都不要CA的特定文件(自定义的文件)
FILES_CA_PREDEFINE_BLACK = [ \
r'E:\SVTI\Code\VPN\src\sbin\ipsec\lib\ipsec_gdoi_dbm.c',\
r'E:\SVTI\Code\VPN\src\sbin\ipsec\cliplugin\ipsec_gdoi_bdrn.c',\
r'E:\SVTI\Code\VPN\src\sbin\ipsec\app\ipsec_data_issucheck.c',\
r'E:\SVTI\Code\VPN\src\kernel\ipsec\ipsec_kcrypto.c',\
r'E:\SVTI\Code\NETFWD\src\sbin\tunnel\cliplugin\tunnel\tunnel_clirc.c',\
]

#========================================================

_DEBUG = False
import sys,os,subprocess,re,time

###########################################################################
#                              执行系统命令
#输入：系统命令
#返回值：输出结果
###########################################################################
def os_shell(cmd):
    
    print("Run:", cmd)
    
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

###########################################################################
#                           获取提交修改的文件
#参数：无
#返回值：files列表
#备注：提交的log可能有多个
###########################################################################
def Git_GetFiles():
    
    log_file = os_shell("git diff --stat=255 " + BRANCH_PUSH + " " + BRANCH_OFFICAL)
    
    #使用正则表达式找出其中的.c .h文件
    p = re.compile(r'(?<=\s)[^. ]+\.[ch](?=\s+\|\s+\d+\s)')
    files = p.findall(log_file)
    
    #为每个文件名完整未绝对路径，反斜杠'/'替换为斜杠'\'
    i = 0
    for f in files:
        files[i] = PATH_PROJECT + '\\' + f.replace('/', '\\')
        i += 1

    return files

############################################################################
#
#                    判断某个C文件和H文件列表是否具有依赖关系
#Head_File：头文件名(也可以有c文件)
#C_File：C文件名
############################################################################
def JudgeReleation(Head_Files, C_File):

    #读取C文件内容
    try:
        f = open(C_File)
    except:
        return False
    try:
        Lines = f.readlines()
    except Exception,e:
        print("Read File " + C_File + " Error: " + str(e))
        return False
    f.close()

    Content = "".join(Lines)
    
    #获取CFile的依赖文件
    r = re.compile(r'#include\s{0,}["<]\s{0,}(\S+)\s{0,}[">]')
    ships = r.findall(Content)
    
    #去除路径名取文件名
    i = 0
    for ship in ships:
        ships[i] = os.path.basename(ship)
        i += 1
        
    #去除路径名取文件名
    headers = []
    for head in Head_Files:
        headers.append(os.path.basename(head))
    
    #查找Head_Files是否在依赖文件中(即查找两列表的交集是否为空)
    if set(headers) & set(ships):
        return True
    
    return False

############################################################################
#
#                    删指定目录中与列表中无依赖关系的C文件
#                    根据黑白名单删除文件
#Path:要遍历的文件夹路径
#PushFiles:用户上传的文件列表
#PreDefWhiteFiles:自定义的文件列表(白名单)
#PreDefBlackFiles:自定义的文件列表(黑名单)
############################################################################
def DeleteCFiles(Path, PushFiles, PreDefWhiteFiles, PreDefBlackFiles):  

    #遍历所有的文件f
    for root,dirs,files in os.walk(Path):
        for filespath in files:
            f = os.path.join(root, filespath)
            #如果属于黑名单且不属于白名单，删除
            if f in PreDefBlackFiles and not f in PreDefWhiteFiles:
                os.remove(f)
                continue
            #如果是C文件且不属于(用户上传+白名单)文件列表且与用户上传文件无依赖关系 则删除
            if f[-2:] == ".c" and not f in PushFiles and not f in PreDefWhiteFiles:
                #如果使能了CA_ADD_CFILE_BY_HFILE才根据依赖关系查找
                if CA_ADD_CFILE_BY_HFILE:                
                    if not JudgeReleation(PushFiles, f):
                        os.remove(f)
                    else:
                        print("|CA Add| " + f)
                else:
                    #如果未使能则只要不属于(用户上传+白名单)文件列表就删除
                    os.remove(f)

############################################################################
#
#                              运行CA脚本
#
############################################################################
def CA_Run():

    for each_cmd in CA_CMDS:
        os.system(each_cmd)
        #每条命令留出1s
        time.sleep(1)


############################################################################
#
#                                  init
#设置工作路径
############################################################################
def Init():

    #将路径设置到仓库目录
    os.chdir(PATH_PROJECT)
    
    #切换到BRANCH_PUSH (调用该脚本前先确认已在PUSH分支)
    os.system("git checkout " + BRANCH_PUSH)


############################################################################
#
#                                  Fini
############################################################################
def Fini():
    
    os.system("git reset --hard")
    
    
if _DEBUG == True: 
    import pdb 
    pdb.set_trace()
############################################################################
#
#                                  main
#
############################################################################
if __name__ == '__main__':
    
    Init()
    
    #暂时切换到主线分支然后回到上传分支，来获取修改的文件，再加上特定的文件
    PushFiles = Git_GetFiles()
    
    #删除不在files中的C文件
    for dirs in DIRS_CA_NOT_PUBLIC:
        DeleteCFiles(dirs, PushFiles, FILES_CA_PREDEFINE_WHITE, FILES_CA_PREDEFINE_BLACK)
        
    #CA并保存结果到文件
    CA_Run()
    
    Fini()
    
    

