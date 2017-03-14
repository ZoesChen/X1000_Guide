/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include "log.h"
//#include "util.h"

#define DEV_NAME "/dev/watchdog"
/*this is in the util.c,we try to move it,but it nend other lib so we copy in there
 * if in the future you transplant util.c you can remove it.
 * */
static void open_devnull_stdio(void)
{
	int fd;
	static const char *name = "/dev/__null__";
	if (mknod(name, S_IFCHR | 0600, (1 << 8) | 3) == 0) {
		fd = open(name, O_RDWR);
		unlink(name);
		if (fd >= 0) {
			dup2(fd, 0);
			dup2(fd, 1);
			dup2(fd, 2);
			if (fd > 2) {
				close(fd);
			}
			return;
		}
	}
	exit(1);
}

int main(int argc, char *argv[])
{
    int fd;
    int ret;
    int interval = 10;
    int margin = 10;
    int timeout;

    open_devnull_stdio();
    klog_init();

    INFO("Starting watchdogd\n");

    if (argc >= 2)
        interval = atoi(argv[1]);

    if (argc >= 3)
        margin = atoi(argv[2]);

    timeout = interval + margin;

    fd = open(DEV_NAME, O_RDWR);
    if (fd < 0) {
        ERROR("watchdogd: Failed to open %s: %s\n", DEV_NAME, strerror(errno));
        return 1;
    }

    ret = ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
    if (ret) {
        ERROR("watchdogd: Failed to set timeout to %d: %s\n", timeout, strerror(errno));
        ret = ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
        if (ret) {
            ERROR("watchdogd: Failed to get timeout: %s\n", strerror(errno));
        } else {
            if (timeout > margin)
                interval = timeout - margin;
            else
                interval = 1;
            ERROR("watchdogd: Adjusted interval to timeout returned by driver: timeout %d, interval %d, margin %d\n",
                  timeout, interval, margin);
        }
    }

    while(1) {
        write(fd, "", 1);
        sleep(interval);
    }
}

