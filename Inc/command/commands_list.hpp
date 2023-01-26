//
// Created by Andrew Yaroshevych on 02.12.2022.
//

#ifndef PHONE_COMMANDS_LIST_HPP
#define PHONE_COMMANDS_LIST_HPP

#define AT "AT" // Test communication with shield.
#define AT_CPOF "AT+CPOF" // Turn off the module.
#define ATS0 "ATS0" // Set/request the number of rings before auto answer.
#define ATS3 "ATS3" // Set/request code for end character of command line.
#define ATS4 "ATS4" // Set/request character code for the beginning of the command line.
#define ATS5 "ATS5" // Set/query backspace character code for command line.
#define ATO "ATO" // Switching from mode (offline) to data mode (online), aka PPP mode.
#define AT_F "AT&F" // Restore factory settings.
#define ATV0 "ATV0" // Set short headers and numeric response codes.
#define ATV1 "ATV1" // Set full headers and text responses (as in the examples in this article).
#define ATE0 "ATE0" // Disable AT command echo.
#define ATE1 "ATE1" // Enable AT command echo.
#define AT_W "AT&W" // Save current settings to EEPROM as user profile.
#define ATZ "ATZ" // Load settings from user profile.
#define ATQ0 "ATQ0" // Allow transmission of result codes.
#define ATQ1 "ATQ1" // Block transmission of result codes.
#define ATX0 "ATX0" // Return connection result code only.
#define ATX1 "ATX1" // Return the code and text of the connection result.
#define ATX2 "ATX2" // Return code and text of connection result, incl. tone detection.
#define ATX3 "ATX3" // Return code and text of connection result, incl. definition of employment.
#define ATX4 "ATX4" // Return code and text of connection result, incl. tone detection and occupancy detection.
#define AT_CFUN "AT+CFUN" // Set functionality.
#define AT_CMEE "AT+CMEE" // Set/query error output format.
#define AT_CSCS "AT+CSCS" // Set/request character set.
#define AT_CMUX "AT+CMUX" // Set/query multiplexing mode.
#define AT_ICF "AT+ICF" // Set/request parameters for data transfer via the UART bus.
#define AT_IPR "AT+IPR" // Set/request the baud rate on the UART bus.
#define AT_GSN "AT+GSN" // Request adapter serial number.
#define AT_GMM (AT_CGMM) "AT+GMM (AT+CGMM)" // Request adapter (module) model.
#define AT_GMR (AT_CGMR) "AT+GMR (AT+CGMR)" // Request the software version of the adapter (module).
#define AT_GMI (AT_CGMI) "AT+GMI (AT+CGMI)" // Request for adapter (module) manufacturer.
#define ATI "ATI" // Request the manufacturer, model and version of the adapter (module).
#define AT_CIMI "AT+CIMI" // Request IMSI (SIM card code).
#define AT_EGMR "AT+EGMR" // Set/request IMEI (device code).
#define AT_CALA "AT + CALA" // Set / request a list of alarms.
#define AT_CALD "AT+CALD" // Delete alarms.
#define AT_VGR "AT+VGR" // Set/query speaker gain level.
#define AT_VGT "AT+VGT" // Set/query the microphone gain level.
#define AT_CLVL "AT+CLVL" // Set/request speaker volume.
#define AT_CMUT "AT + CMUT" // Set / request to mute the microphone during a voice call.
#define AT_CCLK "AT+CCLK" // Set/request real time clock (RTC).
#define AT_CBC "AT + CBC" // Request the status of the memory and battery capacity (battery).
#define AT_CBCM "AT + CBCM" // Management of notifications about changes in the capacity of the battery (battery).
#define AT_CMER "AT+CMER" // Set/request the mode of sending unsolicited reports (URC) about events.
#define AT_CEER "AT+CEER" // Request extended last connection error report.
#define AT_CPAS "AT+CPAS" // Request for mobile device activity status.
#define AT_CPIN "AT+CPIN" // SIM card authentication (PIN1, PUK1, PIN2, PUK2 input).
#define AT_CPIN2 "AT+CPIN2" // SIM card authentication (PIN2, PUK2 input).
#define AT_CPINC "AT^CPINC" // Request the remaining number of attempts to enter passwords (PIN1, PUK1, PIN2, PUK2).
#define AT_CLCK "AT+CLCK" // Lock/unlock device functions.
#define AT_CPWD "AT+CPWD" // Change SIM card password (PIN1, PIN2).
#define AT_CRSM "AT+CRSM" // Sending commands to the SIM card in restricted access mode.
#define AT_CNUM "AT+CNUM" // MSISDN subscriber number request (display your phone number).
#define AT_CPBS "AT + CPBS" // Set / request a memory area for the phone book.
#define AT_CPBR "AT+CPBR" // Read phone book entries.
#define AT_CPBF "AT+CPBF" // Search for phone book entries.
#define AT_CPBW "AT + CPBW" // Editing the phone book (adding, replacing, deleting entries).
#define ATA "ATA" // Answer an incoming voice call.
#define ATH "ATH" // Disconnect voice call.
#define AT_CHUP "AT+CHUP" // Disconnect all existing voice calls.
#define ATD "ATD" // Dialing (outgoing voice call).
#define AT_DLST "AT+DLST" // Redial the last outgoing call.
#define AT_CHLD "AT+CHLD" // Manage group calls and conference calls.
#define AT_CLCC "AT+CLCC" // Request a list of current calls.
#define AT_VTD "AT+VTD" // Set/request the duration of DTMF tones.
#define AT_VTS "AT+VTS" // Send DTMF tone.
#define AT_VTSEX "AT+VTSEX" // Send a special DTMF tone.
#define AT_COPN "AT+COPN" // Request a list of all operator names from the module's memory.
#define AT_COPS "AT+COPS" // Request/select PLMN carrier.
#define AT_CREG "AT+CREG" // Setting/querying the mode and status of registration in the operator's network.
#define AT_CSQ "AT+CSQ" // Signal strength request.
#define AT_CPOL "AT+CPOL" // Set/request a list of preferred operators.
#define CACM "CACM" // Reset / call counter request.
#define CAMM "CAMM" // Set the maximum value for the call counter.
#define CAOC "CAOC" // Call Cost Announcement (not supported by A6 and A9 modules).
#define CPUC "CPUC" // Set/request the cost of c.u. calls and currency tables.
#define CCFC "CCFC" // Call Forwarding Control.
#define CCWA "CCWA" // Call Waiting Service Management.
#define CLIP "CLIP" // Calling Line Identification Control (CLI).
#define CLIR "CLIR" // Calling Line Identification Restriction Control (anti ANI).
#define COLP "COLP" // Connected line identification management.
#define CSSN "CSSN" // Supplementary Service Notification Management.
#define AT_CUSD "AT+CUSD" // Management and execution of USSD requests (Unstructured Supplementary Service Data).
#define STA "STA" // SAT Interface Activation (SIM Application Toolkit)
#define STN "STN" // STK Notification (SIM ToolKit)
#define STGI "STGI" // Request information from STK (SIM ToolKit)
#define STR "STR" // SAT Remote Response (SIM Application Toolkit)
#define STF "STF" // Set SAT response format (SIM Application Toolkit)
#define AT_CSDH "AT+CSDH" // Enable/disable display of text message headers.
#define AT_CSMP "AT+CSMP" // Set/request text mode options.
#define AT_CMSS "AT+CMSS" // Sending SMS messages from memory.
#define AT_CMGD "AT+CMGD" // Delete SMS message from memory.
#define AT_CMGF "AT+CMGF" // Set/request the format of SMS messages (text / PDU).
#define AT_CMGL "AT+CMGL" // Read the list of SMS messages from memory.
#define AT_CMGR "AT+CMGR" // Read one SMS message from memory.
#define AT_CMGS "AT+CMGS" // Sending an SMS message without saving to memory.
#define AT_CMGW "AT+CMGW" // Write SMS message to memory.
#define AT_CNMA "AT+CNMA" // Request a report on the delivery of SMS messages.
#define AT_CNMI "AT+CNMI" // Set/request indication of new messages.
#define AT_CPMS "AT+CPMS" // Set/query the preferred memory area for storing messages.
#define AT_CSCA "AT+CSCA" // Setting/requesting the number of the service center for SMS messages.
#define AT_CSCB "AT+CSCB" // Set/query the type of received Cell Broadcast messages.
#define AT_CSAS "AT+CSAS" // Saving settings.
#define AT_CRES "AT+CRES" // Restore settings.
#define AT_CAUDIO "AT+CAUDIO" // Open/close the audio stream of a voice call.
#define AT_CRSL "AT+CRSL" // Set/query the volume level of the call.
#define AT_CDTMF "AT+CDTMF" // Play DTMF tone.
#define AT_AUST "AT+AUST" // Start a test audio cycle.
#define AT_AUEND "AT+AUEND" // Stop test audio cycle.
#define AT_SNFS "AT+SNFS" // Install/request audio input/output device.
#define CGATT "CGATT" // Establish/break GPRS connection.
#define CGDCONT "CGDCONT" // Setting PDP context parameters.
#define CGACT "CGACT" // Activate/deactivate a PDP context.
#define CRC "CRC" // Enable/disable the result codes of incoming connections.
#define CGQMIN "CGQMIN" // Setting the minimum allowed QoS (Quality of Service) profile.
#define CGPADDR "CGPADDR" // Get a list of PDP addresses.
#define CGAUTO "CGAUTO" // Enable/disable auto-response of PDP context activation.
#define CGQREQ "CGQREQ" // Setting the requested QoS (Quality of Service) profile.
#define CGREG "CGREG" // GPRS network registration status.
#define CGSMS "CGSMS" // Service selection (GSM or GPRS) for sending SMS messages.
#define CGANS "CGANS" // Manual PDP context activation response to a network request (see CGAUTO).
#define CGEREP "CGEREP" // Enable/disable GPRS event result codes.
#define CGDATA "CGDATA" // Establishing a connection with the network (similar to the ATD * 99 *** 1 # command).
#define CGCLASS "CGCLASS" // Set/request the GPRS class of the mobile device.
#define CIPTART "CIPTART" // Open a TCP or UDP connection.
#define CIPSEND "CIPSEND" // Sending data over a TCP or UDP connection.
#define CIPCLOSE "CIPCLOSE" // Close a TCP or UDP connection.
#define CIPSHUT "CIPSHUT" // Disable the wireless connection.
#define CSTT "CSTT" // Enter the access point name, login and password.
#define CIICR "CIICR" // Setting up a wireless connection.
#define CIFSR "CIFSR" // Obtaining a local IP address.
#define CIPSTATUS "CIPSTATUS" // Request the status of the current connection.
#define CIPATS "CIPATS" // Set the automatic send timer.
#define CIPSCONT "CIPSCONT" // Save TCP/IP context parameters.
#define CDNSGIP "CDNSGIP" // Request the IP address of the specified domain name.


#endif //PHONE_COMMANDS_LIST_HPP
