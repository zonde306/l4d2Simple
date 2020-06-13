#pragma once

namespace indexes
{
	// Client
	const int GetAllClasses = 7;
	const int CreateMove = 20;
	const int FrameStageNotify = 34;
	const int DispatchUserMessage = 35;
	const int In_KeyEvent = 19;
	const int HudProcessInput = 9;
	const int HudUpdate = 10;
	const int IN_IsKeyDown = 18;
	const int WriteUsercmdDeltaToBuffer = 22;
	const int RenderView = 26;

	// Engine
	const int GetScreenSize = 5;
	const int GetPlayerInfo = 8;
	const int GetLocalPlayer = 12;
	const int ClientCmd = 107;
	const int SetViewAngles = 20;
	const int GetViewAngles = 19;
	const int WorldToScreenMatrix = 37;
	const int GetPlayerForUserId = 9;
	const int Con_IsVisible = 11;
	const int Time = 14;
	const int GetMaxClients = 21;
	const int IsInGame = 26;
	const int IsConnected = 27;
	const int IsDrawingLoadingImage = 28;
	const int IsTakingScreenShot = 88;
	const int GetNetChannelInfo = 74;

	// EngineVGui
	const int EnginePaint = 14;
	const int EngineKeyEvent = 10;

	// ClientEntList
	const int GetClientEntity = 3;
	const int GetClientEntityFromHandle = 4;
	const int GetHighestEntityIndex = 8;

	// Surface
	const int DrawSetColor = 11;
	const int DrawFilledRect = 12;
	const int DrawFilledRectArray = 13;
	const int DrawOutlinedRect = 14;
	const int DrawLine = 15;
	const int DrawPolyLine = 16;
	const int DrawSetTextFont = 17;
	const int DrawSetTextColor_Color = 18;
	const int DrawSetTextColor = 19;
	const int DrawSetTextPos = 20;
	const int DrawGetTextPos = 21;
	const int DrawPrintText = 22;
	const int DrawUnicodeChar = 23;
	const int DrawFlushText = 24;
	const int SetCursor = 48;
	const int SetCursorAlwaysVisible = 49;
	const int IsCursorVisible = 50;
	const int UnlockCursor = 58;
	const int LockCursor = 59;
	const int SCreateFont = 63;
	const int SetFontGlyphSet = 64;
	const int DrawColoredCircle = 152;
	const int GetTextSize = 72;
	const int DrawFilledRectFade = 115;
	const int AddCustomFontFile = 65;
	const int SurfacePaintTraverse = 85;
	const int SurfacePaintTraverseEx = 111;

	// ModelInfo
	const int GetStudioModel = 30;
	const int GetModelName = 3;

	// ModelRender
	const int DrawModel = 0;
	const int ForcedMaterialOverride = 1;
	const int DrawModelEx = 16;
	const int DrawModelSetup = 18;
	const int DrawModelExecute = 19;

	// Panel
	const int GetName = 36;
	const int PaintTraverse = 41;

	// MoveHelper
	const int SetHost = 1;

	// Trace
	const int TraceRay = 5;

	// Input
	const int GetUserCmd = 8;
	const int CAM_IsThirdPerson = 29;
	const int CAM_ToThirdPerson = 31;
	const int CAM_ToFirstPerson = 32;
	const int InputWriteUsercmdDeltaToBuffer = 5;

	// Prediction
	const int RunCommand = 18;
	const int SetupMove = 19;
	const int FinishMove = 20;
	const int InPrediction = 14;

	// GameMovement
	const int ProccessMovement = 1;
	const int PlayerMove = 18;
	const int PlaySwimSound = 62;

	// Entity
	const int GetClientClass = 1;
	const int GetAbsOrigin = 11;
	const int GetAbsAngles = 12;
	const int EntIndex = 8;
	const int SetupBones = 13;
	const int IsDormant = 7;
	const int GetModel = 8;
	const int GetWeaponID = 383;
	const int RenderTableDrawModel = 9;
	const int IsGhost = 240;
	const int IsBot = 268;		// 这个函数没有实现(空函数)
	const int ViewPunch = 304;
	const int InSameTeam = 82;
	const int GetRenderTeamNumber = 81;
	const int GetTeamNumber = 79;
	const int GetTeam = 78;
	const int CanAttack = 352;
	const int OnSpawn = 355;
	const int IsReadyToShove = 356;
	const int CanPlayerJump = 358;
	const int CanBeShoved = 359;
	const int IsPlayer = 143;
	const int IsBaseCombatCharacter = 144;
	const int IsNPC = 148;		// 这个永远返回 0
	const int IsNextBot = 149;	// Infected 和 Witch 会返回 1 的
	const int IsBaseCombatWeapon = 151;
	const int EyePosition = 156;
	const int EyeAngles = 157;
	const int IsAbilityReadyToFire = 172;

	// Entity Offset
	const int GetSpread = 0xD0C;
	const int GetPunch = 0x1224;
	const int GetCrosshairsId = 0x19D8;
	const int GetLastCrosshairsId = 0x19E8;
	const int GetWeaponId = 0x958;

	// ClientModeShared
	const int SharedCreateMove = 27;
	const int GetMessagePanel = 24;
	const int KeyInput = 21;

	// CBaseHudChat
	const int Printf = 22;
	const int ChatPrintf = 23;

	// IViewRender
	const int VGui_Paint = 39;
	const int VguiPaint = 24;
	const int Draw3DDebugOverlays = 3;
	const int SetBlend = 4;
	const int GetBlend = 5;
	const int SetColorModulation = 6;
	const int GetColorModulation = 7;
	const int SceneBegin = 8;
	const int SceneEnd = 9;

	// IMaterialSystem
	const int FindMaterial = 71;
	const int IsMaterialLoaded = 72;
	const int FindTexture = 77;
	const int OverrideConfig = 16;

	// IMaterial
	const int SetMaterialVarFlag = 29;
	const int ColorModuleate = 28;

	// CBaseClientState
	const int ProcessStringCmd = 2;
	const int ProcessSetConVar = 3;
	const int ProcessSendTable = 8;
	const int ProcessClassInfo = 9;
	const int ProcessCreateString = 11;
	const int ProcessCreateStringTable = 11;
	const int ProcessUpdate = 12;
	const int ProcessGetCvarValue = 28;
	const int ConnectionClosing = 2;
	const int ConnectionCrashed = 3;
	const int FileRequested = 6;
	const int FileReceived = 7;
	const int FileDenied = 8;
	const int RunFrame = 19;
	const int FullConnect = 12;
	const int Connect = 14;
	const int Disconnect = 16;

	// INetChannelInfo/CNetChan
	const int SendDatagram = 188;
	const int SendNetMsg = 41;

	// RenderView
	const int RenderViewSetColorModulation = 1;

	// IGameEvent
	const int FireEvent = 7;
	const int FireEventClientSide = 8;

	// EngineSound
	const int EmitSound = 5;
};
