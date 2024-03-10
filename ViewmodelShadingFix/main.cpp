#include "nvse/PluginAPI.h"
#include "SafeWrite.h"
#include "GameData.hpp"

static std::map<NiAVObject*, NiPoint3> kLightPosMap;
static NiPoint3 kLightOffset;

template<typename FUNC>
void ForEachLight(NiNode* apNode, FUNC&& arFunc) {
	if (!apNode || !IS_NODE(apNode))
		return;

	UInt32 uiChildCount = apNode->GetArrayCount();
	if (!uiChildCount)
		return;

	for (UInt32 i = 0; i < uiChildCount; i++) {
		NiAVObject* pChild = apNode->GetAt(i);
		if (!pChild)
			continue;

		if (IS_NODE(pChild))
			ForEachLight((NiNode*)pChild, arFunc);
		else if (pChild->IsKindOf(*(NiRTTI*)0x11F4A28))
			arFunc(pChild);
	}
}

static NiPoint3* __stdcall EyeOffsetFixHook(NiPoint3* pOut, NiPoint3* pV, void* pM) {
	// Apply lighting offset - that's the whole fix
	NiPoint3 kLightOffset = BSShaderManager::GetShadowSceneNode(0)->kLightingOffset;
	pV->x += kLightOffset.x;
	pV->y += kLightOffset.y;
	pV->z += kLightOffset.z;
	return StdCall<NiPoint3*>(0xEE6DF8, pOut, pV, pM);
}

static void OffsetLights(NiNode* apNode) {
	kLightOffset = BSShaderManager::GetShadowSceneNode(0)->kLightingOffset;

	ForEachLight(apNode, [](NiAVObject* pLight) {
		// Store the original position of the light
		NiPoint3 kLightPos = pLight->m_kWorld.m_Translate;
		kLightPosMap.emplace(pLight, kLightPos);

		// Apply negative offset to the light, so it will get cancelled out later on
		pLight->m_kWorld.m_Translate = kLightPos - kLightOffset;
		}
	);
}

static void RestoreLights(NiNode* apNode) {
	ForEachLight(apNode, [](NiAVObject* pLight) {
		NiPoint3 kPos = kLightPosMap.find(pLight)->second;

		// Restore the original position of the light
		pLight->m_kWorld.m_Translate = kPos;
		}
	);
	kLightPosMap.clear();
}

static NiNode* __cdecl OffsetPlayerLightPositionsHook() {
	OffsetLights(PlayerCharacter::GetSingleton()->spPlayerNode);
	return *(NiNode**)0x11DEB7C;
}

static void __fastcall RestorePlayerLightPositionsHook(void* apThis) {
	RestoreLights(PlayerCharacter::GetSingleton()->spPlayerNode);
	ThisStdCall(0x404EE0, apThis);
}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "ViewmodelShadingFix";
	info->version = 200;
	return true;
}

bool NVSEPlugin_Load(NVSEInterface* nvse) {
	if (!nvse->isEditor) {
		// Game does not apply the lighting offset to the eye position for ShadowLightShader (it does for Lighting30Shader)
		// This causes viewmodel to have totally incorrect specular lighting
        ReplaceCall(0xB79019, EyeOffsetFixHook);

		// Game applies the lighting offset to lights attached to player's node, which causes them to be offset from the player
		// Ironic, isn't it?
        ReplaceCall(0x87513F, OffsetPlayerLightPositionsHook);
        ReplaceCall(0x875BB8, RestorePlayerLightPositionsHook);

		// Fix PipBoy menu light being offset (we do that already)
		SafeWrite8(0x874F76, 0xEB);
	}

	return true;
}