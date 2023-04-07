#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

#define KEY        ""  //key值
#define CITY       "河北省"                              //城市
#define EXTENSIONS "base"                             //请求内容
#define SIZE       128                                //curl大小    
#define STR        1024                               //字符串大小  

//设置结构体来存放数据
typedef struct DataStruct{
    char* memroy;
    size_t size;
}Data;

typedef struct WearStruct{
    struct json_object* city;              //城市
    struct json_object* weather;           //天气 
    struct json_object* temper;            //温度
    struct json_object* wind;              //风方向
    struct json_object* hum;               //湿度
    struct json_object* reporttime;        //更新时间
}Wear;

//接收数据回调函数
size_t got_data(char* duffer, size_t size, size_t nmend, void* userp){
    size_t bytes = size * nmend;
    //将userp空指针强制转换为Data类型的指针
    Data* data = (Data*)userp;
    //重新为data中的memroy分配空间
    /**
     * 这里加1是因为想要存放\0，所以需要让它+1
    */
    data->memroy = realloc(data->memroy, data->size + bytes + 1);
    //判断是否开辟空间成功
    if (data->memroy == NULL){
        //如果不成功
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    //将获取的内容拷贝到data结构体中的memroy中
    memcpy(&(data->memroy[data->size]), duffer, bytes);
    data->size += bytes;
    //让data中memroy中的最后位置赋予一个0
    data->memroy[data->size] = 0;
    return bytes;
}

void PrintData(json_object* json, json_object* j_arr_json, Wear* wear){
    //提取数据
    json_object_object_get_ex(j_arr_json, "city", &(wear->city)); //城市
    json_object_object_get_ex(j_arr_json, "weather", &(wear->weather)); //天气
    json_object_object_get_ex(j_arr_json, "temperature", &(wear->temper)); //温度
    json_object_object_get_ex(j_arr_json, "winddirection", &(wear->wind)); //风的方向
    json_object_object_get_ex(j_arr_json, "humidity", &(wear->hum));
    json_object_object_get_ex(j_arr_json, "reporttime", &(wear->reporttime));
    //打印数据
    printf("城市:%s\n天气:%s\n温度:%s\n风向:%s\n湿度:%s\n更新时间:%s\n", json_object_get_string(wear->city), json_object_get_string(wear->weather), json_object_get_string(wear->temper), json_object_get_string(wear->wind), json_object_get_string(wear->hum), json_object_get_string(wear->reporttime));
}

int main(){
    char url[SIZE], str[STR], buf[STR];
    Data main_data;
    size_t len; //数组的长度
    int i;      //遍历数组的变量
    //创建json句柄
    struct json_object* json, *j_info, *j_infocode, *j_arr, *j_arr_json;
    //实例化储存数据的结构体
    Wear wear;
    //文件句柄
    FILE* fd = NULL;
    //拼接字符串
    snprintf(url, SIZE, "https://restapi.amap.com/v3/weather/weatherInfo?key=%s&city=%s&extensions=%s", KEY, CITY, EXTENSIONS);
    //创建CURL句柄
    CURL* curl = NULL;
    CURLcode ret;
    //为main_data中的值进行初始化
    main_data.memroy = malloc(1);
    main_data.size = 0;
    //初始化curl
    curl = curl_easy_init();
    if (curl){
        //配置CURL句柄
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, got_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&main_data);
        //发送连接
        ret = curl_easy_perform(curl);
        if (ret != CURLE_OK){
            fprintf(stderr, "curl_easy_perform feild is %s\n", curl_easy_strerror(ret));
        }
        else{
            printf("%s\n", main_data.memroy);
            fd = fopen("data.json", "w+");
            fwrite(main_data.memroy, 1, main_data.size, fd);
            //关闭文件
        fclose(fd);
        fd = NULL;
        }
    }
    //关闭连接
    curl_easy_cleanup(curl);
    curl = NULL;
    //处理数据
    //打开.json文件
    fd = fopen("data.json", "r");
    //读取.json文件中的内容
    fread(buf, STR, 1, fd);
    //printf("data.json is:%s\n", buf);
    fclose(fd);
    fd = NULL;
    //将读取文件中的内容转换为json
    json = json_tokener_parse(buf);
    //提取json中的内容
    json_object_object_get_ex(json, "info", &j_info);
    json_object_object_get_ex(json, "infocode", &j_infocode);
    //获取lives
    json_object_object_get_ex(json, "lives", &j_arr);
    //读取数组的长度
    len = json_object_array_length(j_arr);
    //将数组转换为json数据并打印出数据
    for (i = 0; i < len; i++){
        j_arr_json = json_object_array_get_idx(j_arr, 0);
        PrintData(json, j_arr_json, &wear);
    }
    //释放json对象
    json_object_put(json);
    return 1;
}