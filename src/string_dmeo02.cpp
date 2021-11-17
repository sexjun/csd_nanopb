#include "cds_nanopb.h" // 测试方法的头文件
#include <iostream>
#include <string>

using namespace std;

#define FAILED -1
#define SUCCESS 1

bool encodingString(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    char* name = (char*)*arg;
    // 对字段的线类型和标签编号进行编码
    if (!pb_encode_tag_for_field(stream, field))
    {
        return false;
    }
    // 对该字段的内容进行编码
    return pb_encode_string(stream, (uint8_t*)name, strlen(name));
}

bool decodingString(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    int i=0;
    char* tmp = (char*)*arg;
    while(stream->bytes_left)
    {
        uint64_t value;
        if(!pb_decode_varint(stream, &value)) {
            return false;
        }
        *(tmp+i)=value;
        i++;
    }
    return true;
}

typedef struct {
    string name;
    int age;
    string sex;
}persion_p;
void test(){
    persion_p p ={"cds", 12, "xxxx"};
    cout << p.name << endl;
    cout << p.age << endl;
    cout << p.sex << endl;

    // persion_p p2 = {.age = 12, .sex ="sex", .name = "name"};
    // cout << p2.name << endl;
    // cout << p2.age << endl;
    // cout << p2.sex << endl;
}

int tryStringDataDemo()
{
    char buffer[256] = {0};

    Persion p = Persion_init_default;
    p.name.arg = (char*)&"名字测试";
    p.name.funcs.encode = encodingString;
    p.age = 100;
    pb_ostream_t sizestream = pb_ostream_from_buffer((pb_byte_t *)buffer, sizeof(buffer));;
    bool status = pb_encode(&sizestream, Persion_fields, &p);
    size_t message_length = sizestream.bytes_written;

    std::cout << "=========================" << std::endl;
    {

        Persion p2 = Persion_init_default;
        p2.name.funcs.decode = decodingString;
        char temp[128] = {0};
        pb_istream_t stream = pb_istream_from_buffer((pb_byte_t *)buffer, message_length);
        status = pb_decode(&stream, Persion_fields, &p2);

        std::cout << "name is : " << temp << std::endl;
        std::cout << "age is : " << p2.age << std::endl;
    }

    test();
    return SUCCESS;
}

