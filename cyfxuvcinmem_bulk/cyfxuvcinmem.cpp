/*
 ## Cypress USB 3.0 Platform source file (cyfxuvcinmem.c)
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

/* This file illustrates USB video class application example over BULK Endpoint (streaming from internal
   memory) 
 */

/*
   This example implements a USB video class Driver with the help of the appropriate USB enumeration
   descriptors over BULK Endpoint. With these descriptors, the FX3 device enumerates as a USB Video 
   Class device on the USB host.

   On successful enumeration the device shows up in the Windows Explorer. When the device is opened
   the host initiates a set of UVC specific class requests. The main class requests that need to be
   handled by the device are the GET/SET probe control request and SET commit control request.
   A predefined probe setting is returned as part of the Get Probe request. The Set probe / commit
   request is not interpreted and is only acknowledged.

   A successful set configuration starts the video streaming.

   The video streaming is accomplished with the help of a DMA MANUAL_OUT channel. Video frames are
   stored in contiguous memory location as a constant array. These frames are then loaded onto the
   DMA buffer one by one with appropriate UVC headers. With completion of each video frame the next
   indexed video frame is chosen for transfer. When all the frames are transferred, the index is reset
   to start transfer from the first video frame.

   CY_FX_UVC_STREAM_BUF_SIZE and CY_FX_UVC_STREAM_BUF_COUNT in the header file define the DMA buffer
   size and the number of DMA buffers respectively.

   This example is not supported on full speed interface.
 */

#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyfxuvcinmem.h"
#include "cyu3usb.h"
#include "cyu3uart.h"
#include "cyu3utils.h"

CyU3PThread uvcAppThread;           /* Thread structure */

/* Callback to handle the USB Setup Requests and UVC Class events */
union UsbSetup
{
    struct
    {
        uint8_t bmRequestType; // Request type
        uint8_t bRequest;      // Request
        uint16_t wValue;       // Value
        uint16_t wIndex;       // Index
        uint16_t wLength;      // Length
    } fields;
    struct
    {
        uint32_t setupdat0;
        uint32_t setupdat1;
    } raw;
    uint32_t words[2];
};

/* UVC Header */
uint8_t glUVCHeader[CY_FX_UVC_MAX_HEADER] =
{
    0x0C,                           /* Header Length */
    0x8C,                           /* Bit field header field */
    0x00,0x00,0x00,0x00,            /* Presentation time stamp field */
    0x00,0x00,0x00,0x00,0x00,0x00   /* Source clock reference field */
};

/* Video Probe Commit Control */
uint8_t glCommitCtrl[CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED] __attribute__ ((aligned (32)));

CyU3PDmaChannel          glChHandleUVCStream;           /* DMA Channel Handle  */
static volatile CyBool_t glIsApplnActive = CyFalse;     /* Whether the loopback application is active or not. */
static volatile CyBool_t glIsDevConfigured = CyFalse;   /* Whether SET_CONFIG is complete or not. */

/* Application error handler */
void
CyFxAppErrorHandler (
        CyU3PReturnStatus_t /*apiRetStatus*/    /* API return status */
        )
{
    /* Application failed with the error code apiRetStatus */

    /* Add custom debug or recovery actions here */

    /* Loop indefinitely */
    for (;;)
    {
        /* Thread sleep : 100 ms */
        CyU3PThreadSleep (100);
    }
}

/* This function initializes the debug module for the UVC application */
void
CyFxUVCApplnDebugInit (void)
{
    CyU3PUartConfig_t uartConfig;
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

    /* Initialize the UART for printing debug messages */
    apiRetStatus = CyU3PUartInit();
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* Set UART Configuration */
    uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
    uartConfig.stopBit = CY_U3P_UART_ONE_STOP_BIT;
    uartConfig.parity = CY_U3P_UART_NO_PARITY;
    uartConfig.txEnable = CyTrue;
    uartConfig.rxEnable = CyFalse;
    uartConfig.flowCtrl = CyFalse;
    uartConfig.isDma = CyTrue;

    /* Set the UART configuration */
    apiRetStatus = CyU3PUartSetConfig (&uartConfig, NULL);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* Set the UART transfer */
    apiRetStatus = CyU3PUartTxSetBlockXfer (0xFFFFFFFF);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* Initialize the debug application */
    apiRetStatus = CyU3PDebugInit (CY_U3P_LPP_SOCKET_UART_CONS, 8);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyFxAppErrorHandler(apiRetStatus);
    }

    CyU3PDebugPrint(1, "Hello, world!\r\n");
}

/* Helper function to set USB descriptors and handle errors */
static void CyFxUVCSetUsbDescOrFail(CyU3PUSBSetDescType_t descType, uint8_t index, const uint8_t* desc)
{
    CyU3PReturnStatus_t apiRetStatus = CyU3PUsbSetDesc(descType, index, const_cast<uint8_t*>(desc));
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint(4, "USB set descriptor failed, Type = %d, Index = %d, Error code = %d\n", descType, index, apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }
}

/* Add this function near the top of the file, after includes. */
static void logUsbSetup(const UsbSetup& usbRqt)
{
    /* Print each field in the setup packet for debugging */
    CyU3PDebugPrint(4,
        "SETUP: bmRequestType=%d bRequest=%d wValue=%d wIndex=%d wLength=%d\r\n",
        usbRqt.fields.bmRequestType,
        usbRqt.fields.bRequest,
        usbRqt.fields.wValue,
        usbRqt.fields.wIndex,
        usbRqt.fields.wLength);
}

/* This function starts the video streaming application. It is called
 * when there is a SET_INTERFACE event for alternate interface 1. */
CyU3PReturnStatus_t
CyFxUVCApplnStart (void)
{
    CyU3PEpConfig_t epCfg;
    CyU3PDmaChannelConfig_t dmaCfg;
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    CyU3PUSBSpeed_t usbSpeed = CyU3PUsbGetSpeed();

    /* Video streaming endpoint configuration */
    epCfg.enable = CyTrue;
    epCfg.epType = CY_U3P_USB_EP_BULK;
    epCfg.pcktSize = CY_FX_EP_BULK_VIDEO_PKT_SIZE;
    epCfg.isoPkts = 0;
    epCfg.burstLen = (usbSpeed == CY_U3P_SUPER_SPEED) ? CY_FX_BULK_BURST : 1;
    epCfg.streams = 0;

    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_BULK_VIDEO, &epCfg);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "CyU3PSetEpConfig failed, Error Code = %d\n", apiRetStatus);
        return apiRetStatus;
    }

    dmaCfg.size = CY_FX_UVC_STREAM_BUF_SIZE;
    dmaCfg.count = CY_FX_UVC_STREAM_BUF_COUNT;
    dmaCfg.prodSckId = CY_U3P_CPU_SOCKET_PROD;
    dmaCfg.consSckId = CY_FX_EP_VIDEO_CONS_SOCKET;
    dmaCfg.dmaMode = CY_U3P_DMA_MODE_BYTE;
    dmaCfg.cb = NULL;
    dmaCfg.prodHeader = 0;
    dmaCfg.prodFooter = 0;


    dmaCfg.consHeader = 0;
    dmaCfg.prodAvailCount = 0;
    apiRetStatus = CyU3PDmaChannelCreate (&glChHandleUVCStream, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaCfg);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "CyU3PDmaChannelCreate failed, error code = %d\n",apiRetStatus);
        return apiRetStatus;
    }

    /* Flush the endpoint memory */
    CyU3PUsbFlushEp(CY_FX_EP_BULK_VIDEO);

    apiRetStatus = CyU3PDmaChannelSetXfer (&glChHandleUVCStream, 0);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "CyU3PDmaChannelSetXfer failed, error code = %d\n", apiRetStatus);
        return apiRetStatus;
    }

    /* Update the flag so that the application thread is notified of this. */
    glIsApplnActive = CyTrue;

    return CY_U3P_SUCCESS;
}

/* This function stops the video streaming. It is called from the USB event
 * handler, when there is a reset / disconnect or SET_INTERFACE for alternate
 * interface 0. */
void
CyFxUVCApplnStop (void)
{
    CyU3PEpConfig_t epCfg;

    /* Update the flag so that the application thread is notified of this. */
    glIsApplnActive = CyFalse;

    /* Abort and destroy the video streaming channel */
    CyU3PDmaChannelDestroy (&glChHandleUVCStream);

    /* Flush the endpoint memory */
    CyU3PUsbFlushEp(CY_FX_EP_BULK_VIDEO);

    /* Disable the video streaming endpoint. */
    CyU3PMemSet ((uint8_t *)&epCfg, 0, sizeof (epCfg));
    epCfg.enable = CyFalse;
    CyU3PSetEpConfig(CY_FX_EP_BULK_VIDEO, &epCfg);
}

/* This is the Callback function to handle the USB Events */
static void
CyFxUVCApplnUSBEventCB (
    CyU3PUsbEventType_t evtype, /* Event type */
    uint16_t            evdata  /* Event data */
    )
{
    switch (evtype)
    {
        case CY_U3P_USB_EVENT_SETCONF:
            if (evdata != 0)
                glIsDevConfigured = CyTrue;
            else
                glIsDevConfigured = CyFalse;
            /* Fall-through */

        case CY_U3P_USB_EVENT_SETINTF:
            /* Stop the application before re-starting. */
            if (glIsApplnActive)
            {
                CyFxUVCApplnStop ();
            }
            CyFxUVCApplnStart ();
            break;

        case CY_U3P_USB_EVENT_RESET:
        case CY_U3P_USB_EVENT_DISCONNECT:
            /* Stop the video streamer application. */
            if (glIsApplnActive)
            {
                CyFxUVCApplnStop ();
            }
            glIsDevConfigured = CyFalse;
            break;

        default:
            break;
    }
}

// Helper for standard requests
static CyBool_t CyFxUVCHandleStandardRequest(const UsbSetup& usbRqt)
{
    CyBool_t isHandled = CyFalse;
    uint8_t bTarget = (usbRqt.fields.bmRequestType & CY_U3P_USB_TARGET_MASK);
    uint8_t bRequest = usbRqt.fields.bRequest;
    uint16_t wValue = usbRqt.fields.wValue;

    if ((bTarget == CY_U3P_USB_TARGET_INTF) &&
        ((bRequest == CY_U3P_USB_SC_SET_FEATURE) || (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE)) &&
        (wValue == 0))
    {
        if (glIsDevConfigured)
            CyU3PUsbAckSetup();
        else
            CyU3PUsbStall(0, CyTrue, CyFalse);

        isHandled = CyTrue;
    }
    return isHandled;
}

// Helper for Video Control interface requests
static CyBool_t CyFxUVCHandleVCRequest(const UsbSetup& usbRqt)
{
    CyBool_t isHandled = CyFalse;
    uint16_t wIndex = usbRqt.fields.wIndex;
    uint16_t wValue = usbRqt.fields.wValue;
    //uint8_t bRequest = usbRqt.fields.bRequest;
    uint8_t temp;

    if ((CY_U3P_GET_LSB(wIndex) == CY_FX_UVC_INTERFACE_VC) &&
        (CY_U3P_GET_MSB(wIndex) == 0x00) &&
        (wValue == CY_FX_USB_UVC_VC_RQT_ERROR_CODE_CONTROL))
    {
        temp = CY_FX_USB_UVC_RQT_STAT_INVALID_CTRL;
        isHandled = CyTrue;
        CyU3PUsbSendEP0Data(0x01, &temp);
    }
    return isHandled;
}

// Helper for Video Streaming interface requests
static CyBool_t CyFxUVCHandleVSRequest(const UsbSetup& usbRqt)
{
    CyBool_t isHandled = CyFalse;
    uint16_t wIndex = usbRqt.fields.wIndex;
    uint16_t wValue = usbRqt.fields.wValue;
    uint8_t bRequest = usbRqt.fields.bRequest;
    CyU3PReturnStatus_t status;
    uint16_t readCount = 0;

    if (CY_U3P_GET_LSB(wIndex) != CY_FX_UVC_INTERFACE_VS)
        return CyFalse;

    isHandled = CyTrue;
    switch (wValue)
    {
        case CY_FX_USB_UVC_VS_PROBE_CONTROL:
        case CY_FX_USB_UVC_VS_COMMIT_CONTROL:
            switch (bRequest)
            {
                case CY_FX_USB_UVC_GET_CUR_REQ:
                case CY_FX_USB_UVC_GET_DEF_REQ:
                case CY_FX_USB_UVC_GET_MIN_REQ:
                case CY_FX_USB_UVC_GET_MAX_REQ:
                    status = CyU3PUsbSendEP0Data(CY_FX_UVC_MAX_PROBE_SETTING, (uint8_t *)glProbeCtrl);
                    if (status != CY_U3P_SUCCESS)
                        CyU3PDebugPrint(4, "CyU3PUsbSendEP0Data, error code = %d\n", status);
                    break;

                case CY_FX_USB_UVC_SET_CUR_REQ:
                    CyU3PUsbLPMDisable();
                    status = CyU3PUsbGetEP0Data(CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED, glCommitCtrl, &readCount);
                    if (status != CY_U3P_SUCCESS)
                        CyU3PDebugPrint(4, "CyU3PUsbGetEP0Data failed, error code = %d\n", status);
                    if (readCount != (uint16_t)CY_FX_UVC_MAX_PROBE_SETTING)
                        CyU3PDebugPrint(4, "Invalid number of bytes received in SET_CUR Request");
                    break;

                default:
                    CyU3PUsbStall(0, CyTrue, CyFalse);
                    break;
            }
            break;

        default:
            CyU3PUsbStall(0, CyTrue, CyFalse);
            break;
    }
    return isHandled;
}

static CyBool_t
CyFxUVCApplnUSBSetupCB (
        uint32_t setupdat0,
        uint32_t setupdat1
    )
{
    UsbSetup usbRqt;
    usbRqt.raw.setupdat0 = setupdat0;
    usbRqt.raw.setupdat1 = setupdat1;

    logUsbSetup(usbRqt);

    uint8_t bReqType = usbRqt.fields.bmRequestType;
    uint8_t bType = (bReqType & CY_U3P_USB_TYPE_MASK);
    uint8_t bTarget = (bReqType & CY_U3P_USB_TARGET_MASK);
    uint8_t bRequest = usbRqt.fields.bRequest;
    uint16_t wIndex = usbRqt.fields.wIndex;
    uint16_t wValue = usbRqt.fields.wValue;
    CyBool_t isHandled = CyFalse;

    if (bType == CY_U3P_USB_STANDARD_RQT)
    {
        isHandled = CyFxUVCHandleStandardRequest(usbRqt);
    }

    if (bType == CY_U3P_USB_CLASS_RQT)
    {
        CyU3PDebugPrint(4, "UVC RQT: %x %x %x %x %x\r\n", bTarget, bRequest, CY_U3P_GET_MSB(wIndex),
                        CY_U3P_GET_LSB(wIndex), wValue);

        if (CY_U3P_GET_LSB(wIndex) == CY_FX_UVC_INTERFACE_VC)
        {
            isHandled = CyFxUVCHandleVCRequest(usbRqt) || isHandled;
        }
        if (CY_U3P_GET_LSB(wIndex) == CY_FX_UVC_INTERFACE_VS)
        {
            isHandled = CyFxUVCHandleVSRequest(usbRqt) || isHandled;
        }
    }

    return isHandled;
}

/* Callback function to handle LPM requests from the USB 3.0 host. This function is invoked by the API
   whenever a state change from U0 -> U1 or U0 -> U2 happens. If we return CyTrue from this function, the
   FX3 device is retained in the low power state. If we return CyFalse, the FX3 device immediately tries
   to trigger an exit back to U0.

   This application does not have any state in which we should not allow U1/U2 transitions; and therefore
   the function always return CyTrue.
 */
CyBool_t
CyFxApplnLPMRqtCB (
        CyU3PUsbLinkPowerMode /*link_mode*/)
{
    return CyTrue;
}

/* This function initializes the USB Module, creates event group,
   sets the enumeration descriptors, configures the Endpoints and
   configures the DMA module for the UVC Application */
void
CyFxUVCApplnInit (void)
{
    CyU3PEpConfig_t endPointConfig;

    /* Start the USB functionality */
    CyU3PReturnStatus_t apiRetStatus = CyU3PUsbStart();
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "USB Function Failed to Start, Error Code = %d\n",apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* The fast enumeration is the easiest way to setup a USB connection,
     * where all enumeration phase is handled by the library. Only the
     * class / vendor requests need to be handled by the application. */
    CyU3PUsbRegisterSetupCallback(CyFxUVCApplnUSBSetupCB, CyTrue);

    /* Setup the callback to handle the USB events */
    CyU3PUsbRegisterEventCallback(CyFxUVCApplnUSBEventCB);

    /* Register a callback to handle LPM requests from the USB 3.0 host. */
    CyU3PUsbRegisterLPMRequestCallback(CyFxApplnLPMRqtCB);    
    
    /* Set the USB Enumeration descriptors using the helper function */
    CyFxUVCSetUsbDescOrFail(CY_U3P_USB_SET_SS_DEVICE_DESCR, 0, CyFxUSB30DeviceDscr);
    CyFxUVCSetUsbDescOrFail(CY_U3P_USB_SET_HS_DEVICE_DESCR, 0, CyFxUSB20DeviceDscr);
    CyFxUVCSetUsbDescOrFail(CY_U3P_USB_SET_SS_BOS_DESCR, 0, CyFxUSBBOSDscr);
    CyFxUVCSetUsbDescOrFail(CY_U3P_USB_SET_DEVQUAL_DESCR, 0, CyFxUSBDeviceQualDscr);
    CyFxUVCSetUsbDescOrFail(CY_U3P_USB_SET_SS_CONFIG_DESCR, 0, CyFxUSBSSConfigDscr);
    CyFxUVCSetUsbDescOrFail(CY_U3P_USB_SET_HS_CONFIG_DESCR, 0, CyFxUSBHSConfigDscr);
    CyFxUVCSetUsbDescOrFail(CY_U3P_USB_SET_FS_CONFIG_DESCR, 0, CyFxUSBFSConfigDscr);
    CyFxUVCSetUsbDescOrFail(CY_U3P_USB_SET_STRING_DESCR, 0, CyFxUSBStringLangIDDscr);
    CyFxUVCSetUsbDescOrFail(CY_U3P_USB_SET_STRING_DESCR, 1, CyFxUSBManufactureDscr);
    CyFxUVCSetUsbDescOrFail(CY_U3P_USB_SET_STRING_DESCR, 2, CyFxUSBProductDscr);

    /* Since the status interrupt endpoint is not used in this application,
     * just enable the EP in the beginning. */
    /* Control status interrupt endpoint configuration */
    endPointConfig.enable = 1;
    endPointConfig.epType = CY_U3P_USB_EP_INTR;
    endPointConfig.pcktSize = 64;
    endPointConfig.isoPkts  = 1;
    endPointConfig.burstLen = 1;

    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_CONTROL_STATUS, &endPointConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "CyU3PSetEpConfig failed, error code = %d\n",apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* Connect the USB pins and enable super speed operation */
    apiRetStatus = CyU3PConnectState(CyTrue, CyTrue);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "USB connect failed, Error Code = %d\n",apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }
}

/* UVC header addition function */
static void
CyFxUVCAddHeader (
        uint8_t *buffer_p, /* Buffer pointer */
        uint8_t frameInd   /* EOF or normal frame indication */
    )
{
    /* Copy header to buffer */
    CyU3PMemCopy (buffer_p, (uint8_t *)glUVCHeader, CY_FX_UVC_MAX_HEADER);

    /* Check if last packet of the frame. */
    if (frameInd == CY_FX_UVC_HEADER_EOF)
    {
        /* Modify UVC header to toggle Frame ID */
        glUVCHeader[1] ^= CY_FX_UVC_HEADER_FRAME_ID;

        /* Indicate End of Frame in the buffer */
        buffer_p[1] |=  CY_FX_UVC_HEADER_EOF;
    }
}

/* Entry function for the UVC application thread. */
void
UVCAppThread_Entry (
        uint32_t /*input*/)
{
    CyU3PDmaBuffer_t dmaBuffer;
    uint16_t commitLength = 0;
    uint32_t frameStart = 0, frameIndex = 0, frameOffset = 0;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    /* Initialize the Debug Module */
    CyFxUVCApplnDebugInit();

    /* Initialize the UVC Application */
    CyFxUVCApplnInit();

    for (;;)
    {
        frameStart = 0;
        frameIndex = 0;
        frameOffset = 0;

        /* Reset Frame Id in UVC Header */
        glUVCHeader[1] = CY_FX_UVC_HEADER_DEFAULT_BFH;

        /* Video streamer application. */
        while (glIsApplnActive)
        {
            CyU3PThreadSleep(250);
            /* Wait for a free buffer. */
            status = CyU3PDmaChannelGetBuffer (&glChHandleUVCStream,
                    &dmaBuffer,  CYU3P_WAIT_FOREVER);
            if (status != CY_U3P_SUCCESS)
            {
            	break;
            }

            /* Add headers on every frame. Need to check if the EOF bit has to be set. */
            if (frameOffset + (CY_FX_UVC_STREAM_BUF_SIZE - CY_FX_UVC_MAX_HEADER) < glVidFrameLen[frameIndex])
            {
                /* Not the end of frame. */
                CyFxUVCAddHeader (dmaBuffer.buffer, CY_FX_UVC_HEADER_FRAME);


                CyU3PMemCopy ((dmaBuffer.buffer + CY_FX_UVC_MAX_HEADER),
                        (uint8_t *)&glUVCVidFrames[frameStart + frameOffset],
                        (CY_FX_UVC_STREAM_BUF_SIZE - CY_FX_UVC_MAX_HEADER));

                commitLength = CY_FX_UVC_STREAM_BUF_SIZE;
                frameOffset += (CY_FX_UVC_STREAM_BUF_SIZE - CY_FX_UVC_MAX_HEADER);
            }
            else
            {
                /* Short packet: End of frame. */
                CyFxUVCAddHeader(dmaBuffer.buffer, CY_FX_UVC_HEADER_EOF);

                commitLength = static_cast<uint16_t>((glVidFrameLen[frameIndex] - frameOffset) + CY_FX_UVC_MAX_HEADER);
                CyU3PMemCopy ((dmaBuffer.buffer + CY_FX_UVC_MAX_HEADER),
                        (uint8_t *)&glUVCVidFrames[frameStart + frameOffset],
                        (glVidFrameLen[frameIndex] - frameOffset));
            }

            /* Commit the buffer for transfer */
            status = CyU3PDmaChannelCommitBuffer (&glChHandleUVCStream, commitLength, 0);
            if (status != CY_U3P_SUCCESS)
            {
                break;
            }

            /* Move the USB link to U0 if we are stuck in U1/U2. */
            if (CyU3PUsbGetSpeed () == CY_U3P_SUPER_SPEED)
            {
                CyU3PUsbLinkPowerMode u3mode;

                if ((CyU3PUsbGetLinkPowerState (&u3mode) == CY_U3P_SUCCESS) && (
                            (u3mode == CyU3PUsbLPM_U1) || (u3mode == CyU3PUsbLPM_U2)))
                {
                    CyU3PUsbSetLinkPowerState (CyU3PUsbLPM_U0);
                }
            }

            if (commitLength < CY_FX_UVC_STREAM_BUF_SIZE)
            {
                /* Finished the frame: Move to the next frame. */
                frameOffset = 0;
                frameStart += glVidFrameLen[frameIndex];
                frameIndex++;

                /* If all frames are transferred then start from 0 */
                if (frameIndex >= CY_FX_UVC_MAX_VID_FRAMES)
                {
                    frameIndex = 0;
                    frameStart = 0;
                }
            }
        }

        /* There is a streamer error. Flag it. */
        if ((status != CY_U3P_SUCCESS) && (glIsApplnActive))
        {
            CyU3PDebugPrint (4, "UVC video streamer error. Code %d.\n", status);
            CyFxAppErrorHandler (status);
        }

        /* Sleep for sometime as video streamer is idle. */
        CyU3PThreadSleep (100);

    } /* End of for(;;) */
}

/* Application define function which creates the threads. */
void
CyFxApplicationDefine (
        void)
{
    void *ptr = NULL;
    uint32_t retThrdCreate = CY_U3P_SUCCESS;

    /* Allocate the memory for the thread and create the thread */
    ptr = CyU3PMemAlloc (UVC_APP_THREAD_STACK);
    retThrdCreate = CyU3PThreadCreate (&uvcAppThread,   /* UVC Thread structure */
                           "30:UVC_app_thread",         /* Thread Id and name */
                           UVCAppThread_Entry,          /* UVC Application Thread Entry function */
                           0,                           /* No input parameter to thread */
                           ptr,                         /* Pointer to the allocated thread stack */
                           UVC_APP_THREAD_STACK,        /* UVC Application Thread stack size */
                           UVC_APP_THREAD_PRIORITY,     /* UVC Application Thread priority */
                           UVC_APP_THREAD_PRIORITY,     /* Pre-emption threshold */
                           CYU3P_NO_TIME_SLICE,         /* No time slice for the application thread */
                           CYU3P_AUTO_START             /* Start the Thread immediately */
                           );

    /* Check the return code */
    if (retThrdCreate != 0)
    {
        /* Thread Creation failed with the error code retThrdCreate */

        /* Add custom recovery or debug actions here */

        /* Application cannot continue */
        /* Loop indefinitely */
        while(1);
    }
}

/* [ ] */

