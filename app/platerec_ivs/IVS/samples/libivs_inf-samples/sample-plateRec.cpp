#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>

#include "ivs/ivs_common.h"
#include "ivs/ivs_interface.h"
#include "ivs/ivs_inf_plateRec.h"

#include <string>
#include <vector>
#include <fstream>
using namespace std;


void listdir(const std::string &folder, std::vector<std::string> &files)
{
    auto dir = opendir(folder.c_str());
    if ((dir) != NULL) {
        struct dirent *entry;
        entry = readdir(dir);
        while (entry) {
            if (strcmp(".", entry->d_name) != 0 && strcmp("..", entry->d_name) != 0) {
                files.push_back(folder + "/" + std::string(entry->d_name));
            }
            entry = readdir(dir);
        }
    }
}

void readlines(const std::string &filepath, std::vector<std::string> &files)
{
    std::ifstream ifs;
    ifs.open(filepath, std::ios::in);
    if (!ifs.is_open()) {
        printf("Open %s faild\n", filepath.c_str());
        return;
    }
    std::string buf;
	while (getline(ifs, buf)) {
        files.push_back(buf);
    }
    ifs.close();
}

void getAllFiles(const std::string &path, std::vector<std::string>& files)
{
    if (!strstr(path.c_str(), ".txt")) {
        listdir(path, files);
    } else {
        readlines(path, files);
    }
}

int main(int argc, char *argv[])
{
    const char* help = "input args:\n\
    1. img_folder_path or filepath.txt\n\
    2. img_w\n\
    3. img_h\n";

    if (argc != 4){
        printf("%s", help);
        exit(-1);
    }

    const char* folder = argv[1];
    int image_w = atoi(argv[2]);
    int image_h = atoi(argv[3]);
    std::vector<std::string> files;
    getAllFiles(folder, files);
    if (files.size() == 0) {
        printf("Can't find any file\n");
        exit(0);
    }

    //check ivs version
    //PLATEREC_VERSION_NUM defined in ivs_inf_platerec.h .
    uint32_t platerec_ver = platerec_get_version_info();
    if(platerec_ver != PLATEREC_VERSION_NUM){
        printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n",PLATEREC_VERSION_NUM,platerec_ver);
        return -1;
    }

    platerec_param_input_t param;
    memset(&param, 0, sizeof(platerec_param_input_t));
    param.frameInfo.width = image_w; //recommend 720P
    param.frameInfo.height = image_h;
    param.skip_num = 0;      //skip num
    param.max_vehicle_box = 10;      //real recgonit width
    param.max_plate_box = 10;      //real recgonit width
    param.rot90 = false;
    param.sense = 1;
    param.detdist = 0;
    param.switch_track = true;
    param.switch_plate_rec = true;
    param.switch_plate_vehtype = false;
    param.switch_plate_vehcolor = false;
    param.switch_plate_placolor = true;
    param.switch_plate_platype = true;
    param.enable_move = false;
    param.model_path = "model/plate_det.bin";
    param.model_path_ldmk = "model/plate_ldmk.bin";
    param.model_path_vehtype = "";
    param.model_path_vehcolor = "";
    param.model_path_placolor = "model/plate_color.bin";
    param.model_path_platype = "model/plate_type.bin";
    param.model_path_rec = "model/plate_rec.bin";
    param.ptime = false;
    // param.min_reg_height

    // init
    IMPIVSInterface *interface = PlateRecInterfaceInit(&param);
    int ret = interface->init(interface);

    // alloc mem
    int nv12_size = image_w * image_h * sizeof(uint8_t) * 1.5;
    uint8_t* nv12_data = (uint8_t*) malloc(nv12_size);

    IMPFrameInfo nv12_frame;
    nv12_frame.width = image_w;
    nv12_frame.height = image_h;
    nv12_frame.virAddr = (uint32_t)nv12_data;
    nv12_frame.pixfmt = PIX_FMT_NV12;
    nv12_frame.size = nv12_size;

    platerec_param_output_t* result;

    FILE *fw_handle = fopen("result.txt", "w");
    char txtbuff[1000];

    for (auto filepath: files)
    {
        const char* path = filepath.c_str();
        printf("------> %s\n", path);
        FILE * fp = fopen(path, "rb");
        if(fp){
            ret = fread(nv12_data, 1, nv12_size, fp);
            fclose(fp);
        }else{
            printf("open nv12 file failed\n");
            return -1;
        }
#if 1
        struct timeval tv;
        gettimeofday(&tv, NULL);
        uint64_t time_last = tv.tv_sec*1000000 + tv.tv_usec;
#endif  
        interface->preProcessSync(interface, &nv12_frame);
        interface->processAsync(interface, &nv12_frame);
        interface->getResult(interface, (void**)&result);
#if 1
        gettimeofday(&tv, NULL);
        time_last = tv.tv_sec*1000000 + tv.tv_usec - time_last;
        uint64_t time_ms = time_last*1.0/1000;
        printf("plate recognit time : %llums\n", time_ms);
#endif
        sprintf(txtbuff, "[filename]%s ", path);
        fputs(txtbuff, fw_handle);
        for(int k = 0; k < result->vehicle_count; k++) {
            IVSRect* r = &(result->vehicle[k].show_box);
            float conf = result->vehicle[k].confidence;
            int vehicle_type= result->vehicle[k].type;
            int vehicle_color= result->vehicle[k].color;
            int track_id = result->vehicle[k].track_id;
            sprintf(txtbuff, "[vehicle]%d,%d,%d,%d ", r->ul.x, r->ul.y, r->br.x, r->br.y);
            fputs(txtbuff, fw_handle);
            printf("%s\n", txtbuff);
        }
        for(int k = 0; k < result->plate_count; k++) {
            IVSRect* r = &(result->plate[k].show_box);
            const char *licence = result->plate[k].licence;
            int id = result->plate[k].track_id;
            int plate_color= result->plate[k].color;
            int plate_type= result->plate[k].type;
            float conf = result->plate[k].confidence;

            sprintf(txtbuff, "[plate]%d,%d,%d,%d ", r->ul.x, r->ul.y, r->br.x, r->br.y);
            fputs(txtbuff, fw_handle);
            printf("%s", txtbuff);

            sprintf(txtbuff, "[licence]%s [id]%d [conf]%.2f [type]%d [color]%d", licence, id, conf, plate_type, plate_color);
            fputs(txtbuff, fw_handle);
            printf("%s\n", txtbuff);
        }
        fputs("\n", fw_handle);
        printf("\n");

        interface->releaseResult(interface, (void**)&result);
    }

    // release
    interface->exit(interface);
    PlateRecInterfaceExit(interface);
    free(nv12_data);
    fclose(fw_handle);

    return 0;
}
