#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024 * 4
#define UPLOAD_DIR "/dev/"

// 解析Content-Disposition头部，获取文件名
char *get_filename(char *content_disposition) {
    char *filename = strstr(content_disposition, "filename=");
    if (!filename) {
        return NULL;
    }
    filename += 9; // 跳过"filename="部分
    if (filename[0] == '"' || filename[0] == '\'') {
        filename++;
    }
    char *end = strpbrk(filename, "\"'");
    if (end) {
        *end = '\0';
    }
    return filename;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h> 
int xcam_upgrade(){
    // 要执行的升级脚本命令
    // char script_path[BUFFER_SIZE];
    // snprintf(script_path, sizeof(script_path), "sh /system/bin/read_firmware.sh %s%s", UPLOAD_DIR, "app.bin");
    const char *script_path = "sh /system/bin/read_firmware.sh  /dev/app.bin";
    FILE *fp = popen(script_path, "r");
     struct timespec start, end;
     clock_gettime(CLOCK_MONOTONIC, &start);
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    char output[256];
    while (fgets(output, sizeof(output), fp) != NULL) {
        printf("Upgrade: %s", output);
    }

    // 关闭管道并获取脚本的退出状态
    int status = pclose(fp);
    if (status == -1) {
        perror("pclose");
        return -1;
    } else {
        // WIFEXITED: 如果为非零值表示进程正常退出
        // WEXITSTATUS: 获取进程的退出状态
        if (WIFEXITED(status)) {
            printf("Script exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("Script did not exit normally\n");
            return -1;
        }
    }
     clock_gettime(CLOCK_MONOTONIC, &end);
      double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    
    printf("Elapsed time: %.9f seconds\n", elapsed);
    return 0;
}

int main() {
    char *content_length_str = getenv("CONTENT_LENGTH");
    if (!content_length_str) {
        printf("Status: 411 Length Required\r\n");
        printf("Content-Type: text/html\r\n\r\n");
        printf("<html><body><h1>Content-Length required</h1></body></html>\n");
        return 1;
    }

    long content_length = atol(content_length_str);
    if (content_length <= 0) {
        printf("Status: 400 Bad Request\r\n");
        printf("Content-Type: text/html\r\n\r\n");
        printf("<html><body><h1>Invalid Content-Length</h1></body></html>\n");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read = 0;

    // 读取请求头部，直到找到空行
    char content_disposition[BUFFER_SIZE] = {0};
        //     printf("Status: 400 Bad Request\r\n");
        // printf("Content-Type: text/html\r\n\r\n");
        // printf("<html><body><h1>Filename not found");
        int count = 0;
    while (fgets(buffer, sizeof(buffer), stdin)) {
 count++;
        //   printf("%s", buffer);
        if (strcmp(buffer, "\r\n") == 0 || strcmp(buffer, "\n") == 0) {
            break;
        }
        if (strstr(buffer, "Content-Disposition:") != NULL ) {
            strncpy(content_disposition, buffer, sizeof(content_disposition) - 1);
            break;
        }
    }

    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (strstr(buffer, "Content-Disposition:") != NULL ) {
            strncpy(content_disposition, buffer, sizeof(content_disposition) - 1);
            break;
        }
    }
// printf("%d %s </h1></body></html>\n", count, content_disposition);
// return 1;
    // 获取文件名
    char *filename = get_filename(content_disposition);
    if (!filename) {
       
        printf("Status: 400 Bad Request\r\n");
        printf("Content-Type: text/html\r\n\r\n");
        printf("<html><body><h1>Filename not found</h1></body></html>\n");
        return 1;
    }

    // 构建文件路径
    char filepath[BUFFER_SIZE];
    snprintf(filepath, sizeof(filepath), "%s%s", UPLOAD_DIR, "app.bin");
    if(strstr(buffer, "\r\n\r\n")){
        int h = 0;
    }else{
        fgets(buffer, sizeof(buffer), stdin);
    }
    
    // 打开文件以写入
    FILE *file = fopen(filepath, "wb");
    if (!file) {
        perror("fopen");
        printf("Status: 500 Internal Server Error \r\n");
        printf("Content-Type: text/html\r\n\r\n");
        printf("<html><body><h1>Failed to open file for writing %s</h1></body></html>\n", filepath);
        return 1;
    }
    long max = 0;
    long min = 0;
    // char *start = strstr(buffer, "\r\n\r\n");
    // if(start == NULL){
    //      printf("Status: 500 Internal Server Error %s\r\n", filepath);
    //     printf("Content-Type: text/html\r\n\r\n");
    //     printf("<html><body><h1>Failed to start xxxx %s\r\n %s</h1></body></html>\n", content_disposition, buffer);
    // }
    // return -1;
    // start += 4;
    // long c = (start - buffer);
    //   min += fwrite(start, 1, BUFFER_SIZE - (start - buffer), file);
    // 读取并保存文件内容

    memset(buffer, 0, sizeof(buffer));
    // long remaining = content_length - 61;
    int leng = sizeof(buffer);
    int first = 1;
    while (  (bytes_read = fread(buffer, 1, leng, stdin)) > 0) {
        if(bytes_read <leng ){
            bytes_read -= 51;
        }else if( first == 1){
            first = 0;
            bytes_read -=2;
            min += fwrite(buffer + 2, 1, bytes_read, file);
            max += bytes_read;
            continue;
        }
        // char *end = strstr(buffer,"\r\n");
        // if(end){
        //     bytes_read -= (end - buffer);
        // }
        min += fwrite(buffer, 1, bytes_read, file);
        if(min <=0){
            perror("write error");
        }
        // remaining -= bytes_read;
        max += bytes_read;
        // if(remaining < sizeof(buffer)){
        //     leng = remaining;
        // }
    }

  
    
    // 发送响应
    printf("Status: 200 OK\r\n");
    printf("Content-Type: text/html\r\n\r\n");
    printf("<html><body><h1>File uploaded successfully %ld %ld %ld</h1></body></html>\n", content_length, max, min);
    fclose(file);
      xcam_upgrade();
    return 0;
}
