import subprocess

usage = """ 
    1. 拉取子节点的代码
    2. 上所有代码
    3. 编译nanopb
    
"""



print(usage)
run_code = input("请输入指令：")

if int(run_code) == 0:
    exit(1)
elif int(run_code) == 1:
    sub_code = "git submodule init && git submodule update"
elif int(run_code) == 2:
    sub_code = "git add . && git commit -m \"all\" && git push"
elif int(run_code) == 3:
    sub_code = "cd  nanopb && python cds_compile_and_move_file.py"
else:
    print("========================================")
    print("unknown command\n")
    print("========================================")
    
print("run the cmd:{}".format(sub_code), end="\n")
subprocess.run(sub_code, shell=True)