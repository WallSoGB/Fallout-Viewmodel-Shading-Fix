#include "fose/fose/PluginAPI.h"
#include "nvse/PluginAPI.h"
#include "SafeWrite.h"
#include "GameData.hpp"

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

#ifdef FO3
static NiRTTI* NiLightRTTI = (NiRTTI*)0x108F41C;
static UInt32 D3DXVec3TransformCoordAddr = 0xC20856;
#else
static NiRTTI* NiLightRTTI = (NiRTTI*)0x11F4A28;
static UInt32 D3DXVec3TransformCoordAddr = 0xEE6DF8;
#endif

static std::map<NiAVObject*, NiPoint3> kLightPosMap;
static NiPoint3 kLightOffset;

template<typename FUNC>
void ForEachLight(NiNode* apNode, FUNC&& arFunc) {
	if (!apNode || !apNode->IsNiNode())
		return;

	UInt32 uiChildCount = apNode->GetArrayCount();
	if (!uiChildCount)
		return;

	for (UInt32 i = 0; i < uiChildCount; i++) {
		NiAVObject* pChild = apNode->GetAt(i);
		if (!pChild)
			continue;

		if (pChild->IsNiNode())
			ForEachLight((NiNode*)pChild, arFunc);
		else if (pChild->IsKindOf(*NiLightRTTI))
			arFunc(pChild);
	}
}

static NiPoint3* __stdcall EyeOffsetFixHook(NiPoint3* pOut, NiPoint3* pV, void* pM) {
	// Apply lighting offset - that's the whole fix
	NiPoint3 kLightOffset = BSShaderManager::GetShadowSceneNode(0)->kLightingOffset;
	pV->x += kLightOffset.x;
	pV->y += kLightOffset.y;
	pV->z += kLightOffset.z;
	return StdCall<NiPoint3*>(D3DXVec3TransformCoordAddr, pOut, pV, pM);
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
	for (auto& rData : kLightPosMap)
		rData.first->m_kWorld.m_Translate = rData.second;

	kLightPosMap.clear();
}

#ifdef FO3
static void __fastcall OffsetPlayerLightPositionsHook(void* apThis, void*, UInt32 auiThread, UInt32 auiStage) {
	OffsetLights(PlayerCharacter::GetSingleton()->spPlayerNode);
	ThisStdCall(0xB0D230, apThis, auiThread, auiStage);
}

static void __cdecl RestorePlayerLightPositionsHook(float afFOV) {
	RestoreLights(PlayerCharacter::GetSingleton()->spPlayerNode);
	CdeclCall(0xADA410, afFOV);
}

EXTERN_DLL_EXPORT bool FOSEPlugin_Query(const FOSEInterface* fose, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "ViewmodelShadingFix";
	info->version = 200;
	return true;
}

EXTERN_DLL_EXPORT bool FOSEPlugin_Load(FOSEInterface* fose) {
	if (!fose->isEditor) {
		// Game does not apply the lighting offset to the eye position for ShadowLightShader (it does for Lighting30Shader)
		// This causes viewmodel to have totally incorrect specular lighting
		ReplaceCall(0xAFBB69, EyeOffsetFixHook);

		// Game applies the lighting offset to lights attached to player's node, which causes them to be offset from the player
		// Ironic, isn't it?
		ReplaceCall(0x6E6AA3, OffsetPlayerLightPositionsHook);
		ReplaceCall(0x6E71D2, RestorePlayerLightPositionsHook);

		// Fix PipBoy menu light being offset (we do that already)
		SafeWrite8(0x6E68D3, 0xEB);
	}

	return true;
}
#else
static NiNode* __cdecl OffsetPlayerLightPositionsHook() {
	OffsetLights(PlayerCharacter::GetSingleton()->spPlayerNode);
	return *(NiNode**)0x11DEB7C;
}

static void __fastcall RestorePlayerLightPositionsHook(void* apThis) {
	RestoreLights(PlayerCharacter::GetSingleton()->spPlayerNode);
	ThisStdCall(0x404EE0, apThis);
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "ViewmodelShadingFix";
	info->version = 200;
	return true;
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Load(NVSEInterface* nvse) {
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
#endif