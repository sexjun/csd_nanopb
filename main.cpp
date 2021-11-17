#include "cds_nanopb_info.pb.h" // nanopb的配置文件
#include "cds_nanopb.h" // 测试方法的头文件

#include <stdio.h>
#include <pb_encode.h>
#include <pb_decode.h>

int main()
{
    simple_demo01();
    tryStringDataDemo();
    return 0;
}

