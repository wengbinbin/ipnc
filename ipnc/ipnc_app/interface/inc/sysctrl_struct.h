/* ===========================================================================
* @path $(IPNCPATH)\include
*
* @desc
* .
* Copyright (c) Appro Photoelectron Inc.  2008
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied
*
* =========================================================================== */
#ifndef __SYS_CONTROL_STRUCT_H__
#define __SYS_CONTROL_STRUCT_H__

/* Date    2008/03/13 */
/* Version 1.00.01    */
#define USR_LEN		33
#define PW_LEN		33
#define IP_LEN	20///< IP string length

typedef unsigned short upnp_port_t;
typedef struct{
	char id[USR_LEN];
	char password[PW_LEN];
	char remote_ip[IP_LEN];
}login_data_t;
typedef struct{
	char user_id[USR_LEN];
	int	authority;
}get_user_authority_t;
typedef struct{
	char	user_id[USR_LEN];
	char	password[PW_LEN];
	unsigned char	authority;
}add_user_t;
#endif   /* __SYS_CONTROL_STRUCT_H__ */
