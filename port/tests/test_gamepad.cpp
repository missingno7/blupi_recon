// The winmm joystick calls the game makes (CEvent::ReadInput, the settings
// screen's device list) answered from an SDL gamepad: a virtual gamepad stands
// in for real hardware.

#include "windows.h"
#include "mmsystem.h"

#include "platform/host.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(bool ok, const char* what)
{
	printf("%s %s\n", ok ? "ok  " : "FAIL", what);
	if (!ok) failures++;
}

// Lets SDL deliver the virtual device's events to the host.
static void pump(blupi::host::Event* last = NULL)
{
	blupi::host::Event e;
	SDL_UpdateJoysticks();
	while (blupi::host::poll_event(e))
	{
		if (last) *last = e;
	}
}

int main()
{
	blupi::host::Options options;
	options.headless = true;
	options.gamepads = true;
	if (!blupi::host::init("test", 640, 480, options)) { printf("host init failed\n"); return 1; }

	check(joyGetNumDevs() == 0, "no device before a gamepad is connected");

	SDL_VirtualJoystickDesc desc;
	SDL_INIT_INTERFACE(&desc);
	desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
	desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
	desc.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
	desc.name = "Virtual Pad";
	SDL_JoystickID id = SDL_AttachVirtualJoystick(&desc);
	check(id != 0, "virtual gamepad attached");
	SDL_Joystick* joy = SDL_OpenJoystick(id);
	pump();

	check(joyGetNumDevs() == 1, "one device listed");
	JOYCAPS caps;
	check(joyGetDevCaps(0, &caps, sizeof(caps)) == JOYERR_NOERROR, "device 0 has capabilities");
	check(strcmp(caps.szPname, "Virtual Pad") == 0, "device name as the settings screen shows it");
	check(joyGetDevCaps(1, &caps, sizeof(caps)) != JOYERR_NOERROR, "no device 1");

	JOYINFOEX info;
	memset(&info, 0, sizeof(info));
	check(joyGetPosEx(0, &info) == JOYERR_NOERROR, "position of device 0");
	check(info.dwXpos >= 16384 && info.dwXpos <= 49152 && info.dwYpos >= 16384 && info.dwYpos <= 49152,
		  "centred stick reads as no direction (game thresholds 16384/49152)");
	check(info.dwButtons == 0, "no button");

	// Stick left and up, then the d-pad right and down.
	SDL_SetJoystickVirtualAxis(joy, SDL_GAMEPAD_AXIS_LEFTX, -32768);
	SDL_SetJoystickVirtualAxis(joy, SDL_GAMEPAD_AXIS_LEFTY, -32768);
	pump();
	joyGetPosEx(0, &info);
	check(info.dwXpos < 16384 && info.dwYpos < 16384, "stick left/up: below 16384");
	SDL_SetJoystickVirtualAxis(joy, SDL_GAMEPAD_AXIS_LEFTX, 0);
	SDL_SetJoystickVirtualAxis(joy, SDL_GAMEPAD_AXIS_LEFTY, 0);
	SDL_SetJoystickVirtualButton(joy, SDL_GAMEPAD_BUTTON_DPAD_RIGHT, true);
	SDL_SetJoystickVirtualButton(joy, SDL_GAMEPAD_BUTTON_DPAD_DOWN, true);
	pump();
	joyGetPosEx(0, &info);
	check(info.dwXpos > 49152 && info.dwYpos > 49152, "d-pad right/down: above 49152");
	SDL_SetJoystickVirtualButton(joy, SDL_GAMEPAD_BUTTON_DPAD_RIGHT, false);
	SDL_SetJoystickVirtualButton(joy, SDL_GAMEPAD_BUTTON_DPAD_DOWN, false);

	// Face buttons are joystick buttons 1 to 4.
	const struct { int button; DWORD joy; const char* what; } faces[] =
	{
		{ SDL_GAMEPAD_BUTTON_SOUTH, JOY_BUTTON1, "south (A) is button 1 (jump)" },
		{ SDL_GAMEPAD_BUTTON_EAST,  JOY_BUTTON2, "east (B) is button 2" },
		{ SDL_GAMEPAD_BUTTON_WEST,  JOY_BUTTON3, "west (X) is button 3" },
		{ SDL_GAMEPAD_BUTTON_NORTH, JOY_BUTTON4, "north (Y) is button 4 (fire)" },
	};
	for (const auto& f : faces)
	{
		SDL_SetJoystickVirtualButton(joy, f.button, true);
		pump();
		joyGetPosEx(0, &info);
		check(info.dwButtons == f.joy && info.dwButtonNumber == 1, f.what);
		SDL_SetJoystickVirtualButton(joy, f.button, false);
		pump();
	}

	// Start is the Escape key.
	blupi::host::Event last;
	last.type = blupi::host::EventType::Quit;
	SDL_SetJoystickVirtualButton(joy, SDL_GAMEPAD_BUTTON_START, true);
	pump(&last);
	check(last.type == blupi::host::EventType::KeyDown && last.key == 0x1B, "Start presses Escape");
	SDL_SetJoystickVirtualButton(joy, SDL_GAMEPAD_BUTTON_START, false);
	pump(&last);
	check(last.type == blupi::host::EventType::KeyUp && last.key == 0x1B, "Start released releases Escape");

	// Unplugged.
	SDL_CloseJoystick(joy);
	SDL_DetachVirtualJoystick(id);
	pump();
	check(joyGetNumDevs() == 0, "no device after unplugging");
	check(joyGetPosEx(0, &info) == JOYERR_UNPLUGGED, "reading it reports JOYERR_UNPLUGGED");

	blupi::host::shutdown();
	printf("%s\n", failures == 0 ? "all checks passed" : "FAILED");
	return failures == 0 ? 0 : 1;
}
