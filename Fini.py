#!/usr/bin/python
# -*- coding: utf-8 -*-
#Last modify time 2015/5/1 0:10 Oliver
#这个脚本在CA结束后，合入BRANCH_OFFCIAL前被调用
#(调用该脚本前先确认已在BRANCH_OFFICAL分支)
#参数1 BRANCH_OFFICAL名（主线分支）
#参数2 BRANCH_PUSH名（提交分支）
#参数3 CA结果(Success/Failed)

#=========================================================================
#根据项目情况修改：

#仓库目录
PATH_PROJECT = "E:\\SVTI\\Code"

#Jenkins的Jobs路径
PATH_JENKINS_JOBS = "C:\\Program Files (x86)\\Jenkins\\jobs"

#最后一个Jobs（C_Analyzer）的名称
JOBS_NAME_CA = "C_Analyzer"

#Jenkins的http地址(供外部访问)
URL_JENKINS_JOB = "http://192.168.11.22:8080/job"

#设置服务器，用户名、口令以及邮箱的后缀
mail_host = "192.168.11.22"
mail_user = "sys@svti.com"
mail_pass = "123456"
mail_postfix = "svti.com"

#--------------------------------------------------------------------------
#下一个构建ID文件的文件名
FILE_NEXT_BUILD_NUM = "nextBuildNumber"

#以下文件名应和AutoCA_Config.txt中的内容一致,一般不用修改
#CA结果的文本文件
PATH_LINT = PATH_PROJECT + "\\.git\\CA_Lint.txt"
#保存log的文本文件
PATH_LOG = PATH_PROJECT + "\\.git\\git_log_fini.html"
#GitAutoCA.exe的配置文件
PATH_GIT_CONFIG = PATH_PROJECT + "\\.git\\Git_AutoCA_Config.txt"

#主线分支名 (不需要知道，只需确保调用该脚本时处于BRANCH_OFFCIAL分支)
#BRANCH_OFFCIAL = "master" (改由第一个参数传入)
#提交分支名
#BRANCH_PUSH = "work" (改由第二个参数传入)
#=========================================================================

Success = "Success"
Failed  = "Failed"

_DEBUG = False
#导入smtplib和MIMEText和html和re
import smtplib,sys,os,subprocess,html,re
from email.mime.text import MIMEText
    
#任务信息相关的全局变量    
#任务名
g_Jobs = []
#任务结果
g_Jobs_Result = []
#任务ID
g_Jobs_ID = []
#任务输出URL
g_Jobs_URL = []
#任务数量
g_Jobs_Count = 0
#主线的分支名
g_Branch_Offical = ""
#用户提交的分支名
g_Branch_Push = ""
#最终判定结果(由Git_AutoCA.exe判定)
g_FinalResult = ""

#由参数传入的CA结果 Success/Failed
g_Arg_CA_Result = ""


############################################################################
#                            邮件的某些内容
############################################################################
txt_success='''

===================================================================================
                                                       
                    [ Congratulations! Merge SUCCESSFULLY ! ]       
                                                      
===================================================================================

'''

txt_failed='''

===================================================================================
                                                       
                       [ Could not pass Exam, Merge FAILED. ]            
                                                         
===================================================================================

'''
txt_line = "\n-----------------------------------------------------------------------------------\n\n"
txt_table_line = "=================================================================================================\n"    
txt_each_job_result = u'[各任务情况]'
txt_commit_message = u'[提交信息]'
txt_file_modify = u'[修改文件]'
txt_cabrief_cn = u'[CA结果摘要]'
txt_config_cn = u'[告警数量限制]'
txt_ca_vebose = u'[CA详细结果]'


###########################################################################
#                          给个元素加上html头
#参数：html元素集
#返回：完整的html文本
###########################################################################     
def Html_AddHead(Elems):

    Head = '''
<html>
<head>
<meta name="GENERATOR" content="Microsoft FrontPage 5.0">
<meta name="ProgId" content="FrontPage.Editor.Document">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
</head>
<body>
'''    
    return(Head + Elems + "</body></html>\n\n\n\n")


###########################################################################
#                          将文本转换为Html中的元素
#参数：文本
#返回：文本元素
###########################################################################    
def Html_TxtElem(txt):

    ret = html.escape(txt)
    return("<pre>" + ret + "</pre>")
    

#将文本中的URL转换为Html中的链接(如果有的话)
def Html_TxtElem_Link(txt):

    ret = Html_TxtElem(txt)
    
    ret = re.sub(r'((?:http|ftp|https|file)://(?:[^ \n\r<\)]+))', '<a href="\\1">\\1</a>', ret)
        
    return ret     
    
###########################################################################
#                          绘制一个html表格
#参数Table：一个二维的数组，内容都是文本
#参数Num_Cols:二维数组的列数
#参数Num_Lines:二维数组的行数
#构造一个表格 Table[列][行]是二维数组，Col_Width是一维数组
#表格中若有URL会自动转为超级链接
########################################################################### 
def Html_Table(Table, Num_Cols, Num_Lines):

    ret = '<table style="border-collapse:collapse;" border="1" width="30%" cellpadding="10">\n'
    for i in range(Num_Lines):
        ret += '  <tr>\n'
        for j in range(Num_Cols):
            ret += '    <td align="center" valign="center">' + Html_TxtElem_Link(Table[j][i]) + '</td>\n'
        ret += '  </tr>\n'
    
    ret += '</table>\n'
    
    return ret

    
###########################################################################
#                              执行系统命令
#输入：系统命令
#返回值：输出结果
###########################################################################
def os_shell(cmd):
    
    #获取输出
    out = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    
    #读取输出
    lines = out.stdout.readlines()
    
    #编码转换
    result = ""
    for line in lines:
        result = result + line.decode('utf-8')
    
    return result

   
###########################################################################
#                            发送邮件函数
#mailto_list    收件人邮箱
#sub            邮件题目
#content_html   邮件内容
###########################################################################
def send_mail(mailto_list, sub, content_html):

    me=mail_user+"<"+mail_user+"@"+mail_postfix+">"
    msg = MIMEText(content_html, 'html', 'utf-8') 
    msg['Subject'] = sub 
    msg['From'] = me
    msg['To'] = ";".join(mailto_list) 
    try: 
        s = smtplib.SMTP() 
        s.connect(mail_host) 
        s.login(mail_user,mail_pass) 
        s.sendmail(me, mailto_list, msg.as_string()) 
        s.close() 
        return True
    except Exception: 
        print (Exception)
        return False


###########################################################################
#                            获取提交者邮箱
#返回值:提交者邮箱
###########################################################################
def get_mail():

    mail = os_shell("git log -1 --format=%ae --branches " + g_Branch_Push)

    if len(mail) > 5:
        print("Get user`s email address:" + mail)
    else:
        print("无法识别提交者的邮箱")
        return None
    
    #去掉结尾的'\n'
    mail = mail[:-1]
    
    return mail
    
        
###########################################################################
#                           获取CA结果及摘要
#参数：无
#返回值一：CA完整结果
#返回值二：CA摘要
###########################################################################
def get_CA():
    
    #读取CA_Lint文件
    f = open(PATH_LINT)
    lines = f.readlines()
    f.close()
    
    #将读取的内容存到txt,并读取Error结果
    ca_all = ca_brief = ""
    for i in lines:
        ca_all = ca_all + i
        if i.find(" Error,") >= 0 and \
           i.find(" Warning,") >= 0 and \
           i.find(" Info,") >= 0 and \
           i.find(" Note") >= 0:
            ca_brief = ca_brief + i
    
    return (ca_all, ca_brief)
    
    
###########################################################################
#                           获取提交分支的log
#参数：无
#返回值：log内容
###########################################################################
def get_log():
    
    #获取BRANCH_PUSH的所有log (这里限制了一次性最大commit数)
    log = os_shell("git log -1 --branches " + g_Branch_Push)
         
    return log


###########################################################################
#                           获取修改的文件
#参数：无
#返回值：log内容
###########################################################################
def Git_GetModifyFiles():
    
    log = os_shell("git diff --stat=255 --stat-graph-width=35 " + g_Branch_Offical + " " + g_Branch_Push)

    return log
    
    
###########################################################################
#                           获取配置文件内容
#参数：无
#返回值：配置文件的告警部分
###########################################################################
def get_CAConfig():
    
    f = open(PATH_GIT_CONFIG, 'r')
    
    all = f.readlines()
    
    f.close()
    
    config = ""
    for eachline in all:
        if eachline.find("error") >= 0 or \
           eachline.find("warning") >= 0 or \
           eachline.find("info") >= 0 or \
           eachline.find("note") >= 0:
            config = config + eachline
            
    return config

###########################################################################
#                          获取各任务的结果
#参数一:Log文件路径
#返回:Success/Failed
###########################################################################
def _get_Jobs_Result(Path_Log):
        
    print("Job path is ", Path_Log)
    #读取文件最后9个字符获得编译成功/失败的信息
    f_log = open(Path_Log, "rb")
    f_log.seek(-9, 2)
    Result = f_log.read(7).decode()    
    print("Job Result:", Result)
    f_log.close()
    
    #结果以Success或Failed表示
    if Result == "SUCCESS":
        return Success

    return Failed 
    
    
###########################################################################
#                          获取各任务的构建ID
#结果保存到g_Jobs_ID中
########################################################################### 
def get_Jobs_ID_Result_URL():

    global g_Jobs_Count, g_Jobs, g_Jobs_ID, g_Jobs_Result,g_Arg_CA_Result, g_Jobs_URL
    
    i = 0
    #根据该路径下的文件(夹)获得任务名
    dirs = os.listdir(PATH_JENKINS_JOBS)
    
    for each_job in dirs:
        #获取任务名
        g_Jobs.append(each_job)    
        
        #打开NextBuildNum文件获取构建ID
        path = PATH_JENKINS_JOBS + "\\" + each_job + "\\" + FILE_NEXT_BUILD_NUM
        print("Open", path, "to get jobs ID", end = '')
        try:
            f = open(path)
        except:
            print("\nUnable to open", path)
            f.close()
            return
        
        g_Jobs_ID.append(str(int(f.readline()) - 1))
        f.close()
        print(" =", g_Jobs_ID[i])
        
        #获取任务结果
        if each_job != JOBS_NAME_CA:
            g_Jobs_Result.append(_get_Jobs_Result(PATH_JENKINS_JOBS + "\\" + each_job + "\\builds\\" + g_Jobs_ID[i] + "\\log"))
        else:
            g_Jobs_Result.append(g_Arg_CA_Result)
        
        #得出构建URL
        g_Jobs_URL.append(URL_JENKINS_JOB + "/" + each_job + "/" + g_Jobs_ID[i] + "/console")
        print("URL :", g_Jobs_URL[i])

        i = i + 1
    g_Jobs_Count = i

    
###########################################################################
#                          把各项任务结果用表格表示
#根据全局变量g_Jobs[], g_Jobs_Result[], g_Jobs_ID[], g_Jobs_URL[] 画表格
#返回Html格式的表格
########################################################################### 
def JobsResult_HtmlTable():
   
    Table = [ [u'任务'] + g_Jobs, \
              [u'结果'] + g_Jobs_Result,\
              [u'构建ID'] + g_Jobs_ID,\
              [u'控制台输出'] + g_Jobs_URL ]
    
    html_each_jobs_result = Html_TxtElem(txt_each_job_result) 
    return (html_each_jobs_result + Html_Table(Table, len(Table), g_Jobs_Count + 1) + Html_TxtElem(" \n"))
    
    
###########################################################################
#                           将信息追加到log文件
#参数：要写入的内容
#返回值：无
###########################################################################
def save_log(log):
    
    #读取CA_Lint文件
    f = open(PATH_LOG, 'a')
    
    f.write(log)
    
    f.close()
    
    
###########################################################################
#                            编辑邮件标题
#IsSuccess:("Success"/"Failed") 是否CA通过，由程序的第一个参数
#返回值：邮件正文
###########################################################################
def edit_title(IsSuccess):
    
    #成功/失败标识
    if IsSuccess == Success:
        txt_status = u'成功：'
    else:
        txt_status = u'失败：'

    #获取commit的log,并作长度限制
    log = os_shell("git log -1 --format=%s --branches " + g_Branch_Push)
    log = log.strip(' ')[:-1]
    log = log[:64]
    
    #组装成标题
    title = txt_status + log
    
    return title

   
###########################################################################
#                            编辑邮件正文
#IsSuccess:("Success"/"Failed") 是否CA通过，由程序的第一个参数
#返回值：邮件正文
###########################################################################
def edit_body(IsSuccess):
    
    body_before_table = ""
    
    #成功/失败标识
    if IsSuccess == Success:
        body_before_table +=  txt_success
    else:
        body_before_table += txt_failed
        
    #各项任务的成功/失败表格
    html_table = JobsResult_HtmlTable()
        
    #获取提交分支的log
    log = txt_commit_message + "\n" + get_log() + "\n\n" + txt_file_modify + "\n" + Git_GetModifyFiles() + "\n"
       
    #获取CA结果及摘要
    (ca_all, ca_brief) = get_CA()    
        
    #获取限制的告警数量
    config = get_CAConfig()
    
    #保存log信息
    save_log(Html_AddHead(Html_TxtElem(body_before_table) + html_table + Html_TxtElem(log + ca_brief + "\n" + config + txt_line)))
    
    #组装成正文
    body = Html_TxtElem(body_before_table) + html_table + Html_TxtElem( log + txt_line + txt_cabrief_cn + "\n" + ca_brief + "\n"+ txt_config_cn + "\n" + config + txt_line + "\n" + txt_ca_vebose + "\n\n" + ca_all )
    
    body = Html_AddHead(body)
    return body
    
    
############################################################################
#
#                                  init
#解析输入参数,设置工作路径
############################################################################
def Init():
    
    global g_Branch_Offical, g_Branch_Push, g_Arg_CA_Result, g_FinalResult
    
    #解析输入参数  
    Num = 0
    for Arg in sys.argv:
        print("Argv", Num, ":" ,Arg)

        if 1 == Num:
            g_Branch_Offical = Arg
        elif 2 == Num:
            g_Branch_Push = Arg
        elif 3 == Num:
            g_Arg_CA_Result = Arg
        
        Num = Num + 1
    
    if Num != 3 + 1:
        print("Input Arguments Error")
        exit(1)
    
    #将路径设置到仓库目录
    os.chdir(PATH_PROJECT)
    
    #切换到BRANCH_OFFICAL (调用该脚本前先确认已在OFFICAL分支)
    os.system("git checkout " + g_Branch_Offical)
    
    #获取各个任务的ID,成功/失败,URL, 存到全局变量中去
    get_Jobs_ID_Result_URL()
    
    #得出最后结果
    g_FinalResult = Success
    for r in g_Jobs_Result:
        if r != Success:
            g_FinalResult = Failed
            break
    
    print("Final Result:", g_FinalResult)
    
    return g_FinalResult    


###########################################################################
#                            版本控制
#获取个任务情况并决定合入分支还是回退
#写入全局变量g_FinalResult
###########################################################################
def Git_Jobs_Action():
    
    global g_FinalResult   
    
    #根据结果进行版本合入/回退
    if g_FinalResult == Success:
        #成功，合入主线分支
        os.system("git merge " + g_Branch_Push)
    else:
        #失败，删除分支重新建立
        os.system("git branch -D " + g_Branch_Push)
        os.system("git branch " + g_Branch_Push)

    
###########################################################################    
if _DEBUG == True: 
    import pdb 
    pdb.set_trace()     
############################################################################
#
#                                  main
#
############################################################################
if __name__ == '__main__':   
      
    #初始化,获取各个任务结果存到全局变量中
    FinalResult = Init()
    
    #获取提交人邮箱
    mailto_list = [ get_mail() , mail_user ]
    
    #编辑邮件标题
    txt_title = edit_title(FinalResult)
    
    #编辑邮件正文
    html_body = edit_body(FinalResult)

    #发送邮件
    if mailto_list != None and send_mail(mailto_list, txt_title, html_body): 
        print (u'邮件已发送到', mailto_list)
    else: 
        print (u'邮件发送失败')
    
    #获取个任务情况并决定合入分支还是回退
    Git_Jobs_Action()
    
    #切换回主线分支
    os.system("git checkout " + g_Branch_Offical)
    
        
   
