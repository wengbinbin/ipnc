/*
 * Main header file 
 *
 */

#ifndef __PSIA_PSIA_H__
#define __PSIA_PSIA_H__

/* each service or resource supported by the device is assigned a node_t structure */
typedef struct node_t
{
#define NODE_MARKER 0x4E4F4445
	unsigned long	marker;
	unsigned long	count;

	int		    len;
	char		*name;
	const char	*version;
	int		    methods;
	int		    type;
   #define NODE_STD	        0x01
   #define NODE_SERVICE	    0x02
   #define NODE_RESOURCE	0x03

	/* description strings */
	const char	*get_query_str;
	const char	*get_inbound_str;
	const char	*get_function_str;
	const char	*get_result_str;
	const char	*get_notes_str;

	const char	*put_query_str;
	const char	*put_inbound_str;
	const char	*put_function_str;
	const char	*put_result_str;
	const char	*put_notes_str;

	const char	*post_query_str;
	const char	*post_inbound_str;
	const char	*post_function_str;
	const char	*post_result_str;
	const char	*post_notes_str;

	const char	*del_query_str;
	const char	*del_inbound_str;
	const char	*del_function_str;
	const char	*del_result_str;
	const char	*del_notes_str;

	int		(*process)(void *, void *);
	struct	node_t	*first_child;
	struct	node_t	*next;
	struct	node_t	*parent;
}node_t;

/* each element in the inbound xml data is assigned a tag_t structure */
typedef struct tag_t
{
	#define TAG_MARKER 0x54414754
	unsigned long	marker;
	unsigned long	count;
	int		        empty;		// empty flag

	char	*name;
	char	*value;
	char	*attr;

	char	*start_name;
	char	*value_name;
	char	*end_name;
	char	*attr_name;

	int		start_len;
	int		value_len;
	int		end_len;
	int		attr_len;

	struct	tag_t	*first_child;
	struct	tag_t	*next;
	struct	tag_t	*parent;
}tag_t;

#define STATUS_OK			        1
#define STATUS_DEVICE_BUSY		    2
#define STATUS_DEVICE_ERROR		    3
#define STATUS_INVALID_OPERATION	4
#define STATUS_INVALID_XML_FORMAT	5
#define STATUS_INVALID_XML_CONTENT	6
#define STATUS_REBOOT_REQUIRED		7
#define STATUS_NOT_IMPLEMENTED		8

extern const char *PrologString;
extern const char *RootAttrStr;
extern const char *ResourceListAttrStr;
extern const char *IndexAttrStr;
extern const char *ResourceDescriptionAttrStr;

extern const char *SystemAttrStr;
extern const char *SystemRebootAttrStr;
extern const char *SystemUpdateAttrStr;
extern const char *SystemConfigAttrStr;
extern const char *SystemResetAttrStr;
extern const char *SystemDeviceInfoAttrStr;
extern const char *SystemReportAttrStr;
extern const char *SystemStatusAttrStr;
extern const char *SystemTimeAttrStr;
extern const char *SystemLoggingAttrStr;

extern const char *VideoAttrStr;
extern const char *VideoInputAttrStr;

extern const char *NetworkAttrStr;
extern const char *NetworkInterfaceAttrStr;

extern const char *SecurityAttrStr;
extern const char *SecurityUsersAttrStr;

extern const char *StreamingAttrStr;
extern const char *StreamingStatusAttrStr;
extern const char *StreamingChannelAttrStr;

extern const char *MotionAttrStr;
extern const char *MotionIDAttrStr;

extern const char *StatusString[];

/* global debug enable */
#ifdef PSIA_DEBUG
#include <syslog.h>
#define debug_printf(FMT...) syslog (LOG_INFO, FMT)
#else
#define debug_printf(FMT...)
#endif

#include "boa.h"

/* xparse.c */
extern int xml_validate(struct request *req, char *xstr, int len);
extern unsigned long xml_required_field(int bit);
extern unsigned long xml_required_mask(int count);
extern int xml_init(struct request *req);
extern void xml_cleanup(struct request *req);

/* psia_root.c */
extern void add_std_resources(node_t *parent, void *fn_index, void *fn_indexr, void *fn_description, void *fn_capabilities);
extern int process_psia_request(struct request *req);
extern int clean_psia_request(struct request *req, node_t *root);

/* tree.c */
extern node_t *add_node(node_t *parent, const char *name, int methods, int type, const char *version, void *fn);
extern void delete_tree(node_t *node);
extern int build_tree_error(void);
extern int process_branch(struct request *req, node_t *parent);

/* index.c */
//extern int process_index(struct request *req, node_t *me);
//extern int process_indexr(struct request *req, node_t *me);

/* description.c */
extern void desc_get_query(node_t *node, const char *str);
extern void desc_get_inbound(node_t *node, const char *str);
extern void desc_get_function(node_t *node, const char *str);
extern void desc_get_result(node_t *node, const char *str);
extern void desc_get_notes(node_t *node, const char *str);
extern void desc_put_query(node_t *node, const char *str);
extern void desc_put_inbound(node_t *node, const char *str);
extern void desc_put_function(node_t *node, const char *str);
extern void desc_put_result(node_t *node, const char *str);
extern void desc_put_notes(node_t *node, const char *str);
extern void desc_post_query(node_t *node, const char *str);
extern void desc_post_inbound(node_t *node, const char *str);
extern void desc_post_function(node_t *node, const char *str);
extern void desc_post_result(node_t *node, const char *str);
extern void desc_post_notes(node_t *node, const char *str);
extern void desc_delete_query(node_t *node, const char *str);
extern void desc_delete_inbound(node_t *node, const char *str);
extern void desc_delete_function(node_t *node, const char *str);
extern void desc_delete_result(node_t *node, const char *str);
extern void desc_delete_notes(node_t *node, const char *str);
//extern int process_description(struct request *req, node_t *me);

/* services */
//int process_system(struct request *req, node_t *parent);
//int process_streaming(struct request *req, node_t *parent);
//int process_security(struct request *req, node_t *parent);

/* xadd.c */
extern int xadd_init(struct request *req);
extern void xadd_text(struct request *req, const char *fmt, ...);
extern void xadd_stag(struct request *req, const char *tag);
extern void xadd_stag_attr(struct request *req, const char *tag, const char *fmt, ...);
extern void xadd_etag(struct request *req, const char *tag);
extern void xadd_elem(struct request *req, const char *tag, const char *val);
extern void xadd_flush(struct request *req);
extern void xadd_cleanup(struct request *req);

#endif /* __PSIA_PSIA_H__ */
