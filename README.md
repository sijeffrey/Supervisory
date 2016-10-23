# Supervisory Alarm System

This project is designed to replace end of life equipment no longer supported.

### Devices used:

- ATmega328p (MCP)
- MCP23017 (16 bit GPIO)
- ILQ74 (opto-coupler)
- ULN2803A (Darlington Transistor Array)

### Operation

The system scans the inputs on the MCP23017 and detects any unhealthy state (LOW) pins.
If a LOW pin is detected then the FLASH register is set and the BUZZER is activated.
Upon the ACCEPT pin going low on the ATmega328 the FLASH register is cleared and the STEADY register is set.
Upon the RESET pin going low on the ATmega328 AND the corresponding MCP23017 pin being healthy then the STEADY register is cleared.
The outputs reflect the registers (FLASH anded with an on/off timer) or STEADY or if the LAMPTEST pin is low.

### Project

This was designed to replace a PLC type function in an alarm system. Hopefully expanding to other sites in the future. A dedicated PLC was considered but cost was an issue.

### Feedback

Any ideas of how to improve the code or design would be warmly received.

### Pictures

Some photos of the project as it develops. 

![Finished product](/docs/20161022_195305.jpg)

