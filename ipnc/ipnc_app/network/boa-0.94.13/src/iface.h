/*
 * Device interface header file 
 *
 */

#ifndef __PSIA_IFACE_H__
#define __PSIA_IFACE_H__

#define MAX_USERS 10

typedef struct {	
	char name[30];
	char version[10];
	char type[30];
}Index_psia;

typedef struct {	
	char name[30];
	char version[30];
	char type[30];	
	char inboundData[30];	
	char returnResult[40];
	char function[50];
	char notes[30];
}Description_psia;

typedef struct {
	char id[10];
	char inputPort[10];
	char powerLineFrequencyMode[20];
	char whiteBalanceMode[20];
	char whiteBalanceLevel[10];
	char exposureMode[30];	
	char exposureTarget[10];
	char exposureAutoMin[10];
	char exposureAutoMax[10];
	char gainLevel[10];
	char brightnessLevel[10];
	char contrastLevel[10];
	char sharpnessLevel[10];
	char saturationLevel[10];
	char dayNightFilterType[10];	
}VideoInputChannel_psia;

typedef struct {
	char id[10];	
	char exposurePriority[30];		
	char blc[10];
	char backlight[10];
	char gbce[10];
	char histDisplay[10];
	char vsEnabled[10];
	char ldcEnabled[10];
	char sensorMode[10];
	char AWBEngine[10];
	char AWBMode[40];
	char snfltCtrl[10];
	char tnfltCtrl[10];
}CameraSetting_psia;

typedef struct {
	char id[10];
	char enabled[10];
	char videoInputID[10];	
	char timeStampEnabled[10];
	char timeFormat[20];
	char dateFormat[20];
	char datePosition[20];
	char timePosition[20];
	char backgroundColor[10];
	char fontColor[10];
	char fontSize[10];
	char displayText[10];
	char horizontalAlignType[10];
	char verticalAlignType[10];	
}TextOverlay_psia;

typedef struct {
	char id[10];
	char daylight[10];
	char timeFormat[20];
	char dateFormat[20];
	char datePosition[20];
	char timePosition[20];		
}DateTimeSetting_psia;

typedef struct {
	char id[10];
	char enabled[10];
	char videoInputID[10];
	char imageName[10];
	char positionX[10];
	char positionY[10];
	char transparentColorEnabled[10];
	char transparentColor[10];	
}ImageOverlay_psia;

typedef struct {	
	char enabled[10];	
}PrivacyMask_psia;

typedef struct {
	char id[10];
	char enabled[10];
	char audioMode[20];
	char microphoneEnabled[10];
	char microphoneSource[20];
	char microphoneVolume[10];
	char speakerEnabled[10];
	char speakerVolume[10];		
}AudioChannel_psia;

typedef struct {
	char id[10];	
	char inputVolume[10];
	char alarmLevel[10];	
}AudioSetting_psia;

typedef struct {
	char device_name[30];
	char device_id[50];
	char model[30];
	char serialNumber[30];
	char mac_address[30];
	char firmware_version[30];
	char firmware_released_date[30];
	char boot_version[30];
	char hardware_version[30];
}DeviceInfo_psia;

typedef struct {
	char totalStreamingSessions[10];	
}StreamingStatus_psia;

typedef struct {
	char ipAddress[30];
	char clientUserName[20];
	char startDateTime[10];		
	char elapsedTime[10];
	char bandwidth[10];
}StreamingSessionStatus_psia;

typedef struct {
	char id[20];
	char channelName[20];
	char rtspPortNo[20];
	char maxPacketSize[20];	
	char streamingTransport[20];
	char interfaceID[20];
	char rtpTransportType[20];
	char enabled[20];
	char disabled[20];
	char videoInputChannelID[20];
	char videoCodecType[20];
	char videoCodecSel[40];
	char videoScanType[20];
	char videoQualityControlType[20];
	char constantBitRate[20];
	char fixedQuality[20];
	char videoResolutionSel[40];
	char videoResolutionWidth[20];
	char videoResolutionHeight[20];	
	char maxFrameRate[20];
	char keyFrameInterval[20];
	char mirrorEnabled[20];	
	char audioEnabled[20];	
	char audioInputChannelID[20];
	char audioCompressionType[20];
	char audioBitRate[20];
	char audioSamplingRate[20];
	char localDisplay[20];
}StreamingChannel_psia;

typedef struct {
	char id[20];	
	char videoCodecSel[40];	
	char videoResolutionSel[40];	
	char mirrorMode[20];	
	char localDisplay[20];
	char captureLength[10];
	char dateStampEnabled[10];
	char detailInfoEnabled[10];
}VideoSetting_psia;

typedef struct {
	char id[20];	
	char clickSnapFileName[20];
	char clickSnapStorage[20];
	char demoCfg[20];
	char notificationStatus[10];
	char recordStatus[10];	
}LiveSetting_psia;

typedef struct {
	char id[10];	
	char forceIFrame[10];
	char qpInit[10];
	char qpMin[10];
	char qpMax[10];
	char meConfig[10];
	char packetSize[10];
	char umv[10];
    char intraPFrame[10];
	char ROIEnable[10];
	char positionX1[10];
	char positionY1[10];
	char width1[10];
	char height1[10];
	char positionX2[10];
	char positionY2[10];
	char width2[10];
	char height2[10];
	char positionX3[10];
	char positionY3[10];
	char width3[10];
	char height3[10];
}VideoAdvancedSetting_psia;

typedef struct {
	char current_device_time[30];
	char device_up_time[10];
	char tempSensorDescription[30];
	char temperature[10];
	char cpu_description[30];
	char cpu_utilization[10];
	char memory_description[30];
	char memory_usage[10];
	char memory_available[10];
}DeviceStatus_psia;

typedef struct {
	char timemode[20];
	char localtime[30];
	char timezone[100];
	char daylight[10];
}Time_psia;

typedef struct {
	char id[10];
	char addressingFormatType[20];
	char hostname[30];
	char ip_address[30];
	char portNo[10];
}NtpServer_psia;

typedef struct {
	char logNo[10];
	char dateTime[30];
	char severity[10];
	char message[30];
}LogMessage_psia;

typedef struct {
	char id[10];
	char ip_address[30];	
}NetworkInterface_psia;

typedef struct {
	char id[10];
	char ip_version[30];
	char addressing_type[30];
	char ip_address[30];
	char subnet_mask[30];
	char gateway[30];
	char primaryDNS[30];
	char secondaryDNS[30];
}IPAddress_psia;

typedef struct {
	char upnp_enabled[10];
	char zconf_enabled[10];
	char mcast_enabled[10];
	char ipAddress[30];	     // zeroconf mcast addr
	char portNo[10];		 // zeroconf port no
	char ttl[10];            // zeroconf ttl
}Discovery_psia;

typedef struct {
	char id[10];
	char volumeName[20];
	char volumePath[30];
	char volumeDescription[30];
	char volumeType[20];
	char storageLocation[30];
	char storageType[20];
	char capacity[20];
}StorageVolume_psia;

typedef struct {	
	char enabled[10];
	char freeSpace[20];
}StorageVolumeStatus_psia;

typedef struct {	
	char id[10];
	char fileName[30];
	char fileTimeStamp[30];
	char fileSize[30];	
}StorageFile_psia;

typedef struct {
	char id[10];
	char userName[30];	
	char password[30];	
}User_psia;

typedef struct {
	char id[10];	
	char minNameLen[10];
	char maxNameLen[10];	
	char maxPwdLen[10];
	char privilegeLevel[20]; 
}UserSetting_psia;

typedef struct {
	char id[10];
	char triggering[10];
}IOInputPort_psia;

typedef struct {
	char id[10];
	char defaultState[10];
	char outputState[10];
}IOOutputPort_psia;

typedef struct {
	char id[10];
	char enabled[10];
	char serialPortType[10];
	char duplexMode[10];
	char direction[20];
	char baudRate[10];
	char dataBits[10];
	char parityType[10];
	char stopBits[10];
}SerialPort_psia;

typedef struct {
	char id[10];
	char eventType[20];
	char eventDescription[40];		
	char inputIOPortID[10];
	char intervalBetweenEvents[20];	
}EventTrigger_psia;

typedef struct {
	char id[10];	
	char notificationMethod[40];
	char notificationRecurrence[10];		
	char notificationInterval[20];
}EventTriggerNotification_psia;

typedef struct {		
	char dayOfWeek[20];
	char beginTime[20];
	char endTime[20];
}EventSchedule_psia;

typedef struct {
	char id[10];
	char enabled[10];
	char storageLocation[30];
	char expireEnable[10];
	char numWeeks[10];	
	char delschedule[10];
}StorageSetting_psia;

typedef struct {
	char uploadVideoClipEnabled[10];
	char senderEmailAddress[30];
	char receiverEmailAddress[30];
	char attachedVideoClipEnabled[10];
	char videoClipFormatType[20];	
}EvenNotificationMethods_psia;

typedef struct {
	char id[10];
	char enabled[10];
	char inputType[20];
	char outputType[20];
	char numFilesAttach[10];
	char minFilesAttach[10];
	char maxFilesAttach[10];
	char localstorageneabled[10];	
	char ftpFileFormatType[10];
	char attachedFileFormatType[10];
	char sdFileFormatType[10];
	char alarmAudioFile[20];
}AlarmSetting_psia;

typedef struct {
	char id[10];
	char authenticationMode[10];
	char addressingFormatType[10];		
	char hostName[20];
	char ipAddress[30];
	char accountName[20];
	char password[20];	
}MailingNotification_psia;

typedef struct {
	char id[10];
	char addressingFormatType[10];	
	char ipAddress[30];
	char portNo[10];
	char userName[20];
	char password[20];
	char uploadPath[30];	
}FTPNotification_psia;

typedef struct {
	char id[10];
	char url[30];	
	char protocolType[10];
	char addressingFormatType[10];	
	char ipAddress[30];
	char portNo[10];	
}HttpHostNotification_psia;

typedef struct {
	char id[10];	
	char serialPortMode[10];
	char httpsPortNo[10];
}NetworkSetting_psia;

typedef struct {
	char id[10];
	char enabled[10];	
	char samplingInterval[10];
	char startTriggerTime[10];
	char endTriggerTime[10];
	char directionSensitivity[10];	
	char regionType[10];
	char minObjectSize[10];
	char maxObjectSize[10];
	char rowGranularity[10];
	char columnGranularity[10];
}MotionDetection_psia;

typedef struct {
	char id[10];
	char enabled[10];	
	char maskEnabled[10];
	char sensitivityLevel[10];
	char detectionThreshold[10];	
	char positionX[10];	
	char positionY[10];
}MotionDetectionRegion_psia;

typedef struct {
	char id[10];	
	char enabled[10];	
	char conflevel[10];
	char positionX[10];	
	char positionY[10];	
	char width[10];	
	char height[10];
	char maskoption[30];
}FaceDetection_psia;

typedef struct {
	char id[10];
	char enabled[30];
	char conflevel[10];
	char database[20];	
}FaceRecognition_psia;

/* Root */
extern int retrieve_process_index(request *req, node_t *node);
extern int retrieve_process_indexr(request *req, node_t *node);
extern int retrieve_process_description(request *req, node_t *node);

/* Status */
extern void add_response_status(struct request *req, node_t *node);
extern int get_http_status(struct request *req);

/* System/Storage */
extern int retrieve_system_storage_volumes(request *req, node_t *node);
extern int retrieve_system_storage_volumes_id(request *req, node_t *node);
extern int retrieve_system_storage_volumes_id_capabilities(request *req, node_t *node);
extern int retrieve_system_storage_volumes_id_status(request *req, node_t *node);
extern int update_system_storage_volumes_id_format(request *req, node_t *node);
extern int retrieve_system_storage_volumes_id_files(request *req, node_t *node);
extern int delete_system_storage_volumes_id_files(request *req, node_t *node);
extern int retrieve_system_storage_volumes_id_files_id(request *req, node_t *node);
extern int delete_system_storage_volumes_id_files_id(request *req, node_t *node);
extern int retrieve_system_storage_volumes_id_data(request *req, node_t *node);
extern int get_storage_count(void);
extern void get_storage_url(int id, char *idstr);

/* System/Network */
extern int retrieve_process_system_network_index(struct request *req, node_t *node);
extern int retrieve_process_system_network_description(struct request *req, node_t *node);
extern int retrieve_system_network_interfaces(struct request *req, node_t *node);
extern int retrieve_system_network_interfaces_id(struct request *req, node_t *node);
extern int update_system_network_interfaces_id(struct request *req, node_t *node);
extern int retrieve_system_network_interfaces_id_ipaddress(struct request *req, node_t *node);
extern int update_system_network_interfaces_id_ipaddress(struct request *req, node_t *node);
extern int retrieve_system_network_interfaces_id_discovery(struct request *req, node_t *node);
extern int update_system_network_interfaces_id_discovery(struct request *req, node_t *node);
extern int get_network_count(void);
extern void get_network_url(int id, char *idstr);
extern int get_network_index(char *idstr);

/* System/Video */
extern int retrieve_process_system_video_index(struct request *req, node_t *node);
extern int retrieve_process_system_video_description(struct request *req, node_t *node);
extern int retrieve_system_video_channels(struct request *req, node_t *node);
extern int retrieve_system_video_channels_id(struct request *req, node_t *node);
extern int update_system_video_channels_id(struct request *req, node_t *node);
extern int retrieve_system_video_channels_id_overlays(struct request *req, node_t *node);
extern int update_system_video_channels_id_overlays(struct request *req, node_t *node);
extern int delete_system_video_channels_id_overlays(struct request *req, node_t *node);
extern int retrieve_system_video_channels_id_privacymask(struct request *req, node_t *node);
extern int update_system_video_channels_id_privacymask(struct request *req, node_t *node);
extern int retrieve_system_video_channels_id_overlays_text(struct request *req, node_t *node);
extern int update_system_video_channels_id_overlays_text(struct request *req, node_t *node);
extern int create_system_video_channels_id_overlays_text(struct request *req, node_t *node);
extern int delete_system_video_channels_id_overlays_text(struct request *req, node_t *node);
extern int retrieve_system_video_channels_id_overlays_image(struct request *req, node_t *node);
extern int update_system_video_channels_id_overlays_image(struct request *req, node_t *node);
extern int delete_system_video_channels_id_overlays_image(struct request *req, node_t *node);
extern int retrieve_system_video_channels_id_overlays_text_id(struct request *req, node_t *node);
extern int update_system_video_channels_id_overlays_text_id(struct request *req, node_t *node);
extern int delete_system_video_channels_id_overlays_text_id(struct request *req, node_t *node);
extern int retrieve_system_video_channels_id_overlays_image_id(struct request *req, node_t *node);
extern int update_system_video_channels_id_overlays_image_id(struct request *req, node_t *node);
extern int delete_system_video_channels_id_overlays_image_id(struct request *req, node_t *node);
extern int retrieve_system_video_channels_id_capabilities(request *req, node_t *node);
extern int retrieve_system_video_channels_id_privacymask_capabilities(struct request *req, node_t *node);
extern int retrieve_system_video_channels_id_overlays_text_id_capabilities(struct request *req, node_t *node);
extern int retrieve_system_video_channels_id_overlays_image_id_capabilites(struct request *req, node_t *node);

/* System/Audio */
extern int retrieve_system_audio_channels(struct request *req, node_t *node);
extern int retrieve_system_audio_channels_id(struct request *req, node_t *node);
extern int update_system_audio_channels_id(struct request *req, node_t *node);
extern int retrieve_system_audio_channels_id_capabilities(request *req, node_t *node);

/* System */
extern int retrieve_process_system_index(struct request *req, node_t *node);
extern int retrieve_process_system_description(struct request *req, node_t *node);
extern int do_system_reboot(struct request *req, node_t *node);
extern int do_system_updatefirmware(struct request *req, node_t *node);
extern int retrieve_system_configurationdata(struct request *req, node_t *node);
extern int update_system_configurationdata(struct request *req, node_t *node);
extern int do_system_factory_reset(struct request *req, node_t *node);
extern int retrieve_system_deviceinfo(struct request *req, node_t *node);
extern int update_system_deviceinfo(struct request *req, node_t *node);
extern int retrieve_system_supportreport(struct request *req, node_t *node);
extern int retrieve_system_status(struct request *req, node_t *node);
extern int retrieve_system_time(struct request *req, node_t *node);
extern int update_system_time(struct request *req, node_t *node);
extern int retrieve_system_time_localtime(struct request *req, node_t *node);
extern int update_system_time_localtime(struct request *req, node_t *node);
extern int retrieve_system_time_timezone(struct request *req, node_t *node);
extern int update_system_time_timezone(struct request *req, node_t *node);
extern int retrieve_system_time_ntpservers(struct request *req, node_t *node);
extern int update_system_time_ntpservers(struct request *req, node_t *node);
extern int create_system_time_ntpservers(struct request *req, node_t *node);
extern int delete_system_time_ntpservers(struct request *req, node_t *node);
extern int retrieve_system_time_ntpservers_id(struct request *req, node_t *node);
extern int update_system_time_ntpservers_id(struct request *req, node_t *node);
extern int delete_system_time_ntpservers_id(struct request *req, node_t *node);
extern int retrieve_system_logging(request *req, node_t *node);
extern int update_system_logging(request *req, node_t *node);
extern int retrieve_system_logging_messages(request *req, node_t *node);
extern int get_ntpserver_count(void);
extern void get_ntpserver_url(int id, char *idstr);
extern int get_ntpserver_index(char *idstr);

/* Security */
extern int get_user_count(void);
extern void get_user_url(int i, char *idstr);
extern int retrieve_process_security_index(struct request *req, node_t *node);
extern int retrieve_process_security_description(struct request *req, node_t *node);
extern int retrieve_process_security_aaa_index(struct request *req, node_t *node);
extern int retrieve_process_security_aaa_description(struct request *req, node_t *node);
extern int retieve_security_aaa_users(struct request *req, node_t *node);
extern int update_security_aaa_users(struct request *req, node_t *node);
extern int create_security_aaa_users(struct request *req, node_t *node);
extern int delete_security_aaa_users(struct request *req, node_t *node);
extern int retieve_security_aaa_users_id(struct request *req, node_t *node);
extern int update_security_aaa_users_id(struct request *req, node_t *node);
extern int delete_security_aaa_users_id(struct request *req, node_t *node);
extern int retrieve_security_aaa_users_id_capabilities(request *req, node_t *node);

/* Streaming */
extern int get_channel_count(void);
extern void get_channel_url(int id, char *idstr);
extern int get_channel_index(char *idstr);
extern int retrieve_process_streaming_index(struct request *req, node_t *node);
extern int retrieve_process_streaming_description(struct request *req, node_t *node);
extern int retrieve_streaming_status(struct request *req, node_t *node);
extern int retrieve_streaming_channels(struct request *req, node_t *node);
extern int retrieve_streaming_channels_id(struct request *req, node_t *node);
extern int retrieve_streaming_channels_id_status(struct request *req, node_t *node);
extern int retrieve_streaming_channels_id_http(struct request *req, node_t *node);
extern int retrieve_streaming_channels_id_picture(struct request *req, node_t *node);
extern int retrieve_streaming_channels_id_http_capabilities(struct request *req, node_t *node);
extern int retrieve_streaming_channels_id_picture_capabilities(struct request *req, node_t *node);

/* System/IO */
extern int retrieve_system_io(struct request *req, node_t *node);
extern int retrieve_system_io_inputs(struct request *req, node_t *node);
extern int retrieve_system_io_inputs_id(struct request *req, node_t *node);
extern int update_system_io_inputs_id(struct request *req, node_t *node);
extern int retrieve_system_io_outputs(struct request *req, node_t *node);
extern int retrieve_system_io_inputs_id_capabilities(request *req, node_t *node);
extern int retrieve_system_io_outputs_id(struct request *req, node_t *node);
extern int update_system_io_outputs_id(struct request *req, node_t *node);
extern int retrieve_system_io_outputs_id_capabilities(request *req, node_t *node);
extern int get_port_count(void);

/* System/Serial */
extern int retrieve_system_serial_ports(struct request *req, node_t *node);
extern int retrieve_system_serial_ports_id(struct request *req, node_t *node);
extern int update_system_serial_ports_id(struct request *req, node_t *node);
extern int retrieve_system_serial_ports_id_capabilities(request *req, node_t *node);

/* Custom/Event */
extern int get_event_count(void);
extern int retrieve_custom_event(struct request *req, node_t *node);
extern int update_custom_event(struct request *req, node_t *node);
extern int retrieve_custom_event_triggers(struct request *req, node_t *node);
extern int update_custom_event_triggers(struct request *req, node_t *node);
extern int create_custom_event_triggers(struct request *req, node_t *node);
extern int delete_custom_event_triggers(struct request *req, node_t *node);
extern int retrieve_custom_event_triggers_id(struct request *req, node_t *node);
extern int update_custom_event_triggers_id(struct request *req, node_t *node);
extern int delete_custom_event_triggers_id(struct request *req, node_t *node);
extern int retrieve_custom_event_triggers_id_notifications(struct request *req, node_t *node);
extern int update_custom_event_triggers_id_notifications(struct request *req, node_t *node);
extern int create_custom_event_triggers_id_notifications(struct request *req, node_t *node);
extern int delete_custom_event_triggers_id_notifications(struct request *req, node_t *node);
extern int retrieve_custom_event_triggers_id_notifications_id(struct request *req, node_t *node);
extern int update_custom_event_triggers_id_notifications_id(struct request *req, node_t *node);
extern int delete_custom_event_triggers_id_notifications_id(struct request *req, node_t *node);
extern int retrieve_custom_event_schedule(struct request *req, node_t *node);
extern int update_custom_event_schedule(struct request *req, node_t *node);
extern int retrieve_custom_event_notification(struct request *req, node_t *node);
extern int update_custom_event_notification(struct request *req, node_t *node);
extern int retrieve_custom_event_notification_mailing(struct request *req, node_t *node);
extern int update_custom_event_notification_mailing(struct request *req, node_t *node);
extern int create_custom_event_notification_mailing(struct request *req, node_t *node);
extern int delete_custom_event_notification_mailing(struct request *req, node_t *node);
extern int retrieve_custom_event_notification_mailing_id(struct request *req, node_t *node);
extern int update_custom_event_notification_mailing_id(struct request *req, node_t *node);
extern int delete_custom_event_notification_mailing_id(struct request *req, node_t *node);
extern int retrieve_custom_event_notification_ftp(struct request *req, node_t *node);
extern int update_custom_event_notification_ftp(struct request *req, node_t *node);
extern int create_custom_event_notification_ftp(struct request *req, node_t *node);
extern int delete_custom_event_notification_ftp(struct request *req, node_t *node);
extern int retrieve_custom_event_notification_ftp_id(struct request *req, node_t *node);
extern int update_custom_event_notification_ftp_id(struct request *req, node_t *node);
extern int delete_custom_event_notification_ftp_id(struct request *req, node_t *node);
extern int retrieve_custom_event_notification_httpHost(struct request *req, node_t *node);
extern int update_custom_event_notification_httpHost(struct request *req, node_t *node);
extern int create_custom_event_notification_httpHost(struct request *req, node_t *node);
extern int delete_custom_event_notification_httpHost(struct request *req, node_t *node);
extern int retrieve_custom_event_notification_httpHost_id(struct request *req, node_t *node);
extern int update_custom_event_notification_httpHost_id(struct request *req, node_t *node);
extern int delete_custom_event_notification_httpHost_id(struct request *req, node_t *node);
extern int retrieve_custom_event_triggers_id_capabilities(request *req, node_t *node);
extern int retrieve_custom_event_triggers_id_notifications_id_capabilities(request *req, node_t *node);
extern int retrieve_custom_event_notification_capabilities(request *req, node_t *node);

/* Custom/Analytics */
extern int get_region_count(void);
extern int retrieve_custom_analytics_motiondetection_index(struct request *req, node_t *node);
extern int retrieve_custom_analytics_motiondetection_description(struct request *req, node_t *node);
extern int retrieve_custom_analytics_motiondetection(struct request *req, node_t *node);
extern int retrieve_custom_analytics_motiondetection_id(struct request *req, node_t *node);
extern int update_custom_analytics_motiondetection_id(struct request *req, node_t *node);
extern int retrieve_custom_analytics_motiondetection_id_capabilities(request *req, node_t *node);
extern int retrieve_custom_analytics_facedetection(request *req, node_t *node);
extern int retrieve_custom_analytics_facedetection_id(request *req, node_t *node);
extern int update_custom_analytics_facedetection_id(request *req, node_t *node);
extern int retrieve_custom_analytics_facedetection_id_capabilities(request *req, node_t *node);
extern int retrieve_custom_analytics_facerecognition(request *req, node_t *node);
extern int retrieve_custom_analytics_facerecognition_id(request *req, node_t *node);
extern int update_custom_analytics_facerecognition_id(request *req, node_t *node);
extern int retrieve_custom_analytics_facerecognition_id_capabilities(request *req, node_t *node);
extern int retrieve_custom_analytics_motiondetection_id_regions(struct request *req, node_t *node);
extern int update_custom_analytics_motiondetection_id_regions(struct request *req, node_t *node);
extern int create_custom_analytics_motiondetection_id_regions(struct request *req, node_t *node);
extern int delete_custom_analytics_motiondetection_id_regions(struct request *req, node_t *node);
extern int retrieve_custom_analytics_motiondetection_id_regions_id(struct request *req, node_t *node);
extern int update_custom_analytics_motiondetection_id_regions_id(struct request *req, node_t *node);
extern int delete_custom_analytics_motiondetection_id_regions_id(struct request *req, node_t *node);
extern int retrieve_custom_analytics_motiondetection_id_regions_id_capabilities(request *req, node_t *node);

/* Custom */
extern int retrieve_custom_video_advanced_channels(struct request *req, node_t *node);
extern int retrieve_custom_video_advanced_channels_id(struct request *req, node_t *node);
extern int update_custom_video_advanced_channels(struct request *req, node_t *node);
extern int update_custom_video_advanced_channels_id(struct request *req, node_t *node);
//extern int delete_streaming_channels_id_advanced(struct request *req, node_t *node);
extern int retrieve_custom_video_advanced_channels_id_capabilities(struct request *req, node_t *node);

extern int retrieve_custom_live_channels(struct request *req, node_t *node);
extern int retrieve_custom_live_channels_id(struct request *req, node_t *node);
extern int update_custom_live_channels_id(struct request *req, node_t *node);
extern int retrieve_custom_live_channels_id_capabilities(struct request *req, node_t *node);

extern int retrieve_custom_users(struct request *req, node_t *node);
extern int retrieve_custom_users_id(struct request *req, node_t *node);
extern int update_custom_users_id(struct request *req, node_t *node);

extern int retrieve_custom_video_channels(struct request *req, node_t *node);
extern int retrieve_custom_video_channels_id(struct request *req, node_t *node);
extern int update_custom_video_channels(struct request *req, node_t *node);
extern int update_custom_video_channels_id(struct request *req, node_t *node);
extern int retrieve_custom_video_channels_id_capabilities(struct request *req, node_t *node);

extern int retrieve_custom_camera_channels(struct request *req, node_t *node);
extern int retrieve_custom_camera_channels_id(struct request *req, node_t *node);
extern int update_custom_camera_channels_id(struct request *req, node_t *node);
extern int retrieve_custom_camera_channels_id_capabilities(struct request *req, node_t *node);

extern int retrieve_custom_audio_channels(struct request *req, node_t *node);
extern int retrieve_custom_audio_channels_id(struct request *req, node_t *node);
extern int update_custom_audio_channels_id(struct request *req, node_t *node);

extern int retrieve_custom_datetime_channels(struct request *req, node_t *node);
extern int retrieve_custom_datetime_channels_id(struct request *req, node_t *node);
extern int update_custom_datetime_channels_id(struct request *req, node_t *node);
extern int retrieve_custom_datetime_channels_id_capabilities(struct request *req, node_t *node);

extern int retrieve_custom_network_channels(struct request *req, node_t *node);
extern int retrieve_custom_network_channels_id(struct request *req, node_t *node);
extern int update_custom_network_channels_id(struct request *req, node_t *node);
extern int retrieve_custom_network_channels_id_capabilities(struct request *req, node_t *node);

extern int retrieve_custom_alarm_triggers(struct request *req, node_t *node);
extern int retrieve_custom_alarm_triggers_id(struct request *req, node_t *node);
extern int update_custom_alarm_triggers_id(struct request *req, node_t *node);
extern int retrieve_custom_alarm_triggers_id_capabilities(struct request *req, node_t *node);

extern int retrieve_custom_storage(struct request *req, node_t *node);
extern int retrieve_custom_storage_id(struct request *req, node_t *node);
extern int update_custom_storage_id(struct request *req, node_t *node);
extern int retrieve_custom_storage_id_capabilities(struct request *req, node_t *node);

/* Initialization and cleanup */
extern int iface_init(struct request *req);
extern void iface_cleanup(struct request *req);

#endif /* __PSIA_IFACE_H__ */

