/*
 * Copyright (C) 2011 The Android Open Source Project
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

#define LOG_TAG "ProcessCallStack_test"

#include <stdio.h>
#include <utils/ProcessCallStack.h>
#include <utils/CallStack.h>
#include <dlog.h>
#include <fcntl.h>

namespace android {
}

int function()
{
	int a = 1;
	int b = 2;
	printf("a+b=%d\n",a+b);
	return 0;
}

int test()
{
	function();
	printf("test\n");
	return 0;
}

int main()
{
	printf("start processcallstack test\n");
	android::ProcessCallStack ptest ;
	ptest.update();
	ptest.log("ProcessCallStack_test",DLOG_INFO,"test ProcessCallStack");
	ptest.toString("ProcessCallStack_test");
	int fd = open("/usr/process",O_RDWR);
	if(fd < 0)
		printf("error\n");
	ptest.dump(fd,0,"ProcessCallStack_test");
	printf("size:%d\n",ptest.size());
	test();
	return 0;
}
