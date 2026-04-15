#!/usr/bin/python3
import sys
import os
import json
import collections
import struct
import binascii
import zlib

from datetime import datetime
from ctypes import create_string_buffer

chip = "t31"
sensor = "gc5613"

def main():
    if len(sys.argv) < 1:
        print(sys.argv[0]+" xxxx.json"+ " [pack|make]")
    else:
        if "t21" == sys.argv[2]:
            chip = "t21"
            print("chip is t21")
        elif "t40" == sys.argv[2]:
            chip = "t40"
            print("chip is t40")
        elif "t41" == sys.argv[2]:
            chip = "t41"
            print("chip is t41")
        elif "PRJ007" == sys.argv[2]:
            chip = "PRJ007"
            print("chip is PRJ007")
        else:
            chip = "t31"
            print("chip is t31")
        conf = json.load(open(sys.argv[1]),
                object_pairs_hook=collections.OrderedDict)
        print("---conf---")
        print(json.dumps(conf, sort_keys=False, indent=4))
        if 0 != pack(conf):
            print("error: pack failed!")

def system(dir, cmd):
    cmd = "cd "+dir+";"+cmd
    if 0 != os.system(cmd):
        print("error: cmd failed!"); return -1
    return 0

def pack_copy_files(conf, srcdir, dstdir):
    print("---start copy files---")
    for item in conf["copy.files"]:
        srcfile = item["src"]
        dstfile = dstdir+"/"+item["dst"]
        print(dstfile)
        if 0 != system(".", "cp "+srcfile+" "+dstfile+" -r"):
            print("error: cp failed!"); return -1
    for item in conf["copy.sensors"]["sensors"]:
        if sys.argv[2] == "t21":
            srcfile = conf["copy.sensors"]["src_drv"]+"/sensor_"+item+"_t21.ko"
            print("cp t21 sensor_ ko")
        elif sys.argv[2] == "t31":
            srcfile = conf["copy.sensors"]["src_drv"]+"/sensor_"+item+"_t31.ko"
            print("cp t31 sensor_ ko")
        elif sys.argv[2] == "t40":
            srcfile = conf["copy.sensors"]["src_drv"]+"/sensor_"+item+"_t40.ko"
            print("cp t40 sensor_ ko")
        elif sys.argv[2] == "t41":
            srcfile = conf["copy.sensors"]["src_drv"]+"/sensor_"+item+"_t41.ko"
            print("cp t41 sensor_ ko")
        elif sys.argv[2] == "PRJ007":
            srcfile = conf["copy.sensors"]["src_drv"]+"/sensor_"+item+"_PRJ007.ko"
            print("cp PRJ007 sensor_ ko")
        else:
            print("error: the chip is not support!!!!")
        dstfile = dstdir+"/"+conf["copy.sensors"]["dst_drv"]
        if 0 != system(".", "cp "+srcfile+" "+dstfile+" -r"):
            print("error: cp failed!"); return -1
        if sys.argv[2] == "t21":
            srcfile = conf["copy.sensors"]["src_setting"]+"/"+item+"-t21.bin"
            print("cp t21 sensor sensor_setting")
        elif sys.argv[2] == "t31":
            srcfile = conf["copy.sensors"]["src_setting"]+"/"+item+"-t31.bin"
            print("cp t31 sensor sensor_setting")
        elif sys.argv[2] == "t40":
            srcfile = conf["copy.sensors"]["src_setting"]+"/"+item+"-t40.bin"
            print("cp t40 sensor sensor_setting")
        elif sys.argv[2] == "t41":
            srcfile = conf["copy.sensors"]["src_setting"]+"/"+item+"-t41.bin"
            print("cp t41 sensor sensor_setting")
        elif sys.argv[2] == "PRJ007":
            srcfile = conf["copy.sensors"]["src_setting"]+"/"+item+"-PRJ007-day.bin"
            print("cp PRJ007 sensor sensor_setting")
        else:
            print("error: the chip is not support!!!!")
        dstfile = dstdir+"/"+conf["copy.sensors"]["dst_setting"]
        if 0 != system(".", "cp "+srcfile+" "+dstfile+" -r"):
            print("error: cp failed!"); return -1
        if sys.argv[2] == "PRJ007":
            srcfile = conf["copy.sensors"]["src_setting"]+"/"+item+"-PRJ007-night.bin"
            if 0 != system(".", "cp "+srcfile+" "+dstfile+" -r"):
                print("error: cp night bin failed!"); return -1
        #print(item)
    return 0

def file_combine(dstfile, srcfile, src_padto):
    srcsize = os.path.getsize(srcfile)
    if srcsize > src_padto:
        print("error: file combine, src_padto size is to samll", srcsize, src_padto)
        print(srcfile)
        return -1;
    padsize = src_padto - srcsize
    dst = open(dstfile, "ab")
    zero = open("/dev/zero", "rb")
    src = open(srcfile, "rb")
    dst.write(src.read(srcsize))
    dst.write(zero.read(padsize))
    dst.close()
    zero.close()
    src.close()
    return 0;

def file_pad(dstfile, padsize):
    dst = open(dstfile, "ab")
    zero = open("/dev/zero", "rb")
    dst.write(zero.read(padsize))
    dst.close()
    zero.close()
    return 0;

def file_over_write(dstfile, data, offset):
    dst = open(dstfile, "ab")
    dst.seek(offset)
    dst.write(data)
    dst.close()
    return 0

def mkfs_jffs2(name, dir, size):
    global sensor
    padName = dir[:-5] + dir.split("/")[2]
    print(padName)
    if True != os.path.isdir(dir) and '' == os.path.basename(dir):
        print("error: mkfs jffs2, please input dir {}".format(dir));return -1
    cmdstr = "mkfs.jffs2 -o {} -r {} -e 0x8000 -s 0x1000 -n -l -X zlib --pad={}".format(padName+".appfs", dir, size)
    print(cmdstr)
    if 0 != system(".", cmdstr):
        print("error: mkfs.jffs2 failed!"); return -1
    cmdstr = "mkfs.jffs2 -o {} -r {} -e 0x8000 -s 0x1000 -n -l -X zlib".format(name, dir)
    print(cmdstr)
    if 0 != system(".", cmdstr):
        print("error: mkfs.jffs2 failed!"); return -1
    return 0

def gen_all_partition(conf, ver_name):
    print("---start gen all part---")
    version_dir = "../_release/"+ver_name
    name = "{}/{}".format(version_dir,
            conf["partitions"]["ipc"]["file"])
    dir = os.path.splitext(name)[0]
    size = conf["partitions"]["ipc"]["size"]*1024
    mkfs_jffs2(name, dir, size)
    return 0

def pack_all_partition(conf, ver_name):
    global sensor
    print("---start pack all---")
    version_dir = "../_release/"+ver_name
    print(sensor)
    ver_file_name = ver_name  + "_all.img"
    dst = version_dir+"/"+ver_file_name 
    print(dst)
    for part in conf["partitions"]:
        name = conf["partitions"][part]["file"]
        if name == "ipc.bin":
            name = name+".pad"
        src = version_dir+"/"+name
        size = conf["partitions"][part]["size"]*1024
        if '' == name:
            file_pad(dst, size)
        else:
            file_combine(dst, src, size)
    return 0
def pack_packages(conf, ver_name):
    print("---start pack packages---")
    version_dir = "../_release/"+ver_name
    for item in conf["packages"]:
        tsize = 0
        vname = item["name"]
        parts = item["parts"]
        ver_file_name = "{}_pack_{}.img".format(ver_name, vname)
        dst = version_dir+"/"+ver_file_name
        print(dst)
        list_num = 0
        header_fmt = "IHHII"
        header_size = struct.calcsize(header_fmt)
        list_fmt = "IIII"
        list_size = struct.calcsize(list_fmt)
        #pad header and lists
        list_index = 0
        content_offset = 0
        content = bytes()
        lists = bytes()
        for part in parts:
            name = conf["partitions"][part]["file"]
            if '' != name:
                list_num += 1
                src = "{}/{}".format(version_dir, name)
                part_size = conf["partitions"][part]["size"]*1024
                file_size = os.path.getsize(src)
                print("pack: {:<10} {:<45} {:<10d} {:<10d}".format(vname, name, part_size, file_size))
                offset = conf["partitions"][part]["offset"]*1024
                tsize += file_size
                with open(src, "rb") as f:
                    print(src)
                    content += f.read()
                    f.close()
                lists += struct.pack("<"+list_fmt, offset, content_offset, part_size, file_size)
                list_index += 1
                content_offset += file_size
        lists_size = list_size*list_num
        # plus 4 is crc32 information
        tsize += (header_size+4)+lists_size
        header = struct.pack("<"+header_fmt, conf["project"],
            conf["version"]["major"], conf["version"]["minor"],
            tsize, list_num)
        crc32 = binascii.crc32(header+lists+content)
        print(crc32)
        header += struct.pack("<i", crc32)
        print("crc32:", hex(crc32), dst)
        with open(dst, "wb") as f:
            f.write(header)
            f.write(lists)
            f.write(content)
            f.close()
        if os.path.getsize(dst) != tsize:
            print("error: size error", os.path.getsize(dst), tsize)
            return -1
    return 0
def xcreate_version_file(version_dir, version_name, version_des):
    versor_file_name = version_dir + "/ipc/.version"
    xcreate_version_craete_cmd = "echo " + version_name + ": " + version_des + " > " + versor_file_name
    # print(xcreate_version_craete_cmd)
    if( 0 != system(".", xcreate_version_craete_cmd) ):
        print("error: xcreate_version_file failed!") 
        return -1
    return 0
def pack(conf):
    global sensor
    print("---start pack---")
    sensor = conf["copy.sensors"]["sensors"][0]
    version_name = "XCAMERA_"+ conf["chip"] + conf["board"] + "_" + sensor.upper() + "_" \
        + conf["sensor_num"] + "_" + conf["type"] + "_" + datetime.now().strftime('%Y%m%d%H%M%S')
    version_dir = "../_release/"+version_name
    print(version_name, version_dir)
    sensor_bin_dir = "../_release/" + version_name + "/ipc/etc/sensor"
    lib_modules_dir = "../_release/" + version_name + "/ipc/lib/modules"
    system_file = "../_release/" + version_name + "/ipc/.system"
    versor_file = "../_release/" + version_name 
    model_dir = "../_release/" + version_name + "/ipc/model"

    print(sensor)
    print(model_dir)
    if 0 != system(".", "mkdir -p "+version_dir) or 0 != system(".", "mkdir -p "+lib_modules_dir)  \
       or 0 != system(".", "mkdir -p "+sensor_bin_dir) or  0 != system(".", "touch "+ system_file) \
       or  0 != system(".", "mkdir -p "+model_dir):
        print("error: mkdir failed!"); return -1
    if( xcreate_version_file(version_dir, version_name, conf['version_des']) != 0 ):
        return -1
    if 0 != pack_copy_files(conf, ".", version_dir):
        print("error: pack failed!"); return -1
    if 0 != gen_all_partition(conf, version_name):
        print("error: pack all failed!"); return -1
    if 0 != pack_all_partition(conf, version_name):
        print("error: pack all failed!"); return -1
    if 0 != pack_packages(conf, version_name):
        print("error: pack all failed!"); return -1
    return 0
if __name__ == "__main__":
    main()
