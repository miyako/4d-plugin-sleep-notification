# 4d-plugin-sleep-notification
Receive a notification immediately before the computer goes to sleep.

Platform
---

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🆗|🆗|🚫|🚫|

###TODO: Queue events.

Commands
---

```c
// --- Notification
SN_Set_sleep_method
SN_Get_sleep_method

// --- Information
Get_battery_warning_level
GET_POWER_INFORMATION
```

Examples
---

```
$event:=Form event

Case of 
: ($event=On Load)

C_POINTER(<>MESSAGE)

<>MESSAGE:=OBJECT Get pointer(Object named;"Column1")

$success:=SN Set sleep method ("EVENT_HANDLER")

: ($event=On Unload)

$success:=SN Set sleep method ("")

End case 
```

```
  //EVENT_HANDLER
C_LONGINT($1;$event)

$event:=$1

Case of 
: ($event=SN On After Machine Wake)

APPEND TO ARRAY(<>MESSAGE->;"On After Machine Wake")

: ($event=SN On After Screen Sleep)

APPEND TO ARRAY(<>MESSAGE->;"On After Screen Sleep")

: ($event=SN On After Screen Wake)

APPEND TO ARRAY(<>MESSAGE->;"On After Screen Wake")

: ($event=SN On Before Machine Power Off)

APPEND TO ARRAY(<>MESSAGE->;"On Before Machine Power Off")

: ($event=SN On Before Machine Sleep)

APPEND TO ARRAY(<>MESSAGE->;"On Before Machine Sleep")

End case 

CALL PROCESS(-1)
```

```
$level:=Get battery warning level 
```

This is a simple wrapper of [IOPSGetBatteryWarningLevel](https://developer.apple.com/library/mac/documentation/IOKit/Reference/IOPowerSources_header_reference/#//apple_ref/c/tdef/IOPSLowBatteryWarningLevel).

```
ARRAY TEXT($powerInfos;0)
GET POWER INFORMATION ($powerInfos)
  //array of XML property lists
```

The result array will contain information returned by ``IOPSGetPowerSourceDescription`` and ``IOPSCopyExternalPowerAdapterDetails``.
