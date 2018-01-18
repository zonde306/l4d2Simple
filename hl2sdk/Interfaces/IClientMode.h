#pragma once
#include "IVPanel.h"
#include "IInputSystem.h"

class KeyValues;
class CBaseEntity;
class CBasePlayer;
class CViewSetup;
class AnimationController;

enum HeadtrackMovementMode_t
{
	HMM_SHOOTFACE_MOVEFACE = 0,		// Shoot from your face, move along your face.
	HMM_SHOOTFACE_MOVETORSO,		// Shoot from your face, move the direction your torso is facing.
	HMM_SHOOTMOUSE_MOVEFACE,		// Shoot from the mouse cursor which moves within the HUD, move along your face.
	HMM_SHOOTBOUNDEDMOUSE_LOOKFACE_MOVEFACE,	// Shoot from the mouse cursor which moves, bounded within the HUD, move along your face.
	HMM_SHOOTBOUNDEDMOUSE_LOOKFACE_MOVEMOUSE,	// Shoot from the mouse cursor which moves, bounded within the HUD, move along your weapon (the "mouse")

	// The following are not intended to be user-selectable modes, they are used by e.g. followcam stuff.
	HMM_SHOOTMOVELOOKMOUSEFACE,		// Shoot & move & look along the mouse cursor (i.e. original unchanged gameplay), face just looks on top of that.
	HMM_SHOOTMOVEMOUSE_LOOKFACE,	// Shoot & move along the mouse cursor (i.e. original unchanged gameplay), face just looks.
	HMM_SHOOTMOVELOOKMOUSE,			// Shoot, move and look along the mouse cursor - HMD orientation is completely ignored!

	HMM_LAST,

	HMM_NOOVERRIDE = HMM_LAST		// Used as a retrun from ShouldOverrideHeadtrackControl(), not an actual mode.
};

class IClientMode
{
	// Misc.
public:

	virtual			~IClientMode() {}

	// Called before the HUD is initialized.
	virtual void	InitViewport() = 0;

	// One time init when .dll is first loaded.
	virtual void	Init() = 0;

	// Called when vgui is shutting down.
	virtual void	VGui_Shutdown() = 0;

	// One time call when dll is shutting down
	virtual void	Shutdown() = 0;

	// Called when switching from one IClientMode to another.
	// This can re-layout the view and such.
	// Note that Enable and Disable are called when the DLL initializes and shuts down.
	virtual void	Enable() = 0;

	// Called when it's about to go into another client mode.
	virtual void	Disable() = 0;

	// Called when initializing or when the view changes.
	// This should move the viewport into the correct position.
	virtual void	Layout() = 0;

	// Gets at the viewport, if there is one...
	virtual IVPanel *GetViewport() = 0;

	// Gets at the viewports vgui panel animation controller, if there is one...
	virtual AnimationController *GetViewportAnimationController() = 0;

	// called every time shared client dll/engine data gets changed,
	// and gives the cdll a chance to modify the data.
	virtual void	ProcessInput(bool bActive) = 0;

	// The mode can choose to draw/not draw entities.
	virtual bool	ShouldDrawDetailObjects() = 0;
	virtual bool	ShouldDrawEntity(CBaseEntity *pEnt) = 0;
	virtual bool	ShouldDrawLocalPlayer(CBasePlayer *pPlayer) = 0;
	virtual bool	ShouldDrawParticles() = 0;

	// The mode can choose to not draw fog
	virtual bool	ShouldDrawFog(void) = 0;

	virtual void	OverrideView(CViewSetup *pSetup) = 0;
	virtual int		KeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding) = 0;
	virtual void	StartMessageMode(int iMessageModeType) = 0;
	virtual IVPanel *GetMessagePanel() = 0;
	virtual void	OverrideMouseInput(float *x, float *y) = 0;
	virtual bool	CreateMove(float flInputSampleTime, CUserCmd *cmd) = 0;

	virtual void	LevelInit(const char *newmap) = 0;
	virtual void	LevelShutdown(void) = 0;

	// Certain modes hide the view model
	virtual bool	ShouldDrawViewModel(void) = 0;
	virtual bool	ShouldDrawCrosshair(void) = 0;

	// Let mode override viewport for engine
	virtual void	AdjustEngineViewport(int& x, int& y, int& width, int& height) = 0;

	// Called before rendering a view.
	virtual void	PreRender(CViewSetup *pSetup) = 0;

	// Called after everything is rendered.
	virtual void	PostRender(void) = 0;

	virtual void	PostRenderVGui() = 0;

	virtual void	ActivateInGameVGuiContext(IVPanel *pPanel) = 0;
	virtual void	DeactivateInGameVGuiContext() = 0;
	virtual float	GetViewModelFOV(void) = 0;

	virtual bool	CanRecordDemo(char *errorMsg, int length) const = 0;

	virtual void	ComputeVguiResConditions(KeyValues *pkvConditions) = 0;

	//=============================================================================
	// HPE_BEGIN:
	// [menglish] Save server information shown to the client in a persistent place
	//=============================================================================

	virtual wchar_t* GetServerName() = 0;
	virtual void SetServerName(wchar_t* name) = 0;
	virtual wchar_t* GetMapName() = 0;
	virtual void SetMapName(wchar_t* name) = 0;

	//=============================================================================
	// HPE_END
	//=============================================================================

	virtual bool	DoPostScreenSpaceEffects(const CViewSetup *pSetup) = 0;

	virtual void	DisplayReplayMessage(const char *pLocalizeName, float flDuration, bool bUrgent,
		const char *pSound, bool bDlg) = 0;

	// Updates.
public:

	// Called every frame.
	virtual void	Update() = 0;

	// Returns true if VR mode should black out everything around the UI
	virtual bool	ShouldBlackoutAroundHUD() = 0;

	// Returns true if VR mode should black out everything around the UI
	virtual HeadtrackMovementMode_t ShouldOverrideHeadtrackControl() = 0;

	virtual bool	IsInfoPanelAllowed() = 0;
	virtual void	InfoPanelDisplayed() = 0;
	virtual bool	IsHTMLInfoPanelAllowed() = 0;
};
