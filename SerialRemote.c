/*
  SerialRemote: Issue power / multimedia commands via. serial port
  Copyright 2020 Jamie Lentin (jm@lentin.co.uk)

  Based on LUFA Library:
*/

/*
             LUFA Library
     Copyright (C) Dean Camera, 2019.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2019  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "SerialRemote.h"

/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevHIDReportBuffer[
	MAX(sizeof(USB_KeyboardReport_Data_t), MAX(
		sizeof(USB_MediaReport_Data_t),
		sizeof(USB_SystemControlReport_Data_t)
	))
];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t MediaControl_HID_Interface =
 	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_HID,
				.ReportINEndpoint             =
					{
						.Address              = MEDIACONTROL_HID_EPADDR,
						.Size                 = MEDIACONTROL_HID_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevHIDReportBuffer),
			},
    };

uint8_t clear_event = 0;

/** Collect user input, either from serial port or button */
uint8_t get_state(void)
{

	int16_t serial_in = Serial_ReceiveByte();
	if (serial_in >= 0) {
		return (uint8_t)serial_in;
	}

#ifdef BUTTONS_BUTTON1
	uint8_t ButtonStatus_LCL = Buttons_GetStatus();
	if ((ButtonStatus_LCL & BUTTONS_BUTTON1)) {
		/* Map BUTTON1 to receiving 0x01 */
		return 0x01;
	}
#endif

	return 0x00;
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	uint8_t state;

	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	for (;;)
	{
		HID_Device_USBTask(&MediaControl_HID_Interface);
		USB_USBTask();

		if (USB_DeviceState != DEVICE_STATE_Configured ) { // TODO: Should be checking for suspended?
			if (USB_Device_RemoteWakeupEnabled) {
				state = get_state();
				if (state == 0x01 || state == 0x31) { // Button or "1"
					USB_Device_SendRemoteWakeup();
				}
			}
		}
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#endif

	/* Hardware Initialization */
	LEDs_Init();
#ifdef BUTTONS_BUTTON1
	Buttons_Init();
#endif
	Serial_Init(600, false);
	USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&MediaControl_HID_Interface);

	USB_Device_EnableSOFEvents();

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&MediaControl_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&MediaControl_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	if (clear_event > 0) {
		/* All keypresses should be momentary, so clear last press if any */
		*ReportID = clear_event;
		if (clear_event == HID_REPORTID_SystemControlReport) {
			*ReportSize = sizeof(USB_SystemControlReport_Data_t);
		} else if (clear_event == HID_REPORTID_KeyboardReport) {
			*ReportSize = sizeof(USB_KeyboardReport_Data_t);
		} else if (clear_event == HID_REPORTID_MediaControlReport) {
			*ReportSize = sizeof(USB_MediaReport_Data_t);
		}
		clear_event = 0;
		return true;
	}
	uint8_t state = get_state();

	if (state == 0x01 || state == 0x30) {
		USB_SystemControlReport_Data_t* SystemControlReport = (USB_SystemControlReport_Data_t*)ReportData;

		*ReportID   = HID_REPORTID_SystemControlReport;
		*ReportSize = sizeof(USB_SystemControlReport_Data_t);
		clear_event = *ReportID;
		SystemControlReport->Sleep = (state == 0x01 || state == 0x30); // Button or "0"

	} else if (state == 0x20) {
		USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

		*ReportID   = HID_REPORTID_KeyboardReport;
		*ReportSize = sizeof(USB_KeyboardReport_Data_t);
		clear_event = *ReportID;
		if (state == 0x20) KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_SPACE; // (space)

	} else if (state > 0) {
		USB_MediaReport_Data_t* MediaReport = (USB_MediaReport_Data_t*)ReportData;

		*ReportID   = HID_REPORTID_MediaControlReport;
		*ReportSize = sizeof(USB_MediaReport_Data_t);
		clear_event = *ReportID;
		MediaReport->Play           = (state == 0x79); // y
		MediaReport->Pause          = (state == 0x65); // e
		MediaReport->FForward       = (state == 0x66); // f
		MediaReport->Rewind         = (state == 0x72); // r
		MediaReport->NextTrack      = (state == 0x6e); // n
		MediaReport->PreviousTrack  = (state == 0x70); // p
		MediaReport->Stop           = (state == 0x73); // s
		MediaReport->PlayPause      = (state == 0x7a); // z
		MediaReport->Mute           = (state == 0x6d); // m
		MediaReport->VolumeUp       = (state == 0x75); // u
		MediaReport->VolumeDown     = (state == 0x64); // d
	}

	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	// Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}

