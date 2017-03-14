
import sys
import math
import os
import glob
import time
import shutil

g_image_dir = "src_image"
g_xImage = "xImage"
g_update = "update"
g_user = "user"
g_fs_postfix = "fs.cramfs"
g_info_postfix = "info"
g_zip_postfix = ".zip"
g_script_postfix = ".script"
g_script_tmpfile = "/tmp/tmpfile"

g_split_xImage = "xImage"
g_split_updatefs = "updatefs"
g_split_userfs = "userfs"
g_tmp_todir = "split"
g_tmp_updatezip = "updatezip"

g_signe_dir = "signapk"
g_signe = "signapk.jar"

g_par_name = "partitions.tab"
g_par_updatefs = "updatefs"
g_par_userfs = "userfs"
g_par_updatefs_size = 0
g_par_userfs_offset = 0
g_par_userfs_size = 0
g_par_userfs_charname = ""
g_par_userfs_blockname = ""
g_kernel_base_userfs_offset = 1024*1024

g_split_xImage_num = 0
g_split_updatefs_num = 0
g_split_userfs_num = 0
g_split_total_num = 0
g_xImage_size = 0
g_updatefs_size = 0
g_userfs_size =0
g_base_size = 1024

g_board_config = "board_config.mk"
g_server_config = "server_config.mk"
g_ota_url = "CONFIG_OTA_URL"
g_server_url = "CONFIG_SERVER_URL"
g_private_key = "CONFIG_PRIVATE_KEYS"
g_public_key = "CONFIG_PUBLIC_KEYS"

def create_dir(dirname):
    if not os.path.exists(dirname):#check whether todir exists or not
        os.mkdir(dirname)

def delete_file_folder(file):
    if os.path.isfile(file):
        try:
            os.remove(file)
        except:
            pass
    elif os.path.isdir(file):
        for item in os.listdir(file):
            itemsrc=os.path.join(file,item)
            delete_file_folder(itemsrc)
        try:
            os.rmdir(file)
        except:
            pass

def delete_old_update():
    for file in glob.glob(r'./update_*'):
        delete_file_folder(file)

def generate_update_info(update_info, cur_time, url, mode):
    info_fd = open(update_info,'a+')
    buf = "url=%s\n" %(url)
    info_fd.write(buf)
    buf = "version=%s\n" %(cur_time)
    info_fd.write(buf)
    buf = "mode=%r\n" %(mode)
    info_fd.write(buf)
    buf = "kernel_offset=%r\n" %(g_kernel_base_userfs_offset)
    info_fd.write(buf)
    info_fd.close()

def is_existed(file):
    return os.path.isfile(file)
def check_update_image(imagepath):
    is_update_kernel = 0

    xImage = os.path.join(imagepath,('%s'%(g_xImage)))
    if is_existed(xImage) == False:
        print "not found", xImage
    else:
        is_update_kernel = 1
    update_fs = os.path.join(imagepath,('%s_%s'%(g_update, g_fs_postfix)))
    if is_existed(update_fs) == False:
        print "not found", update_fs
    else:
        is_update_kernel = 10 + is_update_kernel
    user_fs = os.path.join(imagepath,('%s_%s'%(g_user, g_fs_postfix)))
    if is_existed(user_fs) == False:
        print "not found", user_fs
    else:
        is_update_kernel = 100 + is_update_kernel


    if is_update_kernel == 111:
        return 1
    elif is_update_kernel == 100:
        return 0
    else:
        return -1

def split(fromfile,todir,toname, chunksize=1024*1024):
    partnum = 0
    inputfile = open(fromfile,'rb')
    while True:
        chunk = inputfile.read(chunksize)
        if not chunk:
            break
        filename = os.path.join(todir,('%s%02d'%(toname,partnum)))
        fileobj = open(filename,'wb')
        fileobj.write(chunk)
        fileobj.close()
        partnum += 1

    return partnum

def split_image(imagepath, split_tmp_dir, mode, chunksize):
    global g_split_xImage_num
    global g_split_updatefs_num
    global g_split_userfs_num
    global g_split_total_num

    global g_xImage_size
    global g_updatefs_size
    global g_userfs_size

    if mode == 1:
        xImage = os.path.join(imagepath,('%s'%(g_xImage)))
        g_xImage_size = os.path.getsize(xImage)
        g_split_xImage_num = split(xImage, split_tmp_dir, g_split_xImage, chunksize)
        updatefs = os.path.join(imagepath,('%s_%s'%(g_update, g_fs_postfix)))
        g_updatefs_size = os.path.getsize(updatefs)
        g_split_updatefs_num = split(updatefs, split_tmp_dir, g_split_updatefs, chunksize)
    userfs = os.path.join(imagepath,('%s_%s'%(g_user, g_fs_postfix)))
    g_userfs_size = os.path.getsize(userfs)
    g_split_userfs_num = split(userfs, split_tmp_dir, g_split_userfs, chunksize)

    g_split_total_num = g_split_xImage_num + g_split_updatefs_num + g_split_userfs_num

def get_par_info(par_name):
    global g_par_userfs_offset
    global g_par_userfs_size
    global g_par_userfs_charname
    global g_par_userfs_blockname
    global g_par_updatefs_size

    user_tmp = []
    fd = open(par_name,'r')
    while True:
        line = fd.readline()
        if line:
            if g_par_userfs in line:
                user = line.split(":")
                user_tmp = user[1].split(",")
                for num in range(0, len(user_tmp)):
                    user_tmp[num] = filter(str.isalnum, user_tmp[num])
                    g_par_userfs_offset = user_tmp[0]
                    g_par_userfs_size = user_tmp[1]
                    g_par_userfs_charname = user_tmp[2]
                    g_par_userfs_blockname = user_tmp[3]
            elif g_par_updatefs in line:
                user = line.split(":")
                user_tmp = user[1].split(",")
                for num in range(0, len(user_tmp)):
                    if num == 1:
                        tmp = filter(str.isalnum, user_tmp[num])
                        g_par_updatefs_size = int(tmp, 16) / pow(g_base_size, 2)
                        break
        else:
            break
    fd.close()
    if g_par_userfs_size == 0 or g_par_updatefs_size == 0:
        return None
    return user_tmp

def create_script(scriptname, num):
    inputfile = open(scriptname,'a+')
    buf = "echo sum=%d > %s\n" %(g_split_total_num, g_script_tmpfile)
    inputfile .write(buf)
    if num  + 1 == g_split_xImage_num + g_split_updatefs_num:
        buf = "echo reboot=1 >> %s\n" %(g_script_tmpfile)
    else:
        buf = "echo reboot=0 >> %s\n" %(g_script_tmpfile)
    inputfile .write(buf)
    buf = "echo kernel_size=%d >> %s\n" %(g_xImage_size, g_script_tmpfile)
    inputfile .write(buf)
    buf = "echo update_size=%d >> %s\n" %(g_updatefs_size, g_script_tmpfile)
    inputfile .write(buf)
    buf = "echo usrfs_size=%d >> %s\n" %(g_userfs_size, g_script_tmpfile)
    inputfile .write(buf)

    if num == g_split_xImage_num + g_split_updatefs_num:
        buf = "mtd_debug erase /dev/%s 0 %s\n" \
              %(g_par_userfs_charname, g_par_userfs_size)
        inputfile .write(buf)

    buf = "dd if=/tmp/%s/%s%d" %(g_tmp_updatezip, g_tmp_updatezip, num)
    if num < g_split_xImage_num:
        image_num = num
        buf = "%s/%s%02d" %(buf, g_split_xImage, image_num)
        write_offset = (image_num  + g_par_updatefs_size) * g_base_size
        write_offset = write_offset + g_kernel_base_userfs_offset / g_base_size
    elif num < g_split_xImage_num + g_split_updatefs_num:
        image_num = num - g_split_xImage_num
        buf = "%s/%s%02d" %(buf, g_split_updatefs, image_num)
        write_offset = image_num * g_base_size
    else:
        image_num = num - g_split_xImage_num - g_split_updatefs_num
        buf = "%s/%s%02d" %(buf, g_split_userfs, image_num)
        write_offset = image_num * g_base_size

    buf = "%s of=/dev/%s seek=%d bs=%d count=%d\n"\
          %(buf, g_par_userfs_blockname, write_offset, g_base_size, g_base_size)

    inputfile .write(buf)
    inputfile .close()

def get_keys(cur_path, keyname):

    boardname = os.path.join(cur_path, ('%s'%(g_board_config)))
    key = None
    fd = open(boardname,'r')
    while True:
        line = fd.readline()
        if line:
            if keyname in line:
                key = line[line.find("=") + 1:]
                key = key.strip().lstrip('"').rstrip('"')
                break
        else:
            break

    fd.close()
    return key

def make_update(cur_path, update_dir, split_tmp_dir):

    signapk = os.path.join(cur_path, ('%s/%s'%(g_signe_dir, g_signe)))
    public_key = os.path.join(cur_path, ('%s' %(get_keys(cur_path, g_public_key))))
    private_key = os.path.join(cur_path, ('%s' %(get_keys(cur_path, g_private_key))))


    for num in range(0, g_split_total_num):
        updatezipdir = os.path.join(update_dir, ('%s%d'%(g_tmp_updatezip, num)))
        create_dir(updatezipdir)
        scriptname = os.path.join(updatezipdir, ('%s%s'%(g_update, g_script_postfix)))

        create_script(scriptname, num)
        if num < g_split_xImage_num:
            src_file = os.path.join(split_tmp_dir, ('%s%02d'%(g_split_xImage, num)))
            dst_file = os.path.join(updatezipdir, ('%s%02d'%(g_split_xImage, num)))
            shutil.copyfile(src_file, dst_file)
        elif num < g_split_xImage_num + g_split_updatefs_num:
            cur_num = num - g_split_xImage_num
            src_file = os.path.join(split_tmp_dir, ('%s%02d'%(g_split_updatefs, cur_num)))
            dst_file = os.path.join(updatezipdir, ('%s%02d'%(g_split_updatefs, cur_num)))
            shutil.copyfile(src_file, dst_file)
        else:
            cur_num = num - g_split_xImage_num - g_split_updatefs_num
            src_file = os.path.join(split_tmp_dir, ('%s%02d'%(g_split_userfs, cur_num)))
            dst_file = os.path.join(updatezipdir, ('%s%02d'%(g_split_userfs, cur_num)))
            shutil.copyfile(src_file, dst_file)

        zipname_unsigned = '%sunsigned%d%s' %(g_tmp_updatezip, num, g_zip_postfix)
        zipname = '%s%d%s' %(g_tmp_updatezip, num, g_zip_postfix)
        zip_sh = "%s %s %s%d" %("zip -r", zipname_unsigned, g_tmp_updatezip, num)
        signe = "java -Xms512M -Xmx1024M -jar %s -w %s %s %s %s" \
                %(signapk, public_key, private_key, zipname_unsigned, zipname)
        os.system(zip_sh)
        os.system(signe)

        #shutil.move(zipname, update_dir)
        delete_file_folder(zipname_unsigned)
        delete_file_folder(updatezipdir)

def get_url(urlname):

    cur_path = os.getcwd()
    configname = os.path.join(cur_path, ('%s'%(g_server_config)))
    url = None
    fd = open(configname,'r')
    while True:
        line = fd.readline()
        if (line  and ("#" not in line)):
            if urlname in line:
                url = line[line.find(":") + 1:]
                url = url.strip().lstrip('"').rstrip('"')
                break
        else:
            if "#" in line:
                continue
            break

    fd.close()
    return url

def main(ota_url, server_url):

    cur_path = os.getcwd()
    imagepath = os.path.join(cur_path, ('%s'%(g_image_dir)))
    delete_old_update()
    mode = check_update_image(imagepath)
    if mode < 0:
        print "hasn't update source image"
        exit(-1)

    cur_time = time.strftime('%Y%m%d%H%M',time.localtime(time.time()))
    update_info = os.path.join(cur_path, ('%s_%s'%(g_update, g_info_postfix)))
    generate_update_info(update_info, cur_time, ota_url, mode)

    split_tmp_dir = cur_path + g_tmp_todir
    create_dir(split_tmp_dir)
    split_size = g_base_size * g_base_size
    split_image(imagepath, split_tmp_dir, mode, split_size)

    update_dir = os.path.join(cur_path, ('%s_%s'%(g_update, cur_time)))
    create_dir(update_dir)

    par_name = os.path.join(cur_path, ('%s'%(g_par_name)))
    if get_par_info(par_name) == None:
        print "partition not found user_fs par"
        exit(-1)

    os.chdir(update_dir)
    make_update(cur_path, update_dir, split_tmp_dir)
    os.chdir(cur_path)

    delete_file_folder(split_tmp_dir)
    if server_url:
        cp_sh = "%s %s %s %s" %("scp -r", update_info, update_dir, server_url)
        os.system(cp_sh)

if __name__ == '__main__':
    ota_url = get_url(g_ota_url)
    if ota_url == None:
        print "please ota url to server_config.mk"
        print "example:"
        print "\tCONFIG_OTA_URL: \"http://192.168.1.200/ota/download-bliu\""

        exit(-1)
    server_url = get_url(g_server_url)

    main(ota_url, server_url)

__author__ = 'bliu'
