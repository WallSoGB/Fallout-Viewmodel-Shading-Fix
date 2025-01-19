#include "fose/fose/PluginAPI.h"
#include "nvse/PluginAPI.h"
#include "SafeWrite.h"
#include "GameData.hpp"

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)
#define VERSION 220

BS_ALLOCATORS;

#ifdef FO3
static NiRTTI* NiLightRTTI = (NiRTTI*)0x108F41C;
static UInt32 D3DXVec3TransformCoordAddr = 0xC20856;
#else
static NiRTTI* NiLightRTTI = (NiRTTI*)0x11F4A28;
static UInt32 D3DXVec3TransformCoordAddr = 0xEE6DF8;
#endif

template<typename FUNC>
void ForEachLight(NiNode* apNode, FUNC&& arFunc) {
	if (!apNode || !apNode->IsNiNode() || !apNode->GetChildCount())
		return;

	for (UInt32 i = 0; i < apNode->GetArrayCount(); i++) {
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
	const NiPoint3& kLightOffset = BSShaderManager::GetShadowSceneNode(0)->kLightingOffset;
	pV->x += kLightOffset.x;
	pV->y += kLightOffset.y;
	pV->z += kLightOffset.z;
	return StdCall<NiPoint3*>(D3DXVec3TransformCoordAddr, pOut, pV, pM);
}

static void OffsetLights(NiNode* apNode) {
	ForEachLight(apNode, [](NiAVObject* pLight) {
		// Apply negative offset to the light, so it will get cancelled out later on
		pLight->m_kWorld.m_Translate -= BSShaderManager::GetShadowSceneNode(0)->kLightingOffset;
		}
	);
}

static void __fastcall CullerFrustumFixHook(ShadowSceneNode* apThis, void*, NiCullingProcess* apCuller) {
	apCuller->SetFrustum(apCuller->m_pkCamera->m_kViewFrustum);
	apThis->PreOnVisible(apCuller);
}

#ifdef FO3
static void __fastcall OffsetPlayerLightPositionsHook(void* apThis, void*, UInt32 auiThread, UInt32 auiStage) {
	OffsetLights(PlayerCharacter::GetSingleton()->spPlayerNode);
	ThisStdCall(0xB0D230, apThis, auiThread, auiStage);
}

EXTERN_DLL_EXPORT bool FOSEPlugin_Query(const FOSEInterface* fose, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "ViewmodelShadingFix";
	info->version = VERSION;
	return !fose->isEditor;
}

EXTERN_DLL_EXPORT bool FOSEPlugin_Load(FOSEInterface* fose) {
	if (!fose->isEditor) {
		// Game does not apply the lighting offset to the eye position for ShadowLightShader (it does for Lighting30Shader)
		// This causes viewmodel to have totally incorrect specular lighting
		ReplaceCall(0xAFBB69, EyeOffsetFixHook);

		// Game applies the lighting offset to lights attached to player's node, which causes them to be offset from the player
		// Ironic, isn't it?
		ReplaceCall(0x6E6AA3, OffsetPlayerLightPositionsHook);

		// Fix PipBoy menu light being offset (we do that already)
		SafeWrite8(0x6E68D3, 0xEB);

		// Fix wrong camera frustum when accumulating PipBoy menu if shadows are disabled
		ReplaceCall(0x6EACD1, (UInt32)CullerFrustumFixHook);
	}

	return true;
}
#else
static NiNode* __cdecl OffsetPlayerLightPositionsHook() {
	OffsetLights(PlayerCharacter::GetSingleton()->spPlayerNode);
	return *(NiNode**)0x11DEB7C;
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "ViewmodelShadingFix";
	info->version = VERSION;
	return !nvse->isEditor;
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Load(NVSEInterface* nvse) {
	if (!nvse->isEditor) {
		// Game does not apply the lighting offset to the eye position for ShadowLightShader (it does for Lighting30Shader)
		// This causes viewmodel to have totally incorrect specular lighting
		ReplaceCall(0xB79019, EyeOffsetFixHook);

		// Game applies the lighting offset to lights attached to player's node, which causes them to be offset from the player
		// Ironic, isn't it?
		ReplaceCall(0x87513F, OffsetPlayerLightPositionsHook);

		// Fix PipBoy menu light being offset (we do that already)
		SafeWrite8(0x874F76, 0xEB);

		if (GetModuleHandle("PipBoyShadingFix.dll")) {
			MessageBox(NULL, "\"Pip Boy Shading Fix\" has been merged into \"View Model Shading Fix\".\nPlease remove \"Pip Boy Shading Fix.\"", "Viewmodel Shading Fix", MB_ICONERROR);
		}
		else {
			// Fix wrong camera frustum when accumulating PipBoy menu if shadows are disabled
			ReplaceCall(0x87091F, (UInt32)CullerFrustumFixHook);
		}
	}

	return true;
}
#endif

BOOL WINAPI DllMain(
	HANDLE  hDllHandle,
	DWORD   dwReason,
	LPVOID  lpreserved
)
{
	return TRUE;
}
