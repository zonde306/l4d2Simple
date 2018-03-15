#pragma once
#include "IAppSystem.h"
#include "../Structs/bitbuffer.h"
#include "../Structs/baseentity.h"
#include "../Structs/usercmd.h"
#include "../../l4d2Simple2/vector.h"

enum JoystickAxis_t
{
	JOY_AXIS_X = 0,
	JOY_AXIS_Y,
	JOY_AXIS_Z,
	JOY_AXIS_R,
	JOY_AXIS_U,
	JOY_AXIS_V,
	MAX_JOYSTICK_AXES,
};

enum
{
	JOYSTICK_MAX_BUTTON_COUNT = 32,
	JOYSTICK_POV_BUTTON_COUNT = 4,
	JOYSTICK_AXIS_BUTTON_COUNT = MAX_JOYSTICK_AXES * 2,
};

enum
{
	MAX_JOYSTICKS = 1,
	MOUSE_BUTTON_COUNT = 5,
	MAX_NOVINT_DEVICES = 2,
};

#define JOYSTICK_BUTTON_INTERNAL( _joystick, _button ) ( JOYSTICK_FIRST_BUTTON + ((_joystick) * JOYSTICK_MAX_BUTTON_COUNT) + (_button) )
#define JOYSTICK_POV_BUTTON_INTERNAL( _joystick, _button ) ( JOYSTICK_FIRST_POV_BUTTON + ((_joystick) * JOYSTICK_POV_BUTTON_COUNT) + (_button) )
#define JOYSTICK_AXIS_BUTTON_INTERNAL( _joystick, _button ) ( JOYSTICK_FIRST_AXIS_BUTTON + ((_joystick) * JOYSTICK_AXIS_BUTTON_COUNT) + (_button) )

#define JOYSTICK_BUTTON( _joystick, _button ) ( (ButtonCode_t)JOYSTICK_BUTTON_INTERNAL( _joystick, _button ) )
#define JOYSTICK_POV_BUTTON( _joystick, _button ) ( (ButtonCode_t)JOYSTICK_POV_BUTTON_INTERNAL( _joystick, _button ) )
#define JOYSTICK_AXIS_BUTTON( _joystick, _button ) ( (ButtonCode_t)JOYSTICK_AXIS_BUTTON_INTERNAL( _joystick, _button ) )

enum ButtonCode_t
{
	BUTTON_CODE_INVALID = -1,
	BUTTON_CODE_NONE = 0,

	KEY_FIRST = 0,

	KEY_NONE = KEY_FIRST,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_PAD_0,
	KEY_PAD_1,
	KEY_PAD_2,
	KEY_PAD_3,
	KEY_PAD_4,
	KEY_PAD_5,
	KEY_PAD_6,
	KEY_PAD_7,
	KEY_PAD_8,
	KEY_PAD_9,
	KEY_PAD_DIVIDE,
	KEY_PAD_MULTIPLY,
	KEY_PAD_MINUS,
	KEY_PAD_PLUS,
	KEY_PAD_ENTER,
	KEY_PAD_DECIMAL,
	KEY_LBRACKET,
	KEY_RBRACKET,
	KEY_SEMICOLON,
	KEY_APOSTROPHE,
	KEY_BACKQUOTE,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_BACKSLASH,
	KEY_MINUS,
	KEY_EQUAL,
	KEY_ENTER,
	KEY_SPACE,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_CAPSLOCK,
	KEY_NUMLOCK,
	KEY_ESCAPE,
	KEY_SCROLLLOCK,
	KEY_INSERT,
	KEY_DELETE,
	KEY_HOME,
	KEY_END,
	KEY_PAGEUP,
	KEY_PAGEDOWN,
	KEY_BREAK,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LALT,
	KEY_RALT,
	KEY_LCONTROL,
	KEY_RCONTROL,
	KEY_LWIN,
	KEY_RWIN,
	KEY_APP,
	KEY_UP,
	KEY_LEFT,
	KEY_DOWN,
	KEY_RIGHT,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_CAPSLOCKTOGGLE,
	KEY_NUMLOCKTOGGLE,
	KEY_SCROLLLOCKTOGGLE,

	KEY_LAST = KEY_SCROLLLOCKTOGGLE,
	KEY_COUNT = KEY_LAST - KEY_FIRST + 1,

	// Mouse
	MOUSE_FIRST = KEY_LAST + 1,

	MOUSE_LEFT = MOUSE_FIRST,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,
	MOUSE_4,
	MOUSE_5,
	MOUSE_WHEEL_UP,		// A fake button which is 'pressed' and 'released' when the wheel is moved up 
	MOUSE_WHEEL_DOWN,	// A fake button which is 'pressed' and 'released' when the wheel is moved down

	MOUSE_LAST = MOUSE_WHEEL_DOWN,
	MOUSE_COUNT = MOUSE_LAST - MOUSE_FIRST + 1,

	// Joystick
	JOYSTICK_FIRST = MOUSE_LAST + 1,

	JOYSTICK_FIRST_BUTTON = JOYSTICK_FIRST,
	JOYSTICK_LAST_BUTTON = JOYSTICK_BUTTON_INTERNAL(MAX_JOYSTICKS - 1, JOYSTICK_MAX_BUTTON_COUNT - 1),
	JOYSTICK_FIRST_POV_BUTTON,
	JOYSTICK_LAST_POV_BUTTON = JOYSTICK_POV_BUTTON_INTERNAL(MAX_JOYSTICKS - 1, JOYSTICK_POV_BUTTON_COUNT - 1),
	JOYSTICK_FIRST_AXIS_BUTTON,
	JOYSTICK_LAST_AXIS_BUTTON = JOYSTICK_AXIS_BUTTON_INTERNAL(MAX_JOYSTICKS - 1, JOYSTICK_AXIS_BUTTON_COUNT - 1),

	JOYSTICK_LAST = JOYSTICK_LAST_AXIS_BUTTON,

	BUTTON_CODE_LAST,
	BUTTON_CODE_COUNT = BUTTON_CODE_LAST - KEY_FIRST + 1,

	// Helpers for XBox 360
	KEY_XBUTTON_UP = JOYSTICK_FIRST_POV_BUTTON,	// POV buttons
	KEY_XBUTTON_RIGHT,
	KEY_XBUTTON_DOWN,
	KEY_XBUTTON_LEFT,

	KEY_XBUTTON_A = JOYSTICK_FIRST_BUTTON,		// Buttons
	KEY_XBUTTON_B,
	KEY_XBUTTON_X,
	KEY_XBUTTON_Y,
	KEY_XBUTTON_LEFT_SHOULDER,
	KEY_XBUTTON_RIGHT_SHOULDER,
	KEY_XBUTTON_BACK,
	KEY_XBUTTON_START,
	KEY_XBUTTON_STICK1,
	KEY_XBUTTON_STICK2,

	KEY_XSTICK1_RIGHT = JOYSTICK_FIRST_AXIS_BUTTON,	// XAXIS POSITIVE
	KEY_XSTICK1_LEFT,							// XAXIS NEGATIVE
	KEY_XSTICK1_DOWN,							// YAXIS POSITIVE
	KEY_XSTICK1_UP,								// YAXIS NEGATIVE
	KEY_XBUTTON_LTRIGGER,						// ZAXIS POSITIVE
	KEY_XBUTTON_RTRIGGER,						// ZAXIS NEGATIVE
	KEY_XSTICK2_RIGHT,							// UAXIS POSITIVE
	KEY_XSTICK2_LEFT,							// UAXIS NEGATIVE
	KEY_XSTICK2_DOWN,							// VAXIS POSITIVE
	KEY_XSTICK2_UP,								// VAXIS NEGATIVE
};

enum MouseCodeState_t
{
	BUTTON_RELEASED = 0,
	BUTTON_PRESSED,
	BUTTON_DOUBLECLICKED,
};

enum InputEventType_t
{
	IE_ButtonPressed = 0,	// m_nData contains a ButtonCode_t
	IE_ButtonReleased,		// m_nData contains a ButtonCode_t
	IE_ButtonDoubleClicked,	// m_nData contains a ButtonCode_t
	IE_AnalogValueChanged,	// m_nData contains an AnalogCode_t, m_nData2 contains the value

	IE_FirstSystemEvent = 100,
	IE_Quit = IE_FirstSystemEvent,
	IE_ControllerInserted,	// m_nData contains the controller ID
	IE_ControllerUnplugged,	// m_nData contains the controller ID

	IE_FirstVguiEvent = 1000,	// Assign ranges for other systems that post user events here
	IE_FirstAppEvent = 2000,
};

struct InputEvent_t
{
	int m_nType;				// Type of the event (see InputEventType_t)
	int m_nTick;				// Tick on which the event occurred
	int m_nData;				// Generic 32-bit data, what it contains depends on the event
	int m_nData2;				// Generic 32-bit data, what it contains depends on the event
	int m_nData3;				// Generic 32-bit data, what it contains depends on the event
};

#define JOYSTICK_AXIS_INTERNAL( _joystick, _axis ) ( JOYSTICK_FIRST_AXIS + ((_joystick) * MAX_JOYSTICK_AXES) + (_axis) )
#define JOYSTICK_AXIS( _joystick, _axis ) ( (AnalogCode_t)JOYSTICK_AXIS_INTERNAL( _joystick, _axis ) )
enum AnalogCode_t
{
	ANALOG_CODE_INVALID = -1,
	MOUSE_X = 0,
	MOUSE_Y,
	MOUSE_XY,		// Invoked when either x or y changes
	MOUSE_WHEEL,

	JOYSTICK_FIRST_AXIS,
	JOYSTICK_LAST_AXIS = JOYSTICK_AXIS_INTERNAL(MAX_JOYSTICKS - 1, MAX_JOYSTICK_AXES - 1),

	ANALOG_CODE_LAST,
};

class IInputSystem : public IAppSystem
{
public:
	// Attach, detach input system from a particular window
	// This window should be the root window for the application
	// Only 1 window should be attached at any given time.
	virtual void AttachToWindow(void* hWnd) = 0;
	virtual void DetachFromWindow() = 0;

	// Enables/disables input. PollInputState will not update current 
	// button/analog states when it is called if the system is disabled.
	virtual void EnableInput(bool bEnable) = 0;

	// Enables/disables the windows message pump. PollInputState will not
	// Peek/Dispatch messages if this is disabled
	virtual void EnableMessagePump(bool bEnable) = 0;

	// Polls the current input state
	virtual void PollInputState() = 0;

	// Gets the time of the last polling in ms
	virtual int GetPollTick() const = 0;

	// Is a button down? "Buttons" are binary-state input devices (mouse buttons, keyboard keys)
	virtual bool IsButtonDown(ButtonCode_t code) const = 0;

	// Returns the tick at which the button was pressed and released
	virtual int GetButtonPressedTick(ButtonCode_t code) const = 0;
	virtual int GetButtonReleasedTick(ButtonCode_t code) const = 0;

	// Gets the value of an analog input device this frame
	// Includes joysticks, mousewheel, mouse
	virtual int GetAnalogValue(AnalogCode_t code) const = 0;

	// Gets the change in a particular analog input device this frame
	// Includes joysticks, mousewheel, mouse
	virtual int GetAnalogDelta(AnalogCode_t code) const = 0;

	// Returns the input events since the last poll
	virtual int GetEventCount() const = 0;
	virtual const InputEvent_t* GetEventData() const = 0;

	// Posts a user-defined event into the event queue; this is expected
	// to be called in overridden wndprocs connected to the root panel.
	virtual void PostUserEvent(const InputEvent_t &event) = 0;

	// Returns the number of joysticks
	virtual int GetJoystickCount() const = 0;

	// Enable/disable joystick, it has perf costs
	virtual void EnableJoystickInput(int nJoystick, bool bEnable) = 0;

	// Enable/disable diagonal joystick POV (simultaneous POV buttons down)
	virtual void EnableJoystickDiagonalPOV(int nJoystick, bool bEnable) = 0;

	// Sample the joystick and append events to the input queue
	virtual void SampleDevices(void) = 0;

	// FIXME: Currently force-feedback is only supported on the Xbox 360
	virtual void SetRumble(float fLeftMotor, float fRightMotor, int userId = -1) = 0;
	virtual void StopRumble(void) = 0;

	// Resets the input state
	virtual void ResetInputState() = 0;

	// Sets a player as the primary user - all other controllers will be ignored.
	virtual void SetPrimaryUserId(int userId) = 0;

	// Convert back + forth between ButtonCode/AnalogCode + strings
	virtual const char *ButtonCodeToString(ButtonCode_t code) const = 0;
	virtual const char *AnalogCodeToString(AnalogCode_t code) const = 0;
	virtual ButtonCode_t StringToButtonCode(const char *pString) const = 0;
	virtual AnalogCode_t StringToAnalogCode(const char *pString) const = 0;

	// Sleeps until input happens. Pass a negative number to sleep infinitely
	virtual void SleepUntilInput(int nMaxSleepTimeMS = -1) = 0;

	// Convert back + forth between virtual codes + button codes
	// FIXME: This is a temporary piece of code
	virtual ButtonCode_t VirtualKeyToButtonCode(int nVirtualKey) const = 0;
	virtual int ButtonCodeToVirtualKey(ButtonCode_t code) const = 0;
	virtual ButtonCode_t ScanCodeToButtonCode(int lParam) const = 0;

	// How many times have we called PollInputState?
	virtual int GetPollCount() const = 0;

	// Sets the cursor position
	virtual void SetCursorPosition(int x, int y) = 0;

	// NVNT get address to haptics interface
	virtual void *GetHapticsInterfaceAddress() const = 0;

	virtual void SetNovintPure(bool bPure) = 0;

	// read and clear accumulated raw input values
	virtual bool GetRawMouseAccumulators(int& accumX, int& accumY) = 0;

	// tell the input system that we're not a game, we're console text mode.
	// this is used for dedicated servers to not initialize joystick system.
	// this needs to be called before CInputSystem::Init (e.g. in PreInit of
	// some system) if you want ot prevent the joystick system from ever
	// being initialized.
	virtual void SetConsoleTextMode(bool bConsoleTextMode) = 0;
};

struct CameraThirdData_t
{
	float	m_flPitch;
	float	m_flYaw;
	float	m_flDist;
	float	m_flLag;
	Vector	m_vecHullMin;
	Vector	m_vecHullMax;
};

struct kbutton_t
{
	// key nums holding it down
	int		down[2];
	// low bit is down state
	int		state;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CKeyboardKey
{
public:
	// Name for key
	char				name[32];
	// Pointer to the underlying structure
	kbutton_t			*pkey;
	// Next key in key list.
	CKeyboardKey		*next;
};

// How many data slots to use when in multiplayer.
#define MULTIPLAYER_BACKUP			150

class IInput
{
public:
	// Initialization/shutdown of the subsystem
	virtual	void		Init_All(void) = 0;
	virtual void		Shutdown_All(void) = 0;
	// Latching button states
	virtual int			GetButtonBits(int) = 0;
	// Create movement command
	virtual void		CreateMove(int sequence_number, float input_sample_frametime, bool active) = 0;
	virtual void		ExtraMouseSample(float frametime, bool active) = 0;
	virtual bool		WriteUsercmdDeltaToBuffer(bf_write *buf, int from, int to, bool isnewcommand) = 0;
	virtual void		EncodeUserCmdToBuffer(bf_write& buf, int slot) = 0;
	virtual void		DecodeUserCmdFromBuffer(bf_read& buf, int slot) = 0;

	// 如果 solt 不知道就给个 0
	virtual CUserCmd	*GetUserCmd(int solt, int sequence_number) = 0;

	virtual void		MakeWeaponSelection(CBaseEntity* *weapon) = 0;

	// Retrieve key state
	virtual float		KeyState(kbutton_t *key) = 0;
	// Issue key event
	virtual int			KeyEvent(int eventcode, ButtonCode_t keynum, const char* pszCurrentBinding) = 0;

	// Look for key
	virtual kbutton_t	*FindKey(const char *name) = 0;

	// Issue commands from controllers
	virtual void		ControllerCommands(void) = 0;
	// Extra initialization for some joysticks
	virtual void		Joystick_Advanced(void) = 0;

	/*
	virtual void		Joystick_SetSampleTime(float frametime) = 0;
	virtual void		IN_SetSampleTime(float frametime) = 0;
	*/

	virtual void		Unknown_1() = 0;
	virtual void		Unknown_2() = 0;
	virtual void		Unknown_3() = 0;
	virtual void		Unknown_4() = 0;

	// Accumulate mouse delta
	virtual void		AccumulateMouse(void) = 0;
	// Activate/deactivate mouse
	virtual void		ActivateMouse(void) = 0;
	virtual void		DeactivateMouse(void) = 0;

	// Clear mouse state data
	virtual void		ClearStates(void) = 0;
	// Retrieve lookspring setting
	virtual float		GetLookSpring(void) = 0;

	// Retrieve mouse position
	virtual void		GetFullscreenMousePos(int *mx, int *my, int *unclampedx = 0, int *unclampedy = 0) = 0;
	virtual void		SetFullscreenMousePos(int mx, int my) = 0;
	virtual void		ResetMouse(void) = 0;
	virtual	float		GetLastForwardMove(void) = 0;
	// virtual	float		Joystick_GetForward(void) = 0;
	// virtual	float		Joystick_GetSide(void) = 0;
	// virtual	float		Joystick_GetPitch(void) = 0;
	// virtual	float		Joystick_GetYaw(void) = 0;

	// Third Person camera ( TODO/FIXME:  Move this to a separate interface? )
	virtual void		CAM_Think(void) = 0;
	virtual int			CAM_IsThirdPerson(void) = 0;
	virtual void		CAM_ToThirdPerson(void) = 0;
	virtual void		CAM_ToFirstPerson(void) = 0;
	virtual void		CAM_StartMouseMove(void) = 0;
	virtual void		CAM_EndMouseMove(void) = 0;
	virtual void		CAM_StartDistance(void) = 0;
	virtual void		CAM_EndDistance(void) = 0;
	virtual int			CAM_InterceptingMouse(void) = 0;

	// orthographic camera info	( TODO/FIXME:  Move this to a separate interface? )
	virtual void		CAM_ToOrthographic() = 0;
	virtual	bool		CAM_IsOrthographic() const = 0;
	virtual	void		CAM_OrthographicSize(float& w, float& h) const = 0;

	virtual void		LevelInit(void) = 0;

	// Causes an input to have to be re-pressed to become active
	virtual void		ClearInputButton(int bits) = 0;

	virtual	void		CAM_SetCameraThirdData(CameraThirdData_t *pCameraData, const QAngle &vecCameraOffset) = 0;
	virtual void		CAM_CameraThirdThink(void) = 0;

	virtual	bool		EnableJoystickMode() = 0;

public:
	typedef struct
	{
		unsigned int AxisFlags;
		unsigned int AxisMap;
		unsigned int ControlMap;
	} joy_axis_t;

	void		DescribeJoystickAxis(char const *axis, joy_axis_t *mapping);
	char const	*DescribeAxis(int index);

	enum
	{
		GAME_AXIS_NONE = 0,
		GAME_AXIS_FORWARD,
		GAME_AXIS_PITCH,
		GAME_AXIS_SIDE,
		GAME_AXIS_YAW,
		MAX_GAME_AXES
	};

	enum
	{
		CAM_COMMAND_NONE = 0,
		CAM_COMMAND_TOTHIRDPERSON = 1,
		CAM_COMMAND_TOFIRSTPERSON = 2
	};

	enum
	{
		MOUSE_ACCEL_THRESHHOLD1 = 0,	// if mouse moves > this many mickey's double it
		MOUSE_ACCEL_THRESHHOLD2,		// if mouse moves > this many mickey's double it a second time
		MOUSE_SPEED_FACTOR,				// 0 = disabled, 1 = threshold 1 enabled, 2 = threshold 2 enabled

		NUM_MOUSE_PARAMS,
	};

	// Has the mouse been initialized?
	bool		m_fMouseInitialized;
	// Is the mosue active?
	bool		m_fMouseActive;
	// Has the joystick advanced initialization been run?
	bool		m_fJoystickAdvancedInit;
	// Used to support hotplugging by reinitializing the advanced joystick system when we toggle between some/none joysticks.
	bool		m_fHadJoysticks;

	// Accumulated mouse deltas
	float		m_flAccumulatedMouseXMovement;
	float		m_flAccumulatedMouseYMovement;
	float		m_flPreviousMouseXPosition;
	float		m_flPreviousMouseYPosition;
	float		m_flRemainingJoystickSampleTime;
	float		m_flKeyboardSampleTime;

	// Flag to restore systemparameters when exiting
	bool		m_fRestoreSPI;
	// Original mouse parameters
	int			m_rgOrigMouseParms[NUM_MOUSE_PARAMS];
	// Current mouse parameters.
	int			m_rgNewMouseParms[NUM_MOUSE_PARAMS];
	bool		m_rgCheckMouseParam[NUM_MOUSE_PARAMS];
	// Are the parameters valid
	bool		m_fMouseParmsValid;
	// Joystick Axis data
	joy_axis_t	m_rgAxes[MAX_JOYSTICK_AXES];
	// List of queryable keys
	CKeyboardKey *m_pKeys;

	// Is the 3rd person camera using the mouse?
	bool		m_fCameraInterceptingMouse;
	// Are we in 3rd person view?
	bool		m_fCameraInThirdPerson;
	// Should we move view along with mouse?
	bool		m_fCameraMovingWithMouse;


	// Is the camera in distance moving mode?
	bool		m_fCameraDistanceMove;
	// Old and current mouse position readings.
	int			m_nCameraOldX;
	int			m_nCameraOldY;
	int			m_nCameraX;
	int			m_nCameraY;

	// orthographic camera settings
	bool		m_CameraIsOrthographic;

	QAngle		m_angPreviousViewAngles;

	float		m_flLastForwardMove;

	float m_flPreviousJoystickForward;
	float m_flPreviousJoystickSide;
	float m_flPreviousJoystickPitch;
	float m_flPreviousJoystickYaw;

	CUserCmd	*m_pCommands;
	CVerifiedUserCmd *m_pVerifiedCommands;

	CameraThirdData_t	*m_pCameraThirdData;

	// Set until polled by CreateMove and cleared
	CHandle<CBaseEntity> m_hSelectedWeapon;

#if defined( HL2_CLIENT_DLL )
	CUtlVector< CEntityGroundContact > m_EntityGroundContact;
#endif
};
