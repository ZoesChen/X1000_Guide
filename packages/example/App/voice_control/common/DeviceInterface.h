#ifndef __DEVICE_INTERFACE_H__
#define __DEVICE_INTERFACE_H__

#define ACTION_TYPE_ACK_OK "ack_ok"
#define ACTION_TYPE_DEFAULT_ACK "default_ack"
#define ACTION_TYPE_REGISTER "register"
#define ACTION_TYPE_REGISTER_ACK "register_ack"
#define ACTION_TYPE_UPLOAD_ACTION "upload_action"
#define ACTION_TYPE_UPDATE_ACTION "update_action"
#define ACTION_TYPE_SPEECH_COMMAND "speech_command"
#define ACTION_TYPE_QUERY_ONLINE_CLIENT "query_online_client"
#define ACTION_TYPE_QUERY_ONLINE_CLIENT_ACK "query_online_client_ack"
#define ACTION_TYPE_SHUTDOWN "shutdown" /* client less power, say goodbye */
/* #define ACTION_TYPE_ "" */

#define ACTION_TYPE_STRING "action_type"
#define CLIENT_HEADER_STRING "client_header"
//#define ACTION_OPERATE_STRING "action"
#define ACTION_OPERATE_STRING "operaters"

/* client_header */
#define CLIENT_NAME_STRING "client_name"
#define CLIENT_ID_STRING "client_id"


#define CLIENT_NAME_SPEECH "speech_client"
#define CLIENT_NAME_LIGHT "light"
#define CLIENT_NAME_AIRCONDITIONER "air_conditioner"


#define SPEECH_OPERATE_STRING "speech_command"


#endif	/* __DEVICE_INTERFACE_H__ */
