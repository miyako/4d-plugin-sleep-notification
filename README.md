![version](https://img.shields.io/badge/version-1.0.0-3E8B93)
![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm&color=blue)

# 4d-plugin-sleep-notification

Receive a notification immediately before the computer goes to sleep. The plugin listens to the macOS `NSWorkspace` power and screen notifications (machine sleep/wake, power off, display sleep/wake) and calls a 4D project method of your choice for each event. It also exposes the IOKit power-source APIs: the battery warning level as a `Longint`, and power-source and power-adapter details as XML property lists in a `Text` array.

**Deprecated**: see [4d-plugin-system-notification](https://github.com/miyako/4d-plugin-system-notification/)

| Command | Returns | Purpose |
|---|---|---|
| [SN Set sleep method](#sn-set-sleep-method) | `Longint` | Install (or remove) the project method called on sleep/wake events |
| [SN Get sleep method](#sn-get-sleep-method) | `Text` | Name of the currently installed method |
| [Get battery warning level](#get-battery-warning-level) | `Longint` | Current low-battery warning level |
| [GET POWER INFORMATION](#get-power-information) | — | Power sources and external adapter details, as XML plists |

**Platforms:** macOS (Apple silicon and Intel, macOS 11.5 or later). The plugin is mac-only. The `Windows64` folder in the bundle holds an old prebuilt binary that is not built from this source.

Releases: [GitHub Releases](https://github.com/miyako/4d-plugin-sleep-notification/releases)

---

## Requirements & platform notes

- **Notifications are asynchronous.** Your method is never called inside the process that called [SN Set sleep method](#sn-set-sleep-method). Each event starts a **new 4D process** (named with a random UUID) that runs your method once, with the event code in `$1`, then ends.
- **macOS does not wait for your method.** The plugin passes the event on and returns at once. The machine may already be asleep or shutting down before a slow method finishes, so keep the "before sleep" / "before power off" work short.
- One method at a time: installing a new method replaces the previous one.
- The listener is stopped automatically when the database closes.

### Event constants

The plugin adds these constants (theme **Sleep Notification**) for the value passed in `$1`:

| Constant | Value | Event |
|---|---|---|
| `SN On After Machine Wake` | 0 | The machine woke up from sleep |
| `SN On Before Machine Sleep` | 1 | The machine is about to sleep |
| `SN On Before Machine Power Off` | 2 | The user asked to log out, restart or shut down |
| `SN On After Screen Wake` | 3 | The displays woke up |
| `SN On After Screen Sleep` | 4 | The displays went to sleep |

---

## SN Set sleep method

### Syntax

```
SN Set sleep method ( method ) → Longint
```

| Parameter | Type | Description |
|---|---|---|
| `method` | Text | Name of the project method to call for each event, or `""` to stop listening. Declared by reference in the manifest, so pass a text variable. |
| Result | Longint | `1` if the method was installed (or `""` was passed), `0` if no project method with that name exists |

### Description

When `method` names an existing project method, the plugin starts listening for the events listed in [Event constants](#event-constants). It then calls that method in a new process for every event, passing the event code as a `Longint` in `$1`. Calling it again with a different method name switches to the new method. Calling it with the same name does nothing.

Passing `""` stops the listener and clears the installed method. The result is `1`.

If `method` does not name a project method, the result is `0` and **the previously installed method, if any, stays active**.

### Example

The plugin's test database (`Sleep Notification .4dbase`) installs a method called `EVENT_HANDLER`. Its source is stored in binary form, so the code below is illustrative, not a verbatim copy:

```4d
C_TEXT($method)
C_LONGINT($success)

$method:="EVENT_HANDLER"
$success:=SN Set sleep method($method)
If ($success=0)
	ALERT("Method "+$method+" not found.")
End if
```

`EVENT_HANDLER`, the callback:

```4d
C_LONGINT($1)

Case of
	: ($1=SN On Before Machine Sleep)
		// save state, close connections: keep it short
	: ($1=SN On After Machine Wake)
		// reconnect
	: ($1=SN On Before Machine Power Off)
		// logout / restart / shutdown requested
	: ($1=SN On After Screen Sleep)
	: ($1=SN On After Screen Wake)
End case
```

Stop listening:

```4d
C_TEXT($method)
$method:=""
$success:=SN Set sleep method($method)
```

---

## SN Get sleep method

### Syntax

```
SN Get sleep method → Text
```

| Parameter | Type | Description |
|---|---|---|
| Result | Text | Name of the installed method, or `""` if none |

### Description

Returns the method name last installed with [SN Set sleep method](#sn-set-sleep-method), exactly as it was passed.

### Example

```4d
If (SN Get sleep method="")
	C_TEXT($method)
	$method:="EVENT_HANDLER"
	$success:=SN Set sleep method($method)
End if
```

---

## Get battery warning level

### Syntax

```
Get battery warning level → Longint
```

| Parameter | Type | Description |
|---|---|---|
| Result | Longint | `1` no warning, `2` early warning, `3` final warning |

### Description

Returns the value of IOKit's `IOPSGetBatteryWarningLevel`:

| Value | IOKit name | Meaning |
|---|---|---|
| 1 | `kIOPSLowBatteryWarningNone` | Not on battery, or the battery is not low |
| 2 | `kIOPSLowBatteryWarningEarly` | Early low-battery warning |
| 3 | `kIOPSLowBatteryWarningFinal` | Final warning: the machine is about to run out of power |

The thresholds are decided by macOS. The plugin defines no constants for these values; compare with the numbers.

### Example

```4d
C_LONGINT($level)
$level:=Get battery warning level
Case of
	: ($level=1)
		// kIOPSLowBatteryWarningNone
	: ($level=2)
		// kIOPSLowBatteryWarningEarly
	: ($level=3)
		// kIOPSLowBatteryWarningFinal
End case
```

---

## GET POWER INFORMATION

### Syntax

```
GET POWER INFORMATION ( powerInfos )
```

| Parameter | Type | Description |
|---|---|---|
| `powerInfos` | Text array | Receives one XML property list per power source, then one for the external power adapter |
| Result | — | No return value |

### Description

The array is resized and filled as follows:

- Element 0 is empty.
- Elements 1 to *n*: one XML property list (`<?xml … <plist>`) per power source reported by `IOPSCopyPowerSourcesInfo` (for example the internal battery or a UPS), each holding that source's description dictionary (`IOPSGetPowerSourceDescription`).
- Then, if macOS reports an external power adapter, one more element with its details (`IOPSCopyExternalPowerAdapterDetails`).

A desktop Mac with no battery may return only the adapter entry, or an empty array apart from element 0. Nothing marks which element is the adapter. Tell them apart by their keys: power-source descriptions carry IOKit `IOPSKeys.h` keys such as `Type` and `Current Capacity`. Key names and availability depend on the hardware and macOS version.

### Example

```4d
ARRAY TEXT($powerInfos;0)
GET POWER INFORMATION($powerInfos)
For ($i;1;Size of array($powerInfos))
	// $powerInfos{$i} is an XML plist: parse it with 4D's XML (DOM) commands
End For
```

---

## Error handling & troubleshooting

- **Result is 0 from SN Set sleep method**: the name doesn't match a project method. The old method (if any) is still active; check spelling and that it is a project method, not a form or object method.
- **No callback ever runs**: check `SN Get sleep method`. Screen sleep/wake events need the displays to actually sleep; machine sleep events don't happen while a power assertion keeps the Mac awake.
- **The "before sleep" work doesn't finish**: the plugin does not delay sleep. Do the minimum in the callback, or move the work elsewhere.
- **Pending events are dropped when you stop listening**: an event whose process hasn't started yet when you pass `""` is discarded, not delivered.
- **Many processes in the Runtime Explorer**: each event runs in its own short-lived process with a UUID name. This is expected.
- **Fixed in 1.0.0**: earlier builds could crash 4D when events arrived in quick succession, leaked memory on every `GET POWER INFORMATION` call, and could stop delivering events if the method was cleared and set again quickly.

---

## Quick reference

```4d
// install
C_TEXT($method)
$method:="EVENT_HANDLER"
$success:=SN Set sleep method($method)  // 1 = ok, 0 = method not found

// EVENT_HANDLER
C_LONGINT($1)
If ($1=SN On Before Machine Sleep)
	// ...
End if

// query
$name:=SN Get sleep method
$level:=Get battery warning level  // 1 none, 2 early, 3 final
ARRAY TEXT($powerInfos;0)
GET POWER INFORMATION($powerInfos)

// stop
$method:=""
$success:=SN Set sleep method($method)
```
