/*
 ## Cypress USB 3.0 Platform header file (cyfxuvcinmem.h)
 ## ===========================
 ##
 ##  Copyright Cypress Semiconductor Corporation, 2010-2023,
 ##  All Rights Reserved
 ##  UNPUBLISHED, LICENSED SOFTWARE.
 ##
 ##  CONFIDENTIAL AND PROPRIETARY INFORMATION
 ##  WHICH IS THE PROPERTY OF CYPRESS.
 ##
 ##  Use of this file is governed
 ##  by the license agreement included in the file
 ##
 ##     <install>/license/license.txt
 ##
 ##  where <install> is the Cypress software
 ##  installation root directory path.
 ##
 ## ===========================
*/

#ifndef _INCLUDED_CYFXUVCINMEM_H_
#define _INCLUDED_CYFXUVCINMEM_H_

#include <cyu3externcstart.h>
#include <cyu3types.h>
#include <cyu3usbconst.h>
#include <cyu3dma.h>

/* This header file comprises of the UVC application constants and
 * the video frame configurations */

// Thread stack size and priority
constexpr uint32_t UVC_APP_THREAD_STACK = 0x1000;
constexpr uint32_t UVC_APP_THREAD_PRIORITY = 8;

// Endpoint definition for UVC application
constexpr uint8_t CY_FX_EP_BULK_VIDEO = 0x81; // EP 1 IN configured as Bulk EP
constexpr auto CY_FX_EP_VIDEO_CONS_SOCKET = CY_U3P_UIB_SOCKET_CONS_1; // Consumer socket 1
constexpr uint8_t CY_FX_EP_CONTROL_STATUS = 0x82; // EP 2 IN

// UVC descriptor types
constexpr uint8_t CY_FX_INTF_ASSN_DSCR_TYPE = 11; // Interface association descriptor type

// UVC video streaming endpoint packet Size
constexpr uint16_t CY_FX_EP_BULK_VIDEO_PKT_SIZE = 0x400;

// UVC video streaming endpoint packet Count
constexpr uint8_t CY_FX_EP_BULK_VIDEO_PKTS_COUNT = 0x01;

constexpr uint8_t CY_FX_UVC_MAX_VID_FRAMES = 2; // Maximum number of video frames (4)

constexpr uint8_t CY_FX_BULK_BURST = 8; // Burst size for SS operation only.

// UVC Buffer size
constexpr uint32_t CY_FX_UVC_STREAM_BUF_SIZE = 4096;

// UVC Buffer count
constexpr uint8_t CY_FX_UVC_STREAM_BUF_COUNT = 10;

constexpr uint8_t CY_FX_UVC_MAX_HEADER = 12; // Maximum number of header bytes in UVC
constexpr uint8_t CY_FX_UVC_HEADER_DEFAULT_BFH = 0x8C; // Default BFH(Bit Field Header) for the UVC Header

constexpr uint8_t CY_FX_UVC_MAX_PROBE_SETTING = 34; // Maximum number of bytes in Probe Control
constexpr uint8_t CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED = 64; // Maximum number of bytes in Probe Control aligned to 32 byte

constexpr uint8_t CY_FX_UVC_HEADER_FRAME = 0; // Normal frame indication
constexpr uint8_t CY_FX_UVC_HEADER_EOF = 1 << 1; // End of frame indication
constexpr uint8_t CY_FX_UVC_HEADER_FRAME_ID = 1 << 0; // Frame ID toggle bit

constexpr uint8_t CY_FX_UVC_INTERFACE_VC = 0; // Video Control interface id.
constexpr uint8_t CY_FX_UVC_INTERFACE_VS = 1; // Video Streaming interface id.

constexpr uint8_t CY_FX_USB_UVC_SET_REQ_TYPE = 0x21; // UVC interface SET request type
constexpr uint8_t CY_FX_USB_UVC_GET_REQ_TYPE = 0xA1; // UVC Interface GET request type
constexpr uint8_t CY_FX_USB_UVC_GET_CUR_REQ = 0x81; // UVC GET_CUR request
constexpr uint8_t CY_FX_USB_UVC_SET_CUR_REQ = 0x01; // UVC SET_CUR request
constexpr uint8_t CY_FX_USB_UVC_GET_DEF_REQ = 0x87; // UVC GET_DEF request
constexpr uint8_t CY_FX_USB_UVC_GET_MIN_REQ = 0x82; // UVC GET_MIN request
constexpr uint8_t CY_FX_USB_UVC_GET_MAX_REQ = 0x83; // UVC GET_MAX request

constexpr uint16_t CY_FX_USB_UVC_VS_PROBE_CONTROL = 0x0100; // Control selector for VS_PROBE_CONTROL.
constexpr uint16_t CY_FX_USB_UVC_VS_COMMIT_CONTROL = 0x0200; // Control selector for VS_COMMIT_CONTROL.

constexpr uint16_t CY_FX_USB_UVC_VC_RQT_ERROR_CODE_CONTROL = 0x0200;
constexpr uint8_t CY_FX_USB_UVC_RQT_STAT_INVALID_CTRL = 0x06;

/* Extern definitions of the USB Enumeration constant arrays used for the Application */
extern const uint8_t CyFxUSB20DeviceDscr[];
extern const uint8_t CyFxUSB30DeviceDscr[];
extern const uint8_t CyFxUSBDeviceQualDscr[];
extern const uint8_t CyFxUSBFSConfigDscr[];
extern const uint8_t CyFxUSBHSConfigDscr[];
extern const uint8_t CyFxUSBBOSDscr[];
extern const uint8_t CyFxUSBSSConfigDscr[];
extern const uint8_t CyFxUSBStringLangIDDscr[];
extern const uint8_t CyFxUSBManufactureDscr[];
extern const uint8_t CyFxUSBProductDscr[];

/* Extern definitions of the Video frame data */

/* UVC Probe Control Setting */
extern const uint8_t glProbeCtrl[CY_FX_UVC_MAX_PROBE_SETTING];
 
/* Video frame lengths */
extern const uint32_t glVidFrameLen[CY_FX_UVC_MAX_VID_FRAMES];

/* MJPEG Video Frames */
extern const uint8_t glUVCVidFrames[];

#include <cyu3externcend.h>

#endif /* _INCLUDED_CYFXUVCINMEM_H_ */

/*[]*/

