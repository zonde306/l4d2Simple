#include "Visual.h"
#include "../Utils/math.h"
#include "../hook.h"
#include "../../l4d2Simple2/config.h"
#include <sstream>

CVisualPlayer* g_pVisualPlayer = nullptr;

#ifdef _DEBUG
#define Assert_Entity(_e)		(_e == nullptr || !_e->IsAlive())
#else
#define Assert_Entity(_e)		0
#endif

#define IsSurvivor(_id)				(_id == ET_SURVIVORBOT || _id == ET_CTERRORPLAYER)
#define IsCommonInfected(_id)		(_id == ET_INFECTED || _id == ET_WITCH)
#define IsSpecialInfected(_id)		(_id == ET_BOOMER || _id == ET_HUNTER || _id == ET_SMOKER || _id == ET_SPITTER || _id == ET_JOCKEY || _id == ET_CHARGER || _id == ET_TANK)

CVisualPlayer::CVisualPlayer() : CBaseFeatures::CBaseFeatures()
{
}

CVisualPlayer::~CVisualPlayer()
{
	CBaseFeatures::~CBaseFeatures();
}

void CVisualPlayer::OnEnginePaint(PaintMode_t mode)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return;

	// 用于格式化字符串
	std::stringstream ss;
	ss.sync_with_stdio(false);
	ss.tie(nullptr);
	ss.setf(std::ios::fixed);
	ss.precision(0);

	std::string temp;
	int team = local->GetTeam(), totalSpectator = 0;
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();
	Vector myFootOrigin = local->GetAbsOrigin();

	/*
	if (m_hSurfaceFont == 0)
	{
		m_hSurfaceFont = g_pInterface->Surface->CreateFont();
		g_pInterface->Surface->SetFontGlyphSet(m_hSurfaceFont, XorStr("Tahoma"),
			g_pDrawing->m_iFontSize, FW_DONTCARE, 0, 0, 0x200);
	}
	*/

	for (int i = 1; i <= maxEntity; ++i)
	{
		CBasePlayer* entity = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(i));
		if (entity == local)
		{
			m_iLocalPlayer = i;
			continue;
		}
		
		if (entity == nullptr)
			continue;

		if (!entity->IsAlive())
		{
			if (m_bSpectator)
			{
				if (DrawSpectator(entity, local, i, totalSpectator))
					++totalSpectator;
			}

			continue;
		}

		Vector eye, foot, head;
		Vector eyeOrigin = entity->GetEyePosition();
		Vector footOrigin = entity->GetAbsOrigin();
		Vector headOrigin = entity->GetHeadOrigin();

		if (!math::WorldToScreenEx(footOrigin, foot) ||
			!math::WorldToScreenEx(eyeOrigin, eye) ||
			!math::WorldToScreenEx(headOrigin, head))
			continue;

		// 是否为队友
		bool friendly = (team == entity->GetTeam());
		int classId = entity->GetClassID();
		ss.str("");

		// 方框的大小
		float height = fabs(head.y - foot.y);
		float width = height * 0.65f;

		// 距离
		float dist = math::GetVectorDistance(myFootOrigin, footOrigin, true);

		/*
		// 生还者在倒地时是横着的
		// 所以方框需要横着计算大小
		if (IsSurvivor(classId) && entity->IsIncapacitated())
		{
			width = fabs(head.x - foot.x);
			height = width * 0.65f;
		}
		*/

		if (m_bBox)
			DrawBox(friendly, entity, Vector(head.x - width / 2, head.y), Vector(width, height));
		if (m_bBone)
			DrawBone(entity, friendly);
		if (m_bHeadBox)
			DrawHeadBox(entity, head, dist);

		if (IsSurvivor(classId) || IsSpecialInfected(classId))
		{
			if (m_bHealth)
				ss << DrawHealth(entity);
			if (m_bName)
				ss << DrawName(i);
			if (m_bCharacter)
				ss << DrawCharacter(entity);
		}

		if (!ss.str().empty())
			ss << "\n";

		if (IsSurvivor(classId))
		{
			if (m_bWeapon)
				ss << DrawWeapon(entity);
		}

		if (m_bDistance)
			ss << DrawDistance(entity, dist);

		if (ss.str().empty())
			continue;

		GetTextPosition(ss.str(), eye);
		g_pDrawing->DrawText(static_cast<int>(eye.x), static_cast<int>(eye.y),
			GetDrawColor(entity), true, ss.str().c_str());
	}

}

void CVisualPlayer::OnSceneEnd()
{
	if (!m_bChams)
		return;

	static IMaterial* chamsMaterial = nullptr;
	if (chamsMaterial == nullptr)
	{
		if (g_pClientHook->oFindMaterial != nullptr)
		{
			chamsMaterial = g_pClientHook->oFindMaterial(g_pInterface->MaterialSystem,
				XorStr("debug/debugambientcube"), XorStr("Model textures"), true, nullptr);
		}
		else
		{
			chamsMaterial = g_pInterface->MaterialSystem->FindMaterial(
				XorStr("debug/debugambientcube"), XorStr("Model textures"));
		}

		chamsMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
		chamsMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
	}

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || chamsMaterial == nullptr)
		return;

	int team = local->GetTeam();
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();

	for (int i = 1; i <= maxEntity; ++i)
	{
		CBasePlayer* entity = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(i));
		if (entity == nullptr || entity == local || !entity->IsAlive())
			continue;

		if(team == entity->GetTeam())
			chamsMaterial->ColorModulate(0.0f, 1.0f, 1.0f);
		else
			chamsMaterial->ColorModulate(1.0f, 0.0f, 0.0f);

		chamsMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
		g_pInterface->ModelRender->ForcedMaterialOverride(chamsMaterial);
		entity->DrawModel(STUDIO_RENDER);
		g_pInterface->ModelRender->ForcedMaterialOverride(nullptr);
	}
}

void CVisualPlayer::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("Visual Players")))
		return;

	ImGui::Checkbox(XorStr("Left alignment"), &m_bDrawToLeft);
	IMGUI_TIPS("玩家信息左对齐，不选则为右对齐。");

	ImGui::Checkbox(XorStr("Player Box"), &m_bBox);
	IMGUI_TIPS("玩家或普感方框。");

	ImGui::Checkbox(XorStr("Player Head"), &m_bHeadBox);
	IMGUI_TIPS("玩家或普感头部位置。");

	ImGui::Checkbox(XorStr("Player Skeleton"), &m_bBone);
	IMGUI_TIPS("玩家或普感绘制骨骼，目前有 bug 不能用。");

	ImGui::Checkbox(XorStr("Player Name"), &m_bName);
	IMGUI_TIPS("玩家显示名字。");

	ImGui::Checkbox(XorStr("Player Health"), &m_bHealth);
	IMGUI_TIPS("玩家显示血量。");

	ImGui::Checkbox(XorStr("Player Distance"), &m_bDistance);
	IMGUI_TIPS("玩家显示距离。");

	ImGui::Checkbox(XorStr("Player Weapon"), &m_bWeapon);
	IMGUI_TIPS("玩家显示武器和弹药。");

	ImGui::Checkbox(XorStr("Player Character"), &m_bCharacter);
	IMGUI_TIPS("玩家显示角色，例如 Nick, Bill, Smoker 之类的。");

	ImGui::Checkbox(XorStr("Player Chams"), &m_bChams);
	IMGUI_TIPS("玩家透视和上色。");

	ImGui::Checkbox(XorStr("My Spectator"), &m_bSpectator);
	IMGUI_TIPS("显示当前观察者。");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Player Barrel"), &m_bBarrel);
	IMGUI_TIPS("显示玩家瞄准的位置。");

	ImGui::SliderFloat(XorStr("Barrel Distance"), &m_fBarrelDistance, 100.0f, 3000.0f, XorStr("%.0f"));

	ImGui::TreePop();
}

void CVisualPlayer::OnConfigLoading(const config_type & data)
{
	const std::string mainKeys = XorStr("PlayerVisual");
	
	m_bDrawToLeft = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_left_alignment"), m_bDrawToLeft);
	m_bBox = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_box"), m_bBox);
	m_bHeadBox = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_head"), m_bHeadBox);
	m_bBone = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_bone"), m_bBone);
	m_bName = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_name"), m_bName);
	m_bHealth = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_health"), m_bHealth);
	m_bDistance = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_distance"), m_bDistance);
	m_bWeapon = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_weapon"), m_bWeapon);
	m_bCharacter = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_character"), m_bCharacter);
	m_bChams = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_chams"), m_bChams);
	m_bSpectator = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_spectator"), m_bSpectator);
	m_bBarrel = g_pConfig->GetBoolean(mainKeys, XorStr("playeresp_barrel"), m_bBarrel);
	m_fBarrelDistance = g_pConfig->GetFloat(mainKeys, XorStr("playeresp_barrel_distance"), m_fBarrelDistance);
}

void CVisualPlayer::OnConfigSave(config_type & data)
{
	const std::string mainKeys = XorStr("PlayerVisual");
	
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_left_alignment"), m_bDrawToLeft);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_box"), m_bBox);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_head"), m_bHeadBox);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_bone"), m_bBone);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_name"), m_bName);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_health"), m_bHealth);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_distance"), m_bDistance);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_weapon"), m_bWeapon);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_character"), m_bCharacter);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_chams"), m_bChams);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_spectator"), m_bSpectator);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_barrel"), m_bBarrel);
	g_pConfig->SetValue(mainKeys, XorStr("playeresp_barrel_distance"), m_fBarrelDistance);
}

void CVisualPlayer::OnFrameStageNotify(ClientFrameStage_t stage)
{
	if (!m_bBarrel || stage != FRAME_RENDER_START)
		return;

	const float duration = 0.01f;
	int maxEntity = g_pInterface->Engine->GetMaxClients();
	for (int i = 1; i <= maxEntity; ++i)
	{
		if (i == m_iLocalPlayer)
			continue;
		
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(i));
		if (player == nullptr || !player->IsAlive())
			continue;

		int team = player->GetTeam();
		Vector eyePosition = player->GetEyePosition();
		// Vector endPosition = GetSeePosition(player, eyePosition, player->GetEyeAngles());
		Vector endPosition = player->GetEyeAngles().Forward().Scale(m_fBarrelDistance) + eyePosition;

		if (team == 3)
			g_pInterface->DebugOverlay->AddLineOverlay(eyePosition, endPosition, 255, 0, 0, false, duration);
		else if (team == 2)
			g_pInterface->DebugOverlay->AddLineOverlay(eyePosition, endPosition, 0, 128, 255, false, duration);
		else
			g_pInterface->DebugOverlay->AddLineOverlay(eyePosition, endPosition, 255, 128, 0, false, duration);
	}
}

bool CVisualPlayer::HasTargetVisible(CBasePlayer * entity)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return false;

	Ray_t ray;
	CTraceFilter filter;
	ray.Init(local->GetEyePosition(), entity->GetHeadOrigin());
	filter.pSkip1 = local;

	trace_t trace;

	try
	{
		g_pInterface->Trace->TraceRay(ray, MASK_SHOT, &filter, &trace);
	}
	catch (...)
	{
		Utils::log(XorStr("CKnifeBot.HasEnemyVisible.TraceRay Error."));
		return false;
	}

	return (trace.m_pEnt == entity || trace.fraction > 0.97f);
}

void CVisualPlayer::DrawBox(bool friendly, CBasePlayer* entity, const Vector & head, const Vector & foot)
{
	g_pDrawing->DrawRect(static_cast<int>(head.x), static_cast<int>(head.y - 5),
		static_cast<int>(foot.x), static_cast<int>(foot.y + 5), GetDrawColor(entity));
}

void CVisualPlayer::DrawBone(CBasePlayer * entity, bool friendly)
{
	model_t* models = entity->GetModel();
	if (models == nullptr)
		return;

	studiohdr_t* hdr = g_pInterface->ModelInfo->GetStudiomodel(models);
	if (hdr == nullptr)
		return;

	static matrix3x4_t boneMatrix[128];
	if (!entity->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, g_pInterface->GlobalVars->curtime))
		return;

	Vector parent, child, screenParent, screenChild;
	D3DCOLOR color = CDrawing::WHITE;

	if (friendly)
	{
		// g_pInterface->Surface->DrawSetColor(0, 255, 255, 255);
		color = CDrawing::SKYBLUE;
	}
	else
	{
		// g_pInterface->Surface->DrawSetColor(255, 0, 0, 255);
		color = CDrawing::RED;
	}

	for (int i = 0; i < hdr->numbones; ++i)
	{
		mstudiobone_t* bone = hdr->GetBone(i);
		if (bone == nullptr || !(bone->flags & BONE_USED_BY_HITBOX) ||
			bone->parent < 0 || bone->parent >= hdr->numbones)
			continue;

		child = Vector(boneMatrix[i][0][3], boneMatrix[i][1][3], boneMatrix[i][2][3]);
		parent = Vector(boneMatrix[bone->parent][0][3], boneMatrix[bone->parent][1][3], boneMatrix[bone->parent][2][3]);
		if (!child.IsValid() || !parent.IsValid() ||
			!math::WorldToScreenEx(parent, screenParent) ||
			!math::WorldToScreenEx(child, screenChild))
			continue;

		g_pDrawing->DrawLine(static_cast<int>(screenParent.x), static_cast<int>(screenParent.y),
			static_cast<int>(screenChild.x), static_cast<int>(screenChild.y), color);
		// g_pInterface->Surface->DrawLine(screenParent.x, screenParent.y, screenChild.x, screenChild.y);
	}
}

void CVisualPlayer::DrawHeadBox(CBasePlayer* entity, const Vector & head, float distance)
{
	int classId = entity->GetClassID();
	D3DCOLOR color = CDrawing::WHITE;
	bool visible = HasTargetVisible(entity);

	if (IsSurvivor(classId))
	{
		color = CDrawing::SKYBLUE;
		// g_pInterface->Surface->DrawSetColor(0, 255, 255, 255);
	}
	else if (IsSpecialInfected(classId))
	{
		color = CDrawing::RED;
		// g_pInterface->Surface->DrawSetColor(255, 0, 0, 255);
	}
	else if (classId == ET_WITCH)
	{
		color = CDrawing::PINK;
		// g_pInterface->Surface->DrawSetColor(255, 128, 255, 255);
	}
	else if (classId == ET_INFECTED)
	{
		color = CDrawing::ORANGE;
		// g_pInterface->Surface->DrawSetColor(255, 128, 0, 255);
	}

	int boxSize = 5;
	if (distance > 1000.0f)
		boxSize = 3;

	if (visible)
	{
		g_pDrawing->DrawCircleFilled(static_cast<int>(head.x), static_cast<int>(head.y), boxSize, color, 8);
		// g_pInterface->Surface->DrawFilledRect(head.x, head.y, head.x + 3, head.y + 3);
	}
	else
	{
		g_pDrawing->DrawCircle(static_cast<int>(head.x), static_cast<int>(head.y), boxSize, color, 8);
		// g_pInterface->Surface->DrawOutlinedRect(head.x, head.y, head.x + 3, head.y + 3);
	}
}

int CVisualPlayer::GetTextMaxWide(const std::string & text)
{
	size_t width = 0;
	for (auto line : Utils::Split(text, "\n"))
	{
		if (line.length() > width)
			width = line.length();
	}

	return static_cast<int>(width);
}

typename CVisualPlayer::DrawPosition_t CVisualPlayer::GetTextPosition(const std::string & text, Vector & head)
{
	int width = 0, height = 0;
	g_pInterface->Engine->GetScreenSize(width, height);

	// int wide = GetTextMaxWide(text) * g_pDrawing->m_iFontSize;
	// int wide = 0, tall = 0;
	// g_pInterface->Surface->GetTextSize(m_hSurfaceFont, Utils::c2w(text).c_str(), wide, tall);
	auto fontSize = g_pDrawing->GetDrawTextSize(text.c_str());

	DrawPosition_t result = DP_Anywhere;

	if (m_bDrawToLeft)
	{
		if ((head.x - fontSize.first) < 0)
			result = DP_Right;
		else
			result = DP_Left;
	}
	else
	{
		if ((head.x + fontSize.first) > width)
			result = DP_Left;
		else
			result = DP_Right;
	}

	/*
	if ((head.y + fontSize.second) > height)
		result |= DP_Top;
	else
		result |= DP_Bottom;
	*/

	if (result & DP_Left)
		head.x -= fontSize.first;
	else if (result & DP_Right)
		head.x += fontSize.first / 2;

	if (result & DP_Top)
		head.y -= fontSize.second;
	else if (result & DP_Bottom)
		head.y += fontSize.second / 2;

	return result;
}

D3DCOLOR CVisualPlayer::GetDrawColor(CBasePlayer * entity)
{
	if (entity == nullptr || !entity->IsAlive())
		return CDrawing::WHITE;

	int classId = entity->GetClassID();
	if (IsSurvivor(classId))
	{
		if (entity->IsDying())
			return CDrawing::WHITE;
		if(entity->IsIncapacitated())
			return CDrawing::DEEPSKYBLUE;

		return CDrawing::SKYBLUE;
	}
	else if (classId == ET_TANK)
		return CDrawing::ORANGE;
	else if (IsSpecialInfected(classId))
	{
		if (entity->IsGhost())
			return CDrawing::PURPLE;

		return CDrawing::RED;
	}
	else if (classId == ET_WITCH)
	{
		if(entity->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage")) >= 1.0f)
			return CDrawing::ORANGE;

		return CDrawing::PINK;
	}
	else if (classId == ET_INFECTED)
		return CDrawing::YELLOW;

	return CDrawing::GRAY;
}

Vector CVisualPlayer::GetSeePosition(CBasePlayer * player, const Vector & eyePosition, const QAngle & eyeAngles)
{
	Ray_t ray;
	CTraceFilter filter;
	ray.Init(eyePosition, eyeAngles.Forward().Scale(1500.0f) + eyePosition);
	filter.pSkip1 = player;

	trace_t trace;

	try
	{
		g_pInterface->Trace->TraceRay(ray, MASK_SHOT, &filter, &trace);
	}
	catch (...)
	{
		Utils::log(XorStr("CKnifeBot.HasEnemyVisible.TraceRay Error."));
		return false;
	}

	return trace.end;
}

std::string CVisualPlayer::DrawName(int index, bool separator)
{
	player_info_t info;
	if (!g_pInterface->Engine->GetPlayerInfo(index, &info))
		return "";

	if (separator)
		return std::string("\n") + info.name;

	return info.name + std::string(" ");
}

std::string CVisualPlayer::DrawHealth(CBasePlayer * entity, bool separator)
{
	char buffer[32];

	if (entity->GetTeam() == 3)
		sprintf_s(buffer, "[%d] ", entity->GetHealth());
	else
		sprintf_s(buffer, "[%d] ", entity->GetHealth() + entity->GetTempHealth());
	
	if (separator)
		return std::string("\n") + buffer;

	return buffer;
}

std::string CVisualPlayer::DrawWeapon(CBasePlayer * entity, bool separator)
{
	if (entity->GetTeam() != 2)
		return "";

	CBaseWeapon* weapon = entity->GetActiveWeapon();
	if (weapon == nullptr)
		return "";

	char buffer[64];
	const char* weaponName = weapon->GetWeaponName();
	if (weapon->IsReloading())
		sprintf_s(buffer, "%s %s ", weaponName, XorStr("(reloading)"));
	else
		sprintf_s(buffer, "%s (%d/%d) ", weaponName, weapon->GetClip(), weapon->GetAmmo());

	if (separator)
		return std::string("\n") + buffer;

	return buffer;
}

std::string CVisualPlayer::DrawDistance(CBasePlayer * entity, float distance, bool separator)
{
	char buffer[16];
	_itoa_s(static_cast<int>(distance), buffer, 10);

	if (separator)
		return std::string("\n") + buffer;

	return buffer;
}

std::string CVisualPlayer::DrawCharacter(CBasePlayer * entity, bool separator)
{
	std::string buffer = entity->GetCharacterName();

	if (!buffer.empty())
	{
		if (separator)
			buffer = "\n" + buffer;
	}

	return " (" + buffer + ")";
}

bool CVisualPlayer::DrawSpectator(CBasePlayer * player, CBasePlayer* local, int index, int line)
{
	int classId = player->GetClassID();
	if (!IsSurvivor(classId) && !IsSpecialInfected(classId))
		return false;

	int obsMode = player->GetNetProp<byte>(XorStr("DT_BasePlayer"), XorStr("m_iObserverMode"));
	if (obsMode != OBS_MODE_IN_EYE && obsMode != OBS_MODE_CHASE)
		return false;

	CBaseHandle handle = player->GetNetProp<CBaseHandle>(XorStr("DT_BasePlayer"), XorStr("m_hObserverTarget"));
	if (!handle.IsValid())
		return false;

	CBasePlayer* target = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntityFromHandle(handle));
	if (target == nullptr || !target->IsAlive())
		return false;

	if (!local->IsAlive())
	{
		handle = local->GetNetProp<CBaseHandle>(XorStr("DT_BasePlayer"), XorStr("m_hObserverTarget"));
		if (!handle.IsValid())
			return false;

		local = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntityFromHandle(handle));
		if (local == nullptr || !local->IsAlive())
			return false;
	}

	if (target != local)
		return false;

	player_info_t info;
	if (!g_pInterface->Engine->GetPlayerInfo(index, &info))
		return false;

	D3DCOLOR color = CDrawing::WHITE;
	if (IsSurvivor(classId))
		color = CDrawing::SKYBLUE;
	else if (IsSpecialInfected(classId))
		color = CDrawing::RED;

	std::stringstream ss;
	ss.sync_with_stdio(false);
	ss.tie(nullptr);
	ss.setf(std::ios::fixed);
	ss.precision(0);

	ss << info.name;
	if (obsMode == OBS_MODE_CHASE)
		ss << XorStr(" [3rd]");
	else
		ss << XorStr(" [1st]");

	int width, height;
	g_pInterface->Engine->GetScreenSize(width, height);
	auto size = g_pDrawing->GetDrawTextSize(ss.str().c_str());
	g_pDrawing->DrawText(static_cast<int>(width * 0.75f),
		static_cast<int>((height * 0.75f) + (line * size.second)),
		color, false, ss.str().c_str());

	return true;
}
