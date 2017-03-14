/*
 * Copyright (C) 2006 The Android Open Source Project
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

#ifndef __SOCKET_LOCAL_H
#define __SOCKET_LOCAL_H
#define ANDROID_SOCKET_NAMESPACE_ABSTRACT (0)
#define ANDROID_SOCKET_NAMESPACE_RESERVED (1)
#define ANDROID_SOCKET_NAMESPACE_FILESYSTEM (2)
#define FILESYSTEM_SOCKET_PREFIX "/tmp/"
#define ANDROID_RESERVED_SOCKET_PREFIX "/dev/socket/"
#include <sys/socket.h>

/**
 * @brief socket_local_client / create domain socket client
 *
 * @param name:same as linux domain socket name
 * @param namespaceId:
 * @namespaceid:the kind of socket you can choose
 *              ANDROID_SOCKET_NAMESPACE_ABSTRACT:the socket will exist in linux abstract namespace
 *              ANDROID_SOCKET_NAMESPACE_RESERVED:A socket in the Android reserved namespace in /dev/socket but we not support
 *              as I see it, androw make in init.rc ,It is file only the domain socket file floder.under the dev/socket there 
 *              is some system service like rild .
 *		ANDROID_SOCKET_NAMESPACE_FILESYSTEM this socket wile exist in the filesystem
 * @param type:linux socket type eg. SOCK_STREAM
 *
 * @return socket descriptor
 */
#ifdef __cplusplus
extern "C" {
#endif
int socket_local_client(const char *name, int namespaceId, int type);

/**
 * @brief socket_local_server / create domain socket server
 *
 * @param name:same as socket_local_client
 * @param namespaceid:same as socket_local_client
 * @param type:same sa socket_local_client
 *
 * @return socket descriptor
 */
int socket_local_server(const char *name, int namespaceid, int type);
#ifdef __cplusplus
}
#endif
#endif
