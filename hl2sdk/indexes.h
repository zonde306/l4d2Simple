#pragma once

namespace indexes
{
	// Client
	constexpr int GetAllClasses = 7;
	constexpr int CreateMove = 20;
	constexpr int FrameStageNotify = 34;
	constexpr int DispatchUserMessage = 35;
	constexpr int In_KeyEvent = 19;
	constexpr int HudProcessInput = 9;
	constexpr int HudUpdate = 10;
	constexpr int IN_IsKeyDown = 18;
	constexpr int WriteUsercmdDeltaToBuffer = 22;
	constexpr int RenderView = 26;
	constexpr int bSendPacket = 0x1D;
	constexpr int LevelInitPostEntity = 5;
	constexpr int LevelShutdown = 6;

	// Engine
	constexpr int GetScreenSize = 5;
	constexpr int GetPlayerInfo = 8;
	constexpr int GetLocalPlayer = 12;
	constexpr int ClientCmd = 107;
	constexpr int SetViewAngles = 20;
	constexpr int GetViewAngles = 19;
	constexpr int WorldToScreenMatrix = 37;
	constexpr int GetPlayerForUserId = 9;
	constexpr int Con_IsVisible = 11;
	constexpr int Time = 14;
	constexpr int GetMaxClients = 21;
	constexpr int IsInGame = 26;
	constexpr int IsConnected = 27;
	constexpr int IsDrawingLoadingImage = 28;
	constexpr int IsTakingScreenShot = 88;
	constexpr int GetNetChannelInfo = 74;
	constexpr int ClientCmdUnrestricted = 108;

	// EngineVGui
	constexpr int EnginePaint = 14;
	constexpr int EngineKeyEvent = 10;

	// ClientEntList
	constexpr int GetClientEntity = 3;
	constexpr int GetClientEntityFromHandle = 4;
	constexpr int GetHighestEntityIndex = 8;

	// Surface
	constexpr int DrawSetColor = 11;
	constexpr int DrawFilledRect = 12;
	constexpr int DrawFilledRectArray = 13;
	constexpr int DrawOutlinedRect = 14;
	constexpr int DrawLine = 15;
	constexpr int DrawPolyLine = 16;
	constexpr int DrawSetTextFont = 17;
	constexpr int DrawSetTextColor_Color = 18;
	constexpr int DrawSetTextColor = 19;
	constexpr int DrawSetTextPos = 20;
	constexpr int DrawGetTextPos = 21;
	constexpr int DrawPrintText = 22;
	constexpr int DrawUnicodeChar = 23;
	constexpr int DrawFlushText = 24;
	constexpr int SetCursor = 48;
	constexpr int SetCursorAlwaysVisible = 49;
	constexpr int IsCursorVisible = 50;
	constexpr int UnlockCursor = 58;
	constexpr int LockCursor = 59;
	constexpr int SCreateFont = 63;
	constexpr int SetFontGlyphSet = 64;
	constexpr int DrawColoredCircle = 152;
	constexpr int GetTextSize = 72;
	constexpr int DrawFilledRectFade = 115;
	constexpr int AddCustomFontFile = 65;
	constexpr int SurfacePaintTraverse = 85;
	constexpr int SurfacePaintTraverseEx = 111;

	// ModelInfo
	constexpr int GetStudioModel = 30;
	constexpr int GetModelName = 3;

	// ModelRender
	constexpr int DrawModel = 0;
	constexpr int ForcedMaterialOverride = 1;
	constexpr int DrawModelEx = 16;
	constexpr int DrawModelSetup = 18;
	constexpr int DrawModelExecute = 19;

	// Panel
	constexpr int GetName = 36;
	constexpr int PaintTraverse = 41;

	// MoveHelper
	constexpr int SetHost = 1;

	// Trace
	constexpr int TraceRay = 5;

	// Input
	constexpr int GetUserCmd = 8;
	constexpr int CAM_IsThirdPerson = 29;
	constexpr int CAM_ToThirdPerson = 31;
	constexpr int CAM_ToFirstPerson = 32;
	constexpr int InputWriteUsercmdDeltaToBuffer = 5;

	// Prediction
	constexpr int RunCommand = 18;
	constexpr int SetupMove = 19;
	constexpr int FinishMove = 20;
	constexpr int InPrediction = 14;

	// GameMovement
	constexpr int ProccessMovement = 1;
	constexpr int PlayerMove = 18;
	constexpr int PlaySwimSound = 62;

	// Entity
	constexpr int GetClientClass = 1;
	constexpr int GetAbsOrigin = 11;
	constexpr int GetAbsAngles = 12;
	constexpr int EntIndex = 8;
	constexpr int SetupBones = 13;
	constexpr int IsDormant = 7;
	constexpr int GetModel = 8;
	constexpr int GetWeaponID = 383;
	constexpr int RenderTableDrawModel = 9;
	constexpr int IsGhost = 240;
	constexpr int IsBot = 268;		// 这个函数没有实现(空函数)
	constexpr int ViewPunch = 304;
	constexpr int InSameTeam = 82;
	constexpr int GetRenderTeamNumber = 81;
	constexpr int GetTeamNumber = 79;
	constexpr int GetTeam = 78;
	constexpr int CanAttack = 352;
	constexpr int OnSpawn = 355;
	constexpr int IsReadyToShove = 356;
	constexpr int CanPlayerJump = 358;
	constexpr int CanBeShoved = 359;
	constexpr int IsPlayer = 143;
	constexpr int IsBaseCombatCharacter = 144;
	constexpr int IsNPC = 148;		// 这个永远返回 0
	constexpr int IsNextBot = 149;	// Infected 和 Witch 会返回 1 的
	constexpr int IsBaseCombatWeapon = 151;
	constexpr int EyePosition = 156;
	constexpr int EyeAngles = 157;
	constexpr int IsAbilityReadyToFire = 172;
	constexpr int GetCollideable = 3;
	constexpr int GetSolidFlags = 12;
	constexpr int GetRenderBounds = 14;
	constexpr int SetAbsOrigin = 53;
	constexpr int SetAbsAngles = 54;
	constexpr int HasAmmo = 13;
	constexpr int GetCSWeaponData = 98;
	constexpr int GetPrintName = 113;
	constexpr int MoveType = 0x144; // 在 C_BaseEntity::SetMoveType 里，this 后的第一个参数 8A 8E ? ? ? ? 80 F9 08

	// Entity Offset
	constexpr int GetSpread = 0x0D0C;		// 搜索 (%.1f) spread %.1f ( avg %.1f ) ( max %.1f )，最末尾那个 this 赋值
	constexpr int GetPunch = 0x1230;		// 在 C_BasePlayer::ViewPunch 里，float 最小的那个偏移量
	constexpr int GetPunchAngle = 0x1224;	// 搜索 HorizKickDir，末尾的 call，第一个 this 取值
	constexpr int GetCrosshairsId = 0x19D8;
	constexpr int GetLastCrosshairsId = 0x19E8;
	constexpr int GetWeaponId = 0x958;

	// ClientModeShared
	constexpr int SharedCreateMove = 27;
	constexpr int GetMessagePanel = 24;
	constexpr int KeyInput = 21;
	constexpr int OverrideView = 19;
	constexpr int GetViewModelFOV = 40;

	// CBaseHudChat
	constexpr int Printf = 22;
	constexpr int ChatPrintf = 23;

	// IViewRender
	constexpr int VGui_Paint = 39;
	constexpr int VguiPaint = 24;
	constexpr int Draw3DDebugOverlays = 3;
	constexpr int SetBlend = 4;
	constexpr int GetBlend = 5;
	constexpr int SetColorModulation = 6;
	constexpr int GetColorModulation = 7;
	constexpr int SceneBegin = 8;
	constexpr int SceneEnd = 9;

	// IMaterialSystem
	constexpr int FindMaterial = 71;
	constexpr int IsMaterialLoaded = 72;
	constexpr int FindTexture = 77;
	constexpr int OverrideConfig = 16;

	// IMaterial
	constexpr int SetMaterialVarFlag = 29;
	constexpr int ColorModuleate = 28;

	// CBaseClientState
	constexpr int ProcessStringCmd = 2;
	constexpr int ProcessSetConVar = 3;
	constexpr int ProcessSendTable = 8;
	constexpr int ProcessClassInfo = 9;
	constexpr int ProcessCreateString = 11;
	constexpr int ProcessCreateStringTable = 11;
	constexpr int ProcessUpdate = 12;
	constexpr int ProcessGetCvarValue = 28;
	constexpr int ConnectionClosing = 2;
	constexpr int ConnectionCrashed = 3;
	constexpr int FileRequested = 6;
	constexpr int FileReceived = 7;
	constexpr int FileDenied = 8;
	constexpr int RunFrame = 19;
	constexpr int FullConnect = 12;
	constexpr int Connect = 14;
	constexpr int Disconnect = 16;

	// INetChannelInfo/CNetChan
	constexpr int SendDatagram = 188;
	constexpr int SendNetMsg = 41;

	// RenderView
	constexpr int RenderViewSetColorModulation = 1;

	// IGameEvent
	constexpr int FireEvent = 7;
	constexpr int FireEventClientSide = 8;

	// EngineSound
	constexpr int EmitSound = 5;
};
