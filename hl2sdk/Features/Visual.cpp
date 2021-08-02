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

	static bool nightVision = false;
	if (m_bNightVision != nightVision)
	{
		if (m_bNightVision)
			local->GetNetProp<BYTE>(XorStr("DT_TerrorPlayer"), XorStr("m_bNightVisionOn")) = 1;
		else
			local->GetNetProp<BYTE>(XorStr("DT_TerrorPlayer"), XorStr("m_bNightVisionOn")) = 0;
		nightVision = m_bNightVision;
	}

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

			if (m_bNoFog && entity->GetClassID() == ET_FogController)
				entity->GetNetProp<BYTE>(XorStr("DT_FogController"), XorStr("m_fog.enable")) = 0;

			continue;
		}

		Vector eye, foot, head;
		Vector eyeOrigin = entity->GetEyePosition();
		Vector footOrigin = entity->GetAbsOrigin();
		Vector headOrigin = entity->GetHeadOrigin();

		if (!eyeOrigin.IsValid())
			eyeOrigin = footOrigin;

		if (!math::WorldToScreenEx(footOrigin, foot) ||
			!math::WorldToScreenEx(eyeOrigin, eye) ||
			!math::WorldToScreenEx(headOrigin, head))
			continue;

		// 是否为队友
		int targetTeam = entity->GetTeam();
		bool friendly = ((team == 2 && targetTeam == 3) || (team == 3 && targetTeam == 2));
		int classId = entity->GetClassID();
		bool flat = (targetTeam == 2 && entity->IsIncapacitated());

		ss.str("");

		// 方框的大小
		float height = fabs(head.y - foot.y);
		float width = height * 0.65f;
		
		// 生还者在倒地时是横着的
		// 所以方框需要横着计算大小
		if (flat)
		{
			width = -(head.x - foot.x) * 2.0f;
			height = fabs(width) * 0.35f;
		}

		// 距离
		float dist = math::GetVectorDistance(myFootOrigin, footOrigin, true);

		if (m_bBox)
		{
			if(flat)
				DrawBox(friendly, entity, Vector(head.x, head.y), Vector(width, height));
			else
				DrawBox(friendly, entity, Vector(head.x - width / 2, head.y), Vector(width, height));
		}

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

		if (m_bFieldOfView)
			ss << DrawFOV(entity, local);

		if (m_bDebug)
			ss << "\n" << DrawDebugInfo(entity);

		if (ss.str().empty())
			continue;

		GetTextPosition(ss.str(), eye);
		g_pDrawing->DrawText(static_cast<int>(eye.x), static_cast<int>(eye.y),
			GetDrawColor(entity), true, ss.str().c_str());
	}
}

void CVisualPlayer::OnSceneEnd()
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsValid())
		return;
	
	if (m_bChams)
	{
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

		if (chamsMaterial)
		{
			int team = local->GetTeam();
			int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();

			for (int i = 1; i <= maxEntity; ++i)
			{
				CBasePlayer* entity = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(i));
				if (entity == nullptr || entity == local || !entity->IsAlive())
					continue;

				if (team == entity->GetTeam())
					chamsMaterial->ColorModulate(0.0f, 1.0f, 1.0f);
				else
					chamsMaterial->ColorModulate(1.0f, 0.0f, 0.0f);

				chamsMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
				g_pInterface->ModelRender->ForcedMaterialOverride(chamsMaterial);
				entity->DrawModel(STUDIO_RENDER);
				g_pInterface->ModelRender->ForcedMaterialOverride(nullptr);
			}
		}
	}

	if (m_bNoVomit)
	{
		static IMaterial* vomitMaterial = nullptr;
		if (vomitMaterial == nullptr)
		{
			if (g_pClientHook->oFindMaterial != nullptr)
			{
				vomitMaterial = g_pClientHook->oFindMaterial(g_pInterface->MaterialSystem,
					XorStr("particle/screenspaceboomervomit"), XorStr("Particle textures"),
					true, nullptr
				);
			}
			else
			{
				vomitMaterial = g_pInterface->MaterialSystem->FindMaterial(
					XorStr("particle/screenspaceboomervomit"), XorStr("Particle textures")
				);
			}

			vomitMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		}

		/*
		if (vomitMaterial)
		{
			g_pInterface->ModelRender->ForcedMaterialOverride(vomitMaterial);
			g_pInterface->ModelRender->ForcedMaterialOverride(nullptr);
		}
		*/
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
	
	ImGui::Checkbox(XorStr("Player In FOV"), &m_bFieldOfView);
	IMGUI_TIPS("玩家显示瞄准角度。");

	ImGui::Checkbox(XorStr("Player Weapon"), &m_bWeapon);
	IMGUI_TIPS("玩家显示武器和弹药。");

	ImGui::Checkbox(XorStr("Player Character"), &m_bCharacter);
	IMGUI_TIPS("玩家显示角色，例如 Nick, Bill, Smoker 之类的。");

	ImGui::Checkbox(XorStr("Player Chams"), &m_bChams);
	IMGUI_TIPS("玩家透视和上色。");

	ImGui::Checkbox(XorStr("My Spectator"), &m_bSpectator);
	IMGUI_TIPS("显示当前观察者。");
	
	ImGui::Checkbox(XorStr("Show Position"), &m_bDebug);
	IMGUI_TIPS("显示位置/角度/速度。");
	
	ImGui::Separator();
	ImGui::Checkbox(XorStr("Fov Changer"), &m_bFovChanger);
	IMGUI_TIPS("修改 FOV");
	
	ImGui::SliderFloat(XorStr("Fov"), &m_fFov, 90.0f, 150.0f, XorStr("%.0f"));
	IMGUI_TIPS("修改 FOV");
	
	ImGui::SliderFloat(XorStr("Viewmodel Fov"), &m_fViewFov, 10.0f, 130.0f, XorStr("%.0f"));
	IMGUI_TIPS("修改 Viewmodel FOV");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Player Barrel"), &m_bBarrel);
	IMGUI_TIPS("显示玩家瞄准的位置。");

	ImGui::SliderFloat(XorStr("Barrel Distance"), &m_fBarrelDistance, 100.0f, 3000.0f, XorStr("%.0f"));
	IMGUI_TIPS("显示玩家瞄准的位置线条长度。");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("No Vomit Effect"), &m_bNoVomit);
	IMGUI_TIPS("去除胆汁屏幕效果。");

	ImGui::Checkbox(XorStr("No Infected Vision"), &m_bCleanVision);
	IMGUI_TIPS("去除特感黄色屏幕。");

	ImGui::Checkbox(XorStr("No Ghost Vision"), &m_bCleanGhost);
	IMGUI_TIPS("去除灵魂特感蓝色屏幕。");

	ImGui::Checkbox(XorStr("No Fog"), &m_bNoFog);
	IMGUI_TIPS("去除灵魂特感蓝色屏幕。");
	
	ImGui::Checkbox(XorStr("No Mud"), &m_bNoMud);
	IMGUI_TIPS("去除泥浆屏幕效果。");
	
	ImGui::Checkbox(XorStr("No Blood"), &m_bNoBlood);
	IMGUI_TIPS("去除血腥屏幕效果。");
	
	ImGui::Checkbox(XorStr("No Explosion Flash"), &m_bNoFlash);
	IMGUI_TIPS("去除爆炸闪光屏幕效果。");
	
	ImGui::Checkbox(XorStr("Full Bright"), &m_bFullBright);
	IMGUI_TIPS("地图高亮。");
	
	ImGui::Checkbox(XorStr("Full Flashlight"), &m_bFullFlashlight);
	IMGUI_TIPS("手电全屏。");
	
	ImGui::Checkbox(XorStr("Night Vision"), &m_bNightVision);
	IMGUI_TIPS("夜视仪。");

	ImGui::TreePop();

	UpdateConVar();
}

void CVisualPlayer::OnConfigLoading(CProfile& cfg)
{
	const std::string mainKeys = XorStr("PlayerVisual");
	
	m_bDrawToLeft = cfg.GetBoolean(mainKeys, XorStr("playeresp_left_alignment"), m_bDrawToLeft);
	m_bBox = cfg.GetBoolean(mainKeys, XorStr("playeresp_box"), m_bBox);
	m_bHeadBox = cfg.GetBoolean(mainKeys, XorStr("playeresp_head"), m_bHeadBox);
	m_bBone = cfg.GetBoolean(mainKeys, XorStr("playeresp_bone"), m_bBone);
	m_bName = cfg.GetBoolean(mainKeys, XorStr("playeresp_name"), m_bName);
	m_bHealth = cfg.GetBoolean(mainKeys, XorStr("playeresp_health"), m_bHealth);
	m_bDistance = cfg.GetBoolean(mainKeys, XorStr("playeresp_distance"), m_bDistance);
	m_bWeapon = cfg.GetBoolean(mainKeys, XorStr("playeresp_weapon"), m_bWeapon);
	m_bCharacter = cfg.GetBoolean(mainKeys, XorStr("playeresp_character"), m_bCharacter);
	m_bChams = cfg.GetBoolean(mainKeys, XorStr("playeresp_chams"), m_bChams);
	m_bSpectator = cfg.GetBoolean(mainKeys, XorStr("playeresp_spectator"), m_bSpectator);
	m_bBarrel = cfg.GetBoolean(mainKeys, XorStr("playeresp_barrel"), m_bBarrel);
	m_fBarrelDistance = cfg.GetFloat(mainKeys, XorStr("playeresp_barrel_distance"), m_fBarrelDistance);
	m_bNoVomit = cfg.GetBoolean(mainKeys, XorStr("playeresp_no_vomit"), m_bNoVomit);
	m_bCleanVision = cfg.GetBoolean(mainKeys, XorStr("playeresp_no_vision"), m_bCleanVision);
	m_bCleanGhost = cfg.GetBoolean(mainKeys, XorStr("playeresp_no_ghost"), m_bCleanGhost);
	m_bNoFog = cfg.GetBoolean(mainKeys, XorStr("playeresp_no_fog"), m_bNoFog);
	m_fFov = cfg.GetFloat(mainKeys, XorStr("playeresp_fov"), m_fFov);
	m_fViewFov = cfg.GetFloat(mainKeys, XorStr("playeresp_vm_fov"), m_fViewFov);
	m_bFovChanger = cfg.GetBoolean(mainKeys, XorStr("playeresp_fov_changer"), m_bFovChanger);
	m_bFieldOfView = cfg.GetBoolean(mainKeys, XorStr("playeresp_aim_fov"), m_bFieldOfView);
	m_bNoMud = cfg.GetBoolean(mainKeys, XorStr("playeresp_no_mud"), m_bNoMud);
	m_bNoBlood = cfg.GetBoolean(mainKeys, XorStr("playeresp_no_blood"), m_bNoBlood);
	m_bFullBright = cfg.GetBoolean(mainKeys, XorStr("playeresp_fullbright"), m_bFullBright);
	m_bFullFlashlight = cfg.GetBoolean(mainKeys, XorStr("playeresp_fullflashlight"), m_bFullFlashlight);
	m_bNoFlash = cfg.GetBoolean(mainKeys, XorStr("playeresp_no_flash"), m_bNoFlash);
}

void CVisualPlayer::OnConfigSave(CProfile& cfg)
{
	const std::string mainKeys = XorStr("PlayerVisual");
	
	cfg.SetValue(mainKeys, XorStr("playeresp_left_alignment"), m_bDrawToLeft);
	cfg.SetValue(mainKeys, XorStr("playeresp_box"), m_bBox);
	cfg.SetValue(mainKeys, XorStr("playeresp_head"), m_bHeadBox);
	cfg.SetValue(mainKeys, XorStr("playeresp_bone"), m_bBone);
	cfg.SetValue(mainKeys, XorStr("playeresp_name"), m_bName);
	cfg.SetValue(mainKeys, XorStr("playeresp_health"), m_bHealth);
	cfg.SetValue(mainKeys, XorStr("playeresp_distance"), m_bDistance);
	cfg.SetValue(mainKeys, XorStr("playeresp_weapon"), m_bWeapon);
	cfg.SetValue(mainKeys, XorStr("playeresp_character"), m_bCharacter);
	cfg.SetValue(mainKeys, XorStr("playeresp_chams"), m_bChams);
	cfg.SetValue(mainKeys, XorStr("playeresp_spectator"), m_bSpectator);
	cfg.SetValue(mainKeys, XorStr("playeresp_barrel"), m_bBarrel);
	cfg.SetValue(mainKeys, XorStr("playeresp_barrel_distance"), m_fBarrelDistance);
	cfg.SetValue(mainKeys, XorStr("playeresp_no_vomit"), m_bNoVomit);
	cfg.SetValue(mainKeys, XorStr("playeresp_no_vision"), m_bCleanVision);
	cfg.SetValue(mainKeys, XorStr("playeresp_no_ghost"), m_bCleanGhost);
	cfg.SetValue(mainKeys, XorStr("playeresp_no_fog"), m_bNoFog);
	cfg.SetValue(mainKeys, XorStr("playeresp_fov"), m_fFov);
	cfg.SetValue(mainKeys, XorStr("playeresp_vm_fov"), m_fViewFov);
	cfg.SetValue(mainKeys, XorStr("playeresp_fov_changer"), m_bFovChanger);
	cfg.SetValue(mainKeys, XorStr("playeresp_aim_fov"), m_bFieldOfView);
	cfg.SetValue(mainKeys, XorStr("playeresp_no_mud"), m_bNoMud);
	cfg.SetValue(mainKeys, XorStr("playeresp_no_blood"), m_bNoBlood);
	cfg.SetValue(mainKeys, XorStr("playeresp_fullbright"), m_bFullBright);
	cfg.SetValue(mainKeys, XorStr("playeresp_fullflashlight"), m_bFullFlashlight);
	cfg.SetValue(mainKeys, XorStr("playeresp_no_flash"), m_bNoFlash);
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

bool CVisualPlayer::OnFindMaterial(std::string & materialName, std::string & textureGroupName)
{
	if ((m_bNoVomit && materialName.find(XorStr("vomitscreensplash")) != std::string::npos) ||
		(m_bCleanVision && materialName.find(XorStr("flashlight001_infected")) != std::string::npos) ||
		(m_bCleanGhost && materialName.find(XorStr("flashlight001_ghost")) != std::string::npos) ||
		(m_bNoMud && materialName.find(XorStr("droplets")) != std::string::npos) ||
		(m_bNoBlood && materialName.find(XorStr("bloodsplatter")) != std::string::npos) ||
		(m_bNoFlash && materialName.find(XorStr("particle_flare_001")) != std::string::npos) ||
		(m_bNoFlash && materialName.find(XorStr("particle_flare_004b_mod")) != std::string::npos))
	{
		// 替换成透明的材质
		materialName = XorStr("dev/clearalpha");
		textureGroupName.clear();
	}

	return true;
}

void CVisualPlayer::OnOverrideView(CViewSetup* setup)
{
	if (!m_bFovChanger)
		return;
	
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return;
	
	setup->m_fov = m_fFov;
	// setup->m_viewmodelfov = m_fViewFov;
}

bool CVisualPlayer::OnGetViewModelFOV(float& fov)
{
	if (!m_bFovChanger)
		return false;
	
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return false;
	
	fov = m_fViewFov;
	return true;
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
	// 普感满大街都是，画出来挡视线
	if (entity->GetZombieType() == ZC_COMMON)
		return;
	
	g_pDrawing->DrawRect(static_cast<int>(head.x), static_cast<int>(head.y - 5),
		static_cast<int>(foot.x), static_cast<int>(foot.y + 5), GetDrawColor(entity));
}

// FIXME: 目前这个有 bug 待修复
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
		mstudiobone_t* bone = hdr->pBone(i);

		// 在这里有部分骨头拿不到
		// 同样的代码，但是另一个项目却正常...
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
	for (auto line : Utils::StringSplit(text, "\n"))
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
	int team = entity->GetTeam();
	if (IsSpecialInfected(classId) || team == 3)
	{
		if (entity->IsGhost())
			return CDrawing::PURPLE;

		return CDrawing::RED;
	}
	else if (classId == ET_TANK/* && team == 3*/)
	{
		return CDrawing::ORANGE;
	}
	else if (IsSurvivor(classId) || team == 2)
	{
		if (entity->IsDying())
			return CDrawing::WHITE;
		if (entity->IsIncapacitated())
			return CDrawing::DEEPSKYBLUE;

		return CDrawing::SKYBLUE;
	}
	else if (classId == ET_WITCH)
	{
		if(entity->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage")) >= 1.0f)
			return CDrawing::ORANGE;

		return CDrawing::PINK;
	}
	else if (classId == ET_INFECTED)
	{
		return CDrawing::YELLOW;
	}

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
		return INVALID_VECTOR;
	}

	return trace.end;
}

void CVisualPlayer::UpdateConVar()
{
	static bool bright = false, flashlight = false;

	if (m_bFullBright != bright)
	{
		bright = m_bFullBright;
		static ConVar* mat_fullbright = g_pInterface->Cvar->FindVar(XorStr("mat_fullbright"));
		mat_fullbright->SetValue(bright);
	}
	if (m_bFullFlashlight != flashlight)
	{
		flashlight = m_bFullFlashlight;
		static ConVar* r_flashlightfov = g_pInterface->Cvar->FindVar(XorStr("r_flashlightfov"));
		static ConVar* r_flashlightconstant = g_pInterface->Cvar->FindVar(XorStr("r_flashlightconstant"));
		static ConVar* cl_max_shadow_renderable_dist = g_pInterface->Cvar->FindVar(XorStr("cl_max_shadow_renderable_dist"));

		if (flashlight)
		{
			r_flashlightfov->SetValue(120);
			r_flashlightconstant->SetValue(1);
			cl_max_shadow_renderable_dist->SetValue(0);
		}
		else
		{
			r_flashlightfov->SetValue(53);
			r_flashlightconstant->SetValue(0);
			cl_max_shadow_renderable_dist->SetValue(3000);
		}
	}
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

std::string CVisualPlayer::DrawDebugInfo(CBaseEntity* player)
{
	std::stringstream ss;
	ss.sync_with_stdio(false);
	ss.tie(nullptr);
	ss.setf(std::ios::fixed);
	ss.precision(0);

	Vector origin = player->GetAbsOrigin();
	ss << XorStr("pos(") << origin.x << ", " << origin.y << ", " << origin.z << ")\n";

	QAngle angles = player->GetEyeAngles();
	ss << XorStr("ang(") << angles.x << ", " << angles.y << ", " << angles.z << ")\n";

	if (player->IsPlayer())
	{
		Vector velocity = reinterpret_cast<CBasePlayer*>(player)->GetVelocity();
		ss << XorStr("vel(") << velocity.x << ", " << velocity.y << ", " << velocity.z << ") " << velocity.Length() << std::endl;
	}

	ss << XorStr("seq:") << player->GetSequence() << "\n";

	return ss.str();
}

std::string CVisualPlayer::DrawFOV(CBasePlayer* player, CBasePlayer* local)
{
	QAngle eyeAngles/* = local->GetEyeAngles()*/;
	g_pInterface->Engine->GetViewAngles(eyeAngles);
	Vector eyePosition = local->GetEyePosition();
	Vector aimPosition = player->GetHeadOrigin();

	std::stringstream ss;
	ss.sync_with_stdio(false);
	ss.tie(nullptr);
	ss.setf(std::ios::fixed);
	ss.precision(0);

	ss << "(" << math::GetAnglesFieldOfView(eyeAngles, math::CalculateAim(eyePosition, aimPosition)) << ")";
	return ss.str();
}
