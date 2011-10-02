/*!
 * \file 	sccp_protocol.c
 * \brief 	SCCP Protocol implementation.
 * This file does the protocol implementation only. It should not be used as a controller.
 * \author 	Marcello Ceschia <marcello.ceschia [at] users.sourceforge.net>
 * \note		This program is free software and may be modified and distributed under the terms of the GNU Public License.
 *		See the LICENSE file at the top of the source tree.
 *
 * $Date$
 * $Revision$
 */ 

#include "config.h"
#include "common.h"

/* CallInfo Message */
/*!
 * \brief Send CallInfoMessage (V3)
 */
static void sccp_device_sendCallinfoV3(const sccp_device_t * device, const sccp_channel_t * channel)
{
	sccp_moo_t *r;
	uint8_t instance = 0;


	instance = sccp_device_find_index_for_line(device, channel->line->name);

	REQ(r, CallInfoMessage);

	if (channel->callInfo.callingPartyName) {
		sccp_copy_string(r->msg.CallInfoMessage.callingPartyName, channel->callInfo.callingPartyName, sizeof(r->msg.CallInfoMessage.callingPartyName));
	}
	if (channel->callInfo.callingPartyNumber)
		sccp_copy_string(r->msg.CallInfoMessage.callingParty, channel->callInfo.callingPartyNumber, sizeof(r->msg.CallInfoMessage.callingParty));

	if (channel->callInfo.calledPartyName)
		sccp_copy_string(r->msg.CallInfoMessage.calledPartyName, channel->callInfo.calledPartyName, sizeof(r->msg.CallInfoMessage.calledPartyName));
	
	if (channel->callInfo.calledPartyNumber)
		sccp_copy_string(r->msg.CallInfoMessage.calledParty, channel->callInfo.calledPartyNumber, sizeof(r->msg.CallInfoMessage.calledParty));

	if (channel->callInfo.originalCalledPartyName)
		sccp_copy_string(r->msg.CallInfoMessage.originalCalledPartyName, channel->callInfo.originalCalledPartyName, sizeof(r->msg.CallInfoMessage.originalCalledPartyName));
	
	if (channel->callInfo.originalCalledPartyNumber)
		sccp_copy_string(r->msg.CallInfoMessage.originalCalledParty, channel->callInfo.originalCalledPartyNumber, sizeof(r->msg.CallInfoMessage.originalCalledParty));

	if (channel->callInfo.lastRedirectingPartyName)
		sccp_copy_string(r->msg.CallInfoMessage.lastRedirectingPartyName, channel->callInfo.lastRedirectingPartyName, sizeof(r->msg.CallInfoMessage.lastRedirectingPartyName));
	
	if (channel->callInfo.lastRedirectingPartyNumber)
		sccp_copy_string(r->msg.CallInfoMessage.lastRedirectingParty, channel->callInfo.lastRedirectingPartyNumber, sizeof(r->msg.CallInfoMessage.lastRedirectingParty));

	if (channel->callInfo.originalCdpnRedirectReason)
		r->msg.CallInfoMessage.originalCdpnRedirectReason = htolel(channel->callInfo.originalCdpnRedirectReason);
	
	if (channel->callInfo.lastRedirectingReason)
		r->msg.CallInfoMessage.lastRedirectingReason = htolel(channel->callInfo.lastRedirectingReason);

	if (channel->callInfo.cgpnVoiceMailbox)
		sccp_copy_string(r->msg.CallInfoMessage.cgpnVoiceMailbox, channel->callInfo.cgpnVoiceMailbox, sizeof(r->msg.CallInfoMessage.cgpnVoiceMailbox));
	
	if (channel->callInfo.cdpnVoiceMailbox)
		sccp_copy_string(r->msg.CallInfoMessage.cdpnVoiceMailbox, channel->callInfo.cdpnVoiceMailbox, sizeof(r->msg.CallInfoMessage.cdpnVoiceMailbox));

	if (channel->callInfo.originalCdpnVoiceMailbox)
		sccp_copy_string(r->msg.CallInfoMessage.originalCdpnVoiceMailbox, channel->callInfo.originalCdpnVoiceMailbox, sizeof(r->msg.CallInfoMessage.originalCdpnVoiceMailbox));
	
	if (channel->callInfo.lastRedirectingVoiceMailbox)
		sccp_copy_string(r->msg.CallInfoMessage.lastRedirectingVoiceMailbox, channel->callInfo.lastRedirectingVoiceMailbox, sizeof(r->msg.CallInfoMessage.lastRedirectingVoiceMailbox));

	r->msg.CallInfoMessage.lel_lineId = htolel(instance);
	r->msg.CallInfoMessage.lel_callRef = htolel(channel->callid);
	r->msg.CallInfoMessage.lel_callType = htolel(channel->calltype);
	r->msg.CallInfoMessage.lel_callSecurityStatus = htolel(SKINNY_CALLSECURITYSTATE_UNKNOWN);

	sccp_dev_send(device, r);
	sccp_log((DEBUGCAT_CHANNEL | DEBUGCAT_LINE)) (VERBOSE_PREFIX_3 "%s: Send callinfo for %s channel %d on line instance %d" "\n\tcallerid: %s" "\n\tcallerName: %s\n", (device) ? device->id : "(null)", calltype2str(channel->calltype), channel->callid, instance, channel->callInfo.callingPartyNumber, channel->callInfo.callingPartyName);
}

/*!
 * \brief Send CallInfoMessage (V7)
 */
static void sccp_device_sendCallinfoV7(const sccp_device_t *device, const sccp_channel_t * channel)
{
	sccp_moo_t *r;
	unsigned int dataSize = 12;
	const char *data[dataSize];
	int data_len[dataSize];
	unsigned int i = 0;
	int dummy_len = 0;
	uint8_t instance = 0;							/* line instance on device */

	instance = sccp_device_find_index_for_line(device, channel->line->name);
	memset(data, 0, sizeof(data));

	data[0] = (strlen(channel->callInfo.callingPartyNumber) > 0)				? channel->callInfo.callingPartyNumber			: NULL;
	data[1] = (strlen(channel->callInfo.calledPartyNumber) > 0)				? channel->callInfo.calledPartyNumber				: NULL;
	data[2] = (strlen(channel->callInfo.originalCalledPartyNumber) > 0)		? channel->callInfo.originalCalledPartyNumber	: NULL;
	data[3] = (strlen(channel->callInfo.lastRedirectingPartyNumber) > 0)		? channel->callInfo.lastRedirectingPartyNumber	: NULL;
	data[4] = (strlen(channel->callInfo.cgpnVoiceMailbox) > 0)					? channel->callInfo.cgpnVoiceMailbox				: NULL;
	data[5] = (strlen(channel->callInfo.cdpnVoiceMailbox) > 0)					? channel->callInfo.cdpnVoiceMailbox				: NULL;
	data[6] = (strlen(channel->callInfo.originalCdpnVoiceMailbox) > 0)		? channel->callInfo.originalCdpnVoiceMailbox		: NULL;
	data[7] = (strlen(channel->callInfo.lastRedirectingVoiceMailbox) > 0)	? channel->callInfo.lastRedirectingVoiceMailbox : NULL;
	data[8] = (strlen(channel->callInfo.callingPartyName) > 0)					? channel->callInfo.callingPartyName				: NULL;
	data[9] = (strlen(channel->callInfo.calledPartyName) > 0)					? channel->callInfo.calledPartyName				: NULL;
	data[10] = (strlen(channel->callInfo.originalCalledPartyName) > 0)		? channel->callInfo.originalCalledPartyName		: NULL;
	data[11] = (strlen(channel->callInfo.lastRedirectingPartyName) > 0)		? channel->callInfo.lastRedirectingPartyName		: NULL;

	for (i = 0; i < dataSize; i++) {
		if (data[i] != NULL) {
			data_len[i] = strlen(data[i]);
			dummy_len += data_len[i];
		} else {
			data_len[i] = 0;					//! \todo this should be 1?
		}
	}

	int hdr_len = sizeof(r->msg.CallInfoDynamicMessage) + (dataSize - 4);
	int padding = ((dummy_len + hdr_len) % 4);

	padding = (padding > 0) ? 4 - padding : 0;

	r = sccp_build_packet(CallInfoDynamicMessage, hdr_len + dummy_len + padding);

	r->msg.CallInfoDynamicMessage.lel_lineId = htolel(instance);
	r->msg.CallInfoDynamicMessage.lel_callRef = htolel(channel->callid);
	r->msg.CallInfoDynamicMessage.lel_callType = htolel(channel->calltype);
	r->msg.CallInfoDynamicMessage.partyPIRestrictionBits = 0;
	r->msg.CallInfoDynamicMessage.lel_callSecurityStatus = htolel(SKINNY_CALLSECURITYSTATE_NOTAUTHENTICATED);
	r->msg.CallInfoDynamicMessage.lel_callInstance = htolel(instance);
	r->msg.CallInfoDynamicMessage.lel_originalCdpnRedirectReason = htolel(channel->callInfo.originalCdpnRedirectReason);
	r->msg.CallInfoDynamicMessage.lel_lastRedirectingReason = htolel(channel->callInfo.lastRedirectingReason);

	if (dummy_len) {
		int bufferSize = dummy_len + ARRAY_LEN(data);
		char buffer[bufferSize];

		memset(&buffer[0], 0, bufferSize);
		int pos = 0;

		for (i = 0; i < ARRAY_LEN(data); i++) {
			sccp_log(DEBUGCAT_PBX) (VERBOSE_PREFIX_3 "SCCP: cid field %d, value: '%s'\n", i, (data[i]) ? data[i] : "");
			if (data[i]) {
				memcpy(&buffer[pos], data[i], data_len[i]);
				pos += data_len[i] + 1;
			} else {
				pos += 1;
			}
		}
		memcpy(&r->msg.CallInfoDynamicMessage.dummy, &buffer[0], bufferSize);
	}

	sccp_dev_send(device, r);
}

/*!
 * \brief Send CallInfoMessage (V16)
 */
static void sccp_protocol_sendCallinfoV16(const sccp_device_t *device, const sccp_channel_t * channel)
{
	sccp_moo_t *r;
	unsigned int dataSize = 16;
	const char *data[dataSize];
	int data_len[dataSize];
	unsigned int i = 0;
	int dummy_len = 0;
	uint8_t instance = 0;							/* line instance on device */

	instance = sccp_device_find_index_for_line(device, channel->line->name);
	memset(data, 0, sizeof(data));

	data[0] = (strlen(channel->callInfo.callingPartyNumber) > 0)				? channel->callInfo.callingPartyNumber			: NULL;
	data[1] = (strlen(channel->callInfo.originalCallingPartyNumber) > 0) 	? channel->callInfo.originalCallingPartyNumber 	: NULL;
	data[2] = (strlen(channel->callInfo.calledPartyNumber) > 0)				? channel->callInfo.calledPartyNumber				: NULL;
	data[3] = (strlen(channel->callInfo.originalCalledPartyNumber) > 0)		? channel->callInfo.originalCalledPartyNumber 	: NULL;
	data[4] = (strlen(channel->callInfo.lastRedirectingPartyNumber) > 0) 	? channel->callInfo.lastRedirectingPartyNumber 	: NULL;
	data[5] = (strlen(channel->callInfo.cgpnVoiceMailbox) > 0)					? channel->callInfo.cgpnVoiceMailbox				: NULL;
	data[6] = (strlen(channel->callInfo.cdpnVoiceMailbox) > 0)					? channel->callInfo.cdpnVoiceMailbox				: NULL;
	data[7] = (strlen(channel->callInfo.originalCdpnVoiceMailbox) > 0)		? channel->callInfo.originalCdpnVoiceMailbox 	: NULL;
	data[8] = (strlen(channel->callInfo.lastRedirectingVoiceMailbox) > 0)	? channel->callInfo.lastRedirectingVoiceMailbox : NULL;
	data[9] = (strlen(channel->callInfo.callingPartyName) > 0)					? channel->callInfo.callingPartyName				: NULL;
	data[10] = (strlen(channel->callInfo.calledPartyName) > 0)					? channel->callInfo.calledPartyName				: NULL;
	data[11] = (strlen(channel->callInfo.originalCalledPartyName) > 0)		? channel->callInfo.originalCalledPartyName		: NULL;
	data[12] = (strlen(channel->callInfo.lastRedirectingPartyName) > 0)		? channel->callInfo.lastRedirectingPartyName 	: NULL;
	data[13] = (strlen(channel->callInfo.originalCalledPartyName) > 0)		? channel->callInfo.originalCalledPartyName		: NULL;
	data[14] = (strlen(channel->callInfo.cgpnVoiceMailbox) > 0)				? channel->callInfo.cgpnVoiceMailbox				: NULL;
	data[15] = (strlen(channel->callInfo.cdpnVoiceMailbox) > 0)				? channel->callInfo.cdpnVoiceMailbox				: NULL;

	for (i = 0; i < dataSize; i++) {
		if (data[i] != NULL) {
			data_len[i] = strlen(data[i]);
			dummy_len += data_len[i];
		} else {
			data_len[i] = 0;					//! \todo this should be 1?
		}
	}

	int hdr_len = sizeof(r->msg.CallInfoDynamicMessage) + (dataSize - 4);
	int padding = ((dummy_len + hdr_len) % 4);

	padding = (padding > 0) ? 4 - padding : 0;

	r = sccp_build_packet(CallInfoDynamicMessage, hdr_len + dummy_len + padding);

	r->msg.CallInfoDynamicMessage.lel_lineId = htolel(instance);
	r->msg.CallInfoDynamicMessage.lel_callRef = htolel(channel->callid);
	r->msg.CallInfoDynamicMessage.lel_callType = htolel(channel->calltype);
	r->msg.CallInfoDynamicMessage.partyPIRestrictionBits = 0;
	r->msg.CallInfoDynamicMessage.lel_callSecurityStatus = htolel(SKINNY_CALLSECURITYSTATE_NOTAUTHENTICATED);
	r->msg.CallInfoDynamicMessage.lel_callInstance = htolel(instance);
	r->msg.CallInfoDynamicMessage.lel_originalCdpnRedirectReason = htolel(channel->callInfo.originalCdpnRedirectReason);
	r->msg.CallInfoDynamicMessage.lel_lastRedirectingReason = htolel(channel->callInfo.lastRedirectingReason);

	if (dummy_len) {
		int bufferSize = dummy_len + ARRAY_LEN(data);
		char buffer[bufferSize];

		memset(&buffer[0], 0, bufferSize);
		int pos = 0;

		for (i = 0; i < ARRAY_LEN(data); i++) {
			sccp_log(DEBUGCAT_PBX) (VERBOSE_PREFIX_3 "SCCP: cid field %d, value: '%s'\n", i, (data[i]) ? data[i] : "");
			if (data[i]) {
				memcpy(&buffer[pos], data[i], data_len[i]);
				pos += data_len[i] + 1;
			} else {
				pos += 1;
			}
		}
		memcpy(&r->msg.CallInfoDynamicMessage.dummy, &buffer[0], bufferSize);
	}
	sccp_dev_send(device, r);
}
/* done - callInfoMessage*/


/* DialedNumber Message */
/*!
 * \brief Send DialedNumber Message (V3)
 */
static void sccp_protocol_sendDialedNumberV3(const sccp_device_t *device, const sccp_channel_t *channel){
	sccp_moo_t *r;
	uint8_t instance;


	REQ(r, DialedNumberMessage);

	instance = sccp_device_find_index_for_line(device, channel->line->name);
	sccp_copy_string(r->msg.DialedNumberMessage.calledParty, channel->callInfo.calledPartyNumber, sizeof(r->msg.DialedNumberMessage.calledParty));

	r->msg.DialedNumberMessage.lel_lineId = htolel(instance);
	r->msg.DialedNumberMessage.lel_callRef = htolel(channel->callid);
	
	sccp_dev_send(device, r);
	sccp_log(DEBUGCAT_CHANNEL) (VERBOSE_PREFIX_3 "%s: Send the dialed number %s for %s channel %d\n", device->id, channel->callInfo.calledPartyNumber, calltype2str(channel->calltype), channel->callid);
}

/*!
 * \brief Send DialedNumber Message (V19)
 *
 * \todo this message is wrong, but we get 'Placed Calls' working on V >= 19
 */
static void sccp_protocol_sendDialedNumberV19(const sccp_device_t *device, const sccp_channel_t *channel){
	sccp_moo_t *r;
	uint8_t instance;

	REQ(r, DialedNumberMessageV19);

	instance = sccp_device_find_index_for_line(device, channel->line->name);
	
	sccp_copy_string(r->msg.DialedNumberMessageV19.calledParty, channel->callInfo.calledPartyNumber, sizeof(r->msg.DialedNumberMessage.calledParty));

	r->msg.DialedNumberMessageV19.lel_lineId = htolel(instance);
	r->msg.DialedNumberMessageV19.lel_callRef = htolel(channel->callid);
	
	sccp_dev_send(device, r);
	sccp_log(DEBUGCAT_CHANNEL) (VERBOSE_PREFIX_3 "%s: Send the dialed number %s for %s channel %d\n", device->id, channel->callInfo.calledPartyNumber, calltype2str(channel->calltype), channel->callid);
}
/* done - DialedNumber Message */


/* Display Prompt Message */
/*!
 * \brief Send Display Prompt Message (Static)
 */
static void sccp_protocol_sendStaticDisplayprompt(const sccp_device_t *device, uint8_t lineInstance, uint8_t callid, uint8_t timeout, const char *message){
	sccp_moo_t *r;
	
	REQ(r, DisplayPromptStatusMessage);
	r->msg.DisplayPromptStatusMessage.lel_messageTimeout = htolel(timeout);
	r->msg.DisplayPromptStatusMessage.lel_callReference = htolel(callid);
	r->msg.DisplayPromptStatusMessage.lel_lineInstance = htolel(lineInstance);
	sccp_copy_string(r->msg.DisplayPromptStatusMessage.promptMessage, message, sizeof(r->msg.DisplayPromptStatusMessage.promptMessage));
	
	sccp_dev_send(device, r);
	sccp_log((DEBUGCAT_DEVICE | DEBUGCAT_LINE)) (VERBOSE_PREFIX_3 "%s: Display prompt on line %d, callid %d, timeout %d\n", device->id, lineInstance, callid, timeout);
}

/*!
 * \brief Send Display Prompt Message (Dynamic)
 */
static void sccp_protocol_sendDynamicDisplayprompt(const sccp_device_t *device, uint8_t lineInstance, uint8_t callid, uint8_t timeout, const char *message){
	sccp_moo_t *r;
	
	int msg_len = strlen(message);
	int hdr_len = sizeof(r->msg.DisplayDynamicPromptStatusMessage) - 3;
	int padding = ((msg_len + hdr_len) % 4);

	padding = (padding > 0) ? 4 - padding : 0;
	r = sccp_build_packet(DisplayDynamicPromptStatusMessage, hdr_len + msg_len + padding);
	r->msg.DisplayDynamicPromptStatusMessage.lel_messageTimeout = htolel(timeout);
	r->msg.DisplayDynamicPromptStatusMessage.lel_callReference = htolel(callid);
	r->msg.DisplayDynamicPromptStatusMessage.lel_lineInstance = htolel(lineInstance);
	memcpy(&r->msg.DisplayDynamicPromptStatusMessage.dummy, message, msg_len);
	
	sccp_dev_send(device, r);
	sccp_log((DEBUGCAT_DEVICE | DEBUGCAT_LINE)) (VERBOSE_PREFIX_3 "%s: Display prompt on line %d, callid %d, timeout %d\n", device->id, lineInstance, callid, timeout);
}
/* done - display prompt */

/* Display Notify  Message */
/*!
 * \brief Send Display Notify Message (Static)
 */
static void sccp_protocol_sendStaticDisplayNotify(const sccp_device_t *device, uint8_t timeout, const char *message){
	sccp_moo_t *r;
	
	REQ(r, DisplayNotifyMessage);
	r->msg.DisplayNotifyMessage.lel_displayTimeout = htolel(timeout);
	sccp_copy_string(r->msg.DisplayNotifyMessage.displayMessage, message, sizeof(r->msg.DisplayNotifyMessage.displayMessage));
	
	sccp_dev_send(device, r);
	sccp_log((DEBUGCAT_DEVICE | DEBUGCAT_LINE)) (VERBOSE_PREFIX_3 "%s: Display notify timeout %d\n", device->id, timeout);
}

/*!
 * \brief Send Display Notify Message (Dynamic)
 */
static void sccp_protocol_sendDynamicDisplayNotify(const sccp_device_t *device, uint8_t timeout, const char *message){
	sccp_moo_t *r;
	
	int msg_len = strlen(message);
	int hdr_len = sizeof(r->msg.DisplayDynamicNotifyMessage) - 3;
	int padding = ((msg_len + hdr_len) % 4);

	padding = (padding > 0) ? 4 - padding : 0;
	r = sccp_build_packet(DisplayDynamicNotifyMessage, hdr_len + msg_len + padding);
	r->msg.DisplayDynamicNotifyMessage.lel_displayTimeout = htolel(timeout);
	memcpy(&r->msg.DisplayDynamicNotifyMessage.dummy, message, msg_len);
	
	sccp_dev_send(device, r);
	sccp_log((DEBUGCAT_DEVICE | DEBUGCAT_LINE)) (VERBOSE_PREFIX_3 "%s: Display notify timeout %d\n", device->id, timeout);
}
/* done - display notify */

/* Display Priority Notify Message */
/*!
 * \brief Send Priority Display Notify Message (Static)
 */
static void sccp_protocol_sendStaticDisplayPriNotify(const sccp_device_t *device, uint8_t priority, uint8_t timeout, const char *message){
	sccp_moo_t *r;
	
	REQ(r, DisplayPriNotifyMessage);
	r->msg.DisplayPriNotifyMessage.lel_displayTimeout = htolel(timeout);
	r->msg.DisplayPriNotifyMessage.lel_priority = htolel(priority);
	sccp_copy_string(r->msg.DisplayPriNotifyMessage.displayMessage, message, sizeof(r->msg.DisplayPriNotifyMessage.displayMessage));
	
	sccp_dev_send(device, r);
	sccp_log((DEBUGCAT_DEVICE | DEBUGCAT_LINE)) (VERBOSE_PREFIX_3 "%s: Display notify timeout %d\n", device->id, timeout);
}

/*!
 * \brief Send Priority Display Notify Message (Dynamic)
 */
static void sccp_protocol_sendDynamicDisplayPriNotify(const sccp_device_t *device, uint8_t priority, uint8_t timeout, const char *message){
	sccp_moo_t *r;
	
	int msg_len = strlen(message);
	int hdr_len = sizeof(r->msg.DisplayDynamicPriNotifyMessage) - 3;
	int padding = ((msg_len + hdr_len) % 4);

	padding = (padding > 0) ? 4 - padding : 0;
	r = sccp_build_packet(DisplayDynamicPriNotifyMessage, hdr_len + msg_len + padding);
	r->msg.DisplayDynamicPriNotifyMessage.lel_displayTimeout = htolel(timeout);
	r->msg.DisplayDynamicPriNotifyMessage.lel_priority = htolel(priority);
	memcpy(&r->msg.DisplayDynamicPriNotifyMessage.dummy, message, msg_len);
	
	sccp_dev_send(device, r);
	sccp_log((DEBUGCAT_DEVICE | DEBUGCAT_LINE)) (VERBOSE_PREFIX_3 "%s: Display notify timeout %d\n", device->id, timeout);
}
/* done - display notify */

/* callForwardStatus Message */
/*!
 * \brief Send Call Forward Status Message
 */
static void sccp_protocol_sendCallForwardStatus(const sccp_device_t *device, const void *data){
	sccp_moo_t *r;
	const sccp_linedevices_t *linedevice = (sccp_linedevices_t *)data;
	
	REQ(r, ForwardStatMessage);
	r->msg.ForwardStatMessage.lel_lineNumber = htolel(linedevice->lineInstance);
	r->msg.ForwardStatMessage.lel_status = (linedevice->cfwdAll.enabled || linedevice->cfwdBusy.enabled) ? htolel(1) : 0;
	
	if (linedevice->cfwdAll.enabled) {
		r->msg.ForwardStatMessage.lel_cfwdallstatus = htolel(1);
		sccp_copy_string(r->msg.ForwardStatMessage.cfwdallnumber, linedevice->cfwdAll.number, sizeof(r->msg.ForwardStatMessage.cfwdallnumber));
	} if (linedevice->cfwdBusy.enabled) {
		r->msg.ForwardStatMessage.lel_cfwdbusystatus = htolel(1);
		sccp_copy_string(r->msg.ForwardStatMessage.cfwdbusynumber, linedevice->cfwdBusy.number, sizeof(r->msg.ForwardStatMessage.cfwdbusynumber));
	}
	
	sccp_dev_send(device, r);
}

/*!
 * \brief Send Call Forward Status Message (V19)
 */
static void sccp_protocol_sendCallForwardStatusV19(const sccp_device_t *device, const void *data){
	sccp_moo_t *r;
	const sccp_linedevices_t *linedevice = (sccp_linedevices_t *)data;
	
	REQ(r, ForwardStatMessageV19);
	r->msg.ForwardStatMessageV19.lel_lineNumber = htolel(linedevice->lineInstance);
	r->msg.ForwardStatMessageV19.lel_status = (linedevice->cfwdAll.enabled || linedevice->cfwdBusy.enabled) ? htolel(1) : 0;
	
	if (linedevice->cfwdAll.enabled) {
	  
		r->msg.ForwardStatMessageV19.lel_cfwdallstatus = htolel(1);
		sccp_copy_string(r->msg.ForwardStatMessageV19.cfwdallnumber, linedevice->cfwdAll.number, sizeof(r->msg.ForwardStatMessageV19.cfwdallnumber));
		
	} if (linedevice->cfwdBusy.enabled) {
	  
		r->msg.ForwardStatMessageV19.lel_cfwdbusystatus = htolel(1);
		sccp_copy_string(r->msg.ForwardStatMessageV19.cfwdbusynumber, linedevice->cfwdBusy.number, sizeof(r->msg.ForwardStatMessageV19.cfwdbusynumber));
	}
	
	r->msg.ForwardStatMessageV19.lel_unknown = 0x000000FF; 
	sccp_dev_send(device, r);
}
/* done - send callForwardStatus */


/* registerAck Message */
/*!
 * \brief Send Register Acknowledgement Message (V3)
 */
static void sccp_protocol_sendRegisterAckV3(const sccp_device_t *device, uint8_t keepAliveInterval, uint8_t secondaryKeepAlive, char *dateformat){
	sccp_moo_t *r;
	
	REQ(r, RegisterAckMessage);
	
	/* just for documentation */
#if 0
	r->msg.RegisterAckMessage.unknown1 = 0x00;
	r->msg.RegisterAckMessage.unknown2 = 0x00;
	r->msg.RegisterAckMessage.unknown3 = 0x00;
#endif
	r->msg.RegisterAckMessage.protocolVer = device->protocol->version;
	r->msg.RegisterAckMessage.lel_keepAliveInterval = htolel(keepAliveInterval);
	r->msg.RegisterAckMessage.lel_secondaryKeepAliveInterval = htolel(secondaryKeepAlive);

	memcpy(r->msg.RegisterAckMessage.dateTemplate, dateformat, sizeof(r->msg.RegisterAckMessage.dateTemplate));
	sccp_dev_send(device, r);  
}

/*!
 * \brief Send Register Acknowledgement Message (V4)
 */
static void sccp_protocol_sendRegisterAckV4(const sccp_device_t *device, uint8_t keepAliveInterval, uint8_t secondaryKeepAlive, char *dateformat){
	sccp_moo_t *r;
	
	REQ(r, RegisterAckMessage);

	r->msg.RegisterAckMessage.unknown1 = 0x20;			// 0x00;
	r->msg.RegisterAckMessage.unknown2 = 0x00;			// 0x00;
	r->msg.RegisterAckMessage.unknown3 = 0xFE;			// 0xFE;
	
	r->msg.RegisterAckMessage.protocolVer = device->protocol->version;
	
	r->msg.RegisterAckMessage.lel_keepAliveInterval = htolel(keepAliveInterval);
	r->msg.RegisterAckMessage.lel_secondaryKeepAliveInterval = htolel(secondaryKeepAlive);

	memcpy(r->msg.RegisterAckMessage.dateTemplate, dateformat, sizeof(r->msg.RegisterAckMessage.dateTemplate));
	sccp_dev_send(device, r);  
}

/*!
 * \brief Send Register Acknowledgement Message (V11)
 */
static void sccp_protocol_sendRegisterAckV11(const sccp_device_t *device, uint8_t keepAliveInterval, uint8_t secondaryKeepAlive, char *dateformat){
	sccp_moo_t *r;
	
	REQ(r, RegisterAckMessage);
	
	r->msg.RegisterAckMessage.unknown1 = 0x20;			// 0x00;
	r->msg.RegisterAckMessage.unknown2 = 0xF1;			// 0xF1;
	r->msg.RegisterAckMessage.unknown3 = 0xFF;			// 0xFF;
	
	r->msg.RegisterAckMessage.protocolVer = device->protocol->version;
	r->msg.RegisterAckMessage.lel_keepAliveInterval = htolel(keepAliveInterval);
	r->msg.RegisterAckMessage.lel_secondaryKeepAliveInterval = htolel(secondaryKeepAlive);

	memcpy(r->msg.RegisterAckMessage.dateTemplate, dateformat, sizeof(r->msg.RegisterAckMessage.dateTemplate));
	sccp_dev_send(device, r);  
}
/* done - registerACK */

/* Token Messages */
static void sccp_protocol_sendTokenAckSCCP(const sccp_device_t *device, uint32_t features) {
        sccp_moo_t *r;
	REQ(r, RegisterTokenAck);
	sccp_session_send(device, r);
}
static void sccp_protocol_sendTokenRejectSCCP(const sccp_device_t *device, uint32_t backoff_time, uint32_t features) {
        sccp_moo_t *r;
	REQ(r, RegisterTokenReject);
	r->msg.RegisterTokenReject.lel_tokenRejWaitTime=backoff_time;
	sccp_session_send(device, r);
}
static void sccp_protocol_sendTokenAckSPCP(const sccp_device_t *device, uint32_t features) {
        sccp_moo_t *r;
	REQ(r, SPCPRegisterTokenAck);
	r->msg.SPCPRegisterTokenAck.lel_features = htolel(features);
	sccp_session_send(device, r);
}
static void sccp_protocol_sendTokenRejectSPCP(const sccp_device_t *device, uint32_t backoff_time, uint32_t features) {
        sccp_moo_t *r;
	REQ(r, SPCPRegisterTokenReject);
        r->msg.SPCPRegisterTokenReject.lel_features=features;
	sccp_session_send(device, r);
}
/* done - Token Messagss */

/* sendUserToDeviceData Message */
/*!
 * \brief Send User To Device Message (V1)
 */
static void sccp_protocol_sendUserToDeviceDataVersion1Message(const sccp_device_t *device, const void *xmlData, uint8_t priority){
	sccp_moo_t *r = NULL;
	
	int dummy_len, msgSize, hdr_len, padding;

	dummy_len = strlen(xmlData);
	hdr_len = 40 - 1;
	padding = ((dummy_len + hdr_len) % 4);
	padding = (padding > 0) ? 4 - padding : 0;
	msgSize = hdr_len + dummy_len + padding;

	r = sccp_build_packet(UserToDeviceDataVersion1Message, msgSize);
	r->msg.UserToDeviceDataVersion1Message.lel_callReference = htolel(1);
	r->msg.UserToDeviceDataVersion1Message.lel_transactionID = htolel(1);
	r->msg.UserToDeviceDataVersion1Message.lel_sequenceFlag = 0x0002;
	r->msg.UserToDeviceDataVersion1Message.lel_displayPriority = htolel(priority);
	r->msg.UserToDeviceDataVersion1Message.lel_dataLength = htolel(dummy_len);

	if (dummy_len) {
		char buffer[dummy_len + 2];

		memset(&buffer[0], 0, sizeof(buffer));
		memcpy(&buffer[0], xmlData, dummy_len);

		memcpy(&r->msg.UserToDeviceDataVersion1Message.data, &buffer[0], sizeof(buffer));
		sccp_dev_send(device, r); 
	}
}
/* done - sendUserToDeviceData */


/*! \todo need a protocol implementation for ConnectionStatisticsReq using Version 19 and higher */
/*! \todo need a protocol implementation for ForwardStatMessage using Version 19 and higher */

/*! 
 * \brief SCCP Protocol Version to Message Mapping
 */
static const sccp_deviceProtocol_t *sccpProtocolDefinition[] = {
	NULL, 
	NULL,
	NULL,
	&(sccp_deviceProtocol_t){"SCCP", 3, sccp_device_sendCallinfoV3, sccp_protocol_sendDialedNumberV3, sccp_protocol_sendRegisterAckV3, sccp_protocol_sendStaticDisplayprompt, sccp_protocol_sendStaticDisplayNotify, sccp_protocol_sendStaticDisplayPriNotify, sccp_protocol_sendCallForwardStatus,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSCCP, sccp_protocol_sendTokenRejectSCCP},/* default impl*/
	NULL,
	&(sccp_deviceProtocol_t){"SCCP", 5, sccp_device_sendCallinfoV3, sccp_protocol_sendDialedNumberV3, sccp_protocol_sendRegisterAckV4, sccp_protocol_sendStaticDisplayprompt, sccp_protocol_sendStaticDisplayNotify, sccp_protocol_sendStaticDisplayPriNotify, sccp_protocol_sendCallForwardStatus,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSCCP, sccp_protocol_sendTokenRejectSCCP},
	NULL,
	NULL,
	NULL,
	&(sccp_deviceProtocol_t){"SCCP", 9, sccp_device_sendCallinfoV7, sccp_protocol_sendDialedNumberV3, sccp_protocol_sendRegisterAckV4, sccp_protocol_sendDynamicDisplayprompt, sccp_protocol_sendDynamicDisplayNotify, sccp_protocol_sendDynamicDisplayPriNotify, sccp_protocol_sendCallForwardStatus,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSCCP, sccp_protocol_sendTokenRejectSCCP},
	&(sccp_deviceProtocol_t){"SCCP", 10, sccp_device_sendCallinfoV7, sccp_protocol_sendDialedNumberV3, sccp_protocol_sendRegisterAckV4, sccp_protocol_sendDynamicDisplayprompt, sccp_protocol_sendDynamicDisplayNotify, sccp_protocol_sendDynamicDisplayPriNotify, sccp_protocol_sendCallForwardStatus,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSCCP, sccp_protocol_sendTokenRejectSCCP},
	&(sccp_deviceProtocol_t){"SCCP", 11, sccp_device_sendCallinfoV7, sccp_protocol_sendDialedNumberV3, sccp_protocol_sendRegisterAckV11, sccp_protocol_sendDynamicDisplayprompt, sccp_protocol_sendDynamicDisplayNotify, sccp_protocol_sendDynamicDisplayPriNotify, sccp_protocol_sendCallForwardStatus,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSCCP, sccp_protocol_sendTokenRejectSCCP},
	NULL,
	NULL,
	NULL,
	&(sccp_deviceProtocol_t){"SCCP", 15, sccp_device_sendCallinfoV7, sccp_protocol_sendDialedNumberV3, sccp_protocol_sendRegisterAckV11, sccp_protocol_sendDynamicDisplayprompt, sccp_protocol_sendDynamicDisplayNotify, sccp_protocol_sendDynamicDisplayPriNotify, sccp_protocol_sendCallForwardStatus,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSCCP, sccp_protocol_sendTokenRejectSCCP},
	&(sccp_deviceProtocol_t){"SCCP", 16, sccp_protocol_sendCallinfoV16, sccp_protocol_sendDialedNumberV3, sccp_protocol_sendRegisterAckV11, sccp_protocol_sendDynamicDisplayprompt, sccp_protocol_sendDynamicDisplayNotify, sccp_protocol_sendDynamicDisplayPriNotify, sccp_protocol_sendCallForwardStatus,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSCCP, sccp_protocol_sendTokenRejectSCCP},
	&(sccp_deviceProtocol_t){"SCCP", 17, sccp_protocol_sendCallinfoV16, sccp_protocol_sendDialedNumberV3, sccp_protocol_sendRegisterAckV11, sccp_protocol_sendDynamicDisplayprompt, sccp_protocol_sendDynamicDisplayNotify, sccp_protocol_sendDynamicDisplayPriNotify, sccp_protocol_sendCallForwardStatus,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSCCP, sccp_protocol_sendTokenRejectSCCP},
	NULL,
	&(sccp_deviceProtocol_t){"SCCP", 19, sccp_protocol_sendCallinfoV16, sccp_protocol_sendDialedNumberV19, sccp_protocol_sendRegisterAckV11, sccp_protocol_sendDynamicDisplayprompt, sccp_protocol_sendDynamicDisplayNotify, sccp_protocol_sendDynamicDisplayPriNotify, sccp_protocol_sendCallForwardStatusV19,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSCCP, sccp_protocol_sendTokenRejectSCCP},
	&(sccp_deviceProtocol_t){"SCCP", 20, sccp_protocol_sendCallinfoV16, sccp_protocol_sendDialedNumberV19, sccp_protocol_sendRegisterAckV11, sccp_protocol_sendDynamicDisplayprompt, sccp_protocol_sendDynamicDisplayNotify, sccp_protocol_sendDynamicDisplayPriNotify, sccp_protocol_sendCallForwardStatusV19,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSCCP, sccp_protocol_sendTokenRejectSCCP},
};


/*! 
 * \brief SPCP Protocol Version to Message Mapping
 */

static const sccp_deviceProtocol_t *spcpProtocolDefinition[] = {
	&(sccp_deviceProtocol_t){"SPCP", 0, sccp_device_sendCallinfoV3, sccp_protocol_sendDialedNumberV3, sccp_protocol_sendRegisterAckV4, sccp_protocol_sendDynamicDisplayprompt, sccp_protocol_sendDynamicDisplayNotify, sccp_protocol_sendDynamicDisplayPriNotify, sccp_protocol_sendCallForwardStatus,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSPCP, sccp_protocol_sendTokenRejectSPCP},
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&(sccp_deviceProtocol_t){"SPCP", 8, sccp_device_sendCallinfoV3, sccp_protocol_sendDialedNumberV3, sccp_protocol_sendRegisterAckV4, sccp_protocol_sendDynamicDisplayprompt, sccp_protocol_sendDynamicDisplayNotify, sccp_protocol_sendDynamicDisplayPriNotify, sccp_protocol_sendCallForwardStatus,sccp_protocol_sendUserToDeviceDataVersion1Message, sccp_protocol_sendTokenAckSPCP, sccp_protocol_sendTokenRejectSPCP},
};

/*! 
 * \brief Get Maximum Supported Version Number by Protocol Type
 */
uint8_t sccp_protocol_getMaxSupportedVersionNumber(int type){
	switch(type){
	
	  case SCCP_PROTOCOL:
		return ARRAY_LEN(sccpProtocolDefinition) - 1;
	  case SPCP_PROTOCOL:
		return ARRAY_LEN(spcpProtocolDefinition) - 1;
	  default:
		return 0;
	}
}

/*!
 * \brief Get Maximum Possible Protocol Supported by Device
 */
const sccp_deviceProtocol_t *sccp_protocol_getDeviceProtocol(const sccp_device_t *device, int type){
	
	uint8_t i;
	uint8_t version = device->protocolversion;
	const sccp_deviceProtocol_t **protocolDef;
	size_t protocolArraySize;
	uint8_t returnProtocol;
	
	sccp_log(DEBUGCAT_DEVICE) (VERBOSE_PREFIX_3 "SCCP: searching for our capability for device protocol version %d\n", version);
	
	if(type == SCCP_PROTOCOL) {
		protocolArraySize = ARRAY_LEN(sccpProtocolDefinition);
		protocolDef = sccpProtocolDefinition;
		returnProtocol=3;				// setting minimally returned protocol
		sccp_log(DEBUGCAT_DEVICE) (VERBOSE_PREFIX_3 "SCCP: searching for our capability for device protocol SCCP\n");
	} else {
		protocolArraySize = ARRAY_LEN(spcpProtocolDefinition);
		protocolDef = spcpProtocolDefinition;
		returnProtocol=0;
		sccp_log(DEBUGCAT_DEVICE) (VERBOSE_PREFIX_3 "SCCP: searching for our capability for device protocol SPCP\n");
	}
	
	for(i = (protocolArraySize - 1); i>0; i--){
		
		if(protocolDef[i] != NULL && version >= protocolDef[i]->version){
			sccp_log(DEBUGCAT_DEVICE) (VERBOSE_PREFIX_3 "%s: found protocol version '%d' at %d\n", protocolDef[i]->name, protocolDef[i]->version, i);
			returnProtocol=i;
			break;
		}
	}

	return protocolDef[returnProtocol];
}
