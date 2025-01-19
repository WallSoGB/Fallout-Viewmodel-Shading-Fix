#pragma once

#include "SafeWrite.h"
#include "Utilities.h"
#include "Allocator/BSMemory.hpp"

#define ASSERT_SIZE(a, b) static_assert(sizeof(a) == b, "Wrong structure size!");
#define ASSERT_OFFSET(a, b, c) static_assert(offsetof(a, b) == c, "Wrong member offset!");
#define CREATE_OBJECT(CLASS, ADDRESS) static CLASS* CreateObject() { return StdCall<CLASS*>(ADDRESS); };
#ifdef FO3
#define IS_NODE(object) ((*(UInt32**)object)[3 * 4 >> 2] == 0xAA2340)
#else
#define IS_NODE(object) ((*(UInt32**)object)[3 * 4 >> 2] == 0x6815C0)
#endif

class BGSDistantObjectBlock;
class bhkBlendCollisionObject;
class bhkCollisionObject;
class bhkLimitedHingeConstraint;
class bhkRigidBody;
class BSFadeNode;
class BSMultiBoundNode;
class BSMultiBound;
class BSResizableTriShape;
class BSSegmentedTriShape;
class NiCloningProcess;
class NiGeometry;
class NiLines;
class NiNode;
class NiParticles;
class NiStream;
class NiTriBasedGeom;
class NiTriShape;
class NiTriStrips;
class NiControllerManager;
class NiObjectGroup;
class NiObjectNET;
class NiBound;
class NiViewerStringsArray;
class NiUpdateData;
class NiMatrix3;
class NiCullingProcess;
class NiFixedString;
class NiGeometryData;
class NiSkinInstance;
class bhkNiCollisionObject;
class NiDX9Renderer;
class BSShader;
class BGSTextureUseMap;
class NiSourceTexture;
class RenderPassArray;
class RenderPass;
class BSShaderAccumulator;
class NiAdditionalGeometryData;
class NiGeometryBufferData;
class NiD3DPass;
class NiD3DShaderDeclaration;
class NiD3DRenderStateGroup;
class NiD3DShaderConstantMap;
class NiDX9ShaderDeclaration;
class NiDynamicEffectState;
class BGSDistantTreeBlock;
class BGSTerrainChunk;
class NiProperty;
class NiNode;
class BSMultiBoundRoom;
class NiTimeController;

class NiMatrix3 {
public:
	float m_pEntry[3][3];
};

class NiPoint2 {
public:
	float x, y;
};

class NiPoint3 {
public:
	float x, y, z;

	NiPoint3 operator- (const NiPoint3& arPt) const { return NiPoint3(x - arPt.x, y - arPt.y, z - arPt.z); };
	NiPoint3 operator- () const { return NiPoint3(-x, -y, -z); };
	NiPoint3 operator-= (const NiPoint3& arPt) { x -= arPt.x; y -= arPt.y; z -= arPt.z; return *this; };
};

class NiPoint4 {
public:
	float x, y, z, w;
};

class NiTransform {
public:
	NiMatrix3	m_Rotate;
	NiPoint3	m_Translate;
	float		m_fScale;
};

class NiRTTI {
public:
	const char*		m_pcName;
	const NiRTTI*	m_pkBaseRTTI;
};

template <typename T_Data>
class NiTArray {
public:
	virtual ~NiTArray();

	T_Data* m_pBase;
	UInt16 m_usMaxSize;
	UInt16 m_usSize;
	UInt16 m_usESize;
	UInt16 m_usGrowBy;
};

ASSERT_SIZE(NiTArray<void*>, 0x10);

typedef void* NiTListIterator;

template <typename T_Data>
class NiTListItem {
public:
	NiTListItem*	m_pkNext;
	NiTListItem*	m_pkPrev;
	T_Data			m_element;
};

template <typename T_Data>
class NiTListBase {
public:
	NiTListItem<T_Data>*	m_pkHead;
	NiTListItem<T_Data>*	m_pkTail;
	UInt32					m_uiCount;

	inline UInt32 GetSize() const { return m_uiCount; };
	bool IsEmpty() const { return m_uiCount == 0; };

	NiTListIterator GetHeadPos() const { return m_pkHead; };
	NiTListIterator GetTailPos() const { return m_pkTail; };
};


template <class T_Data>
class NiPointer {
public:
	NiPointer(T_Data* apObject = (T_Data*)0) {
		m_pObject = apObject;
		if (m_pObject)
			m_pObject->IncRefCount();
	}
	NiPointer(const NiPointer& arPointer) {
		m_pObject = arPointer.m_pObject;
		if (m_pObject)
			m_pObject->IncRefCount();
	}
	~NiPointer() {
		if (m_pObject)
			m_pObject->DecRefCount();
	}

	T_Data* m_pObject;

	__forceinline NiPointer<T_Data>& operator =(const NiPointer& arPointer) {
		if (m_pObject != arPointer.m_pObject) {
			if (m_pObject)
				m_pObject->DecRefCount();
			m_pObject = arPointer.m_pObject;
			if (m_pObject)
				m_pObject->IncRefCount();
		}
		return *this;
	}

	__forceinline NiPointer<T_Data>& operator =(T_Data* apObject) {
		if (m_pObject != apObject) {
			if (m_pObject)
				m_pObject->DecRefCount();
			m_pObject = apObject;
			if (m_pObject)
				m_pObject->IncRefCount();
		}
		return *this;
	}

	__forceinline bool operator==(T_Data* apObject) const { return (m_pObject == apObject); }
	__forceinline bool operator==(const NiPointer& arPointer) const { return (m_pObject == arPointer.m_pObject); }
	__forceinline operator bool() const { return m_pObject != nullptr; }
	__forceinline operator T_Data* () const { return m_pObject; }
	__forceinline T_Data& operator*() const { return *m_pObject; }
	__forceinline T_Data* operator->() const { return m_pObject; }
};

class NiFrustum {
public:
	NiFrustum() : m_fLeft(0.0f), m_fRight(0.0f), m_fTop(0.0f), m_fBottom(0.0f), m_fNear(0.0f), m_fFar(0.0f), m_bOrtho(false) {}
	NiFrustum(float afLeft, float afRight, float afTop, float afBottom, float afNear, float afFar, bool abOrtho) {
		m_fLeft = afLeft;
		m_fRight = afRight;
		m_fTop = afTop;
		m_fBottom = afBottom;
		m_fNear = afNear;
		m_fFar = afFar;
		m_bOrtho = abOrtho;
	}
	~NiFrustum() {};

	float	m_fLeft;
	float	m_fRight;
	float	m_fTop;
	float	m_fBottom;
	float	m_fNear;
	float	m_fFar;
	bool	m_bOrtho;
};

template <class T> class NiRect {
public:
	NiRect(T left = T(0), T right = T(0), T top = T(0), T bottom = T(0))
	{
		m_left = left;
		m_right = right;
		m_top = top;
		m_bottom = bottom;
	}

	T m_left, m_right, m_top, m_bottom;
};

class NiRefObject {
public:
    virtual			~NiRefObject();
    virtual void	DeleteThis();

    UInt32 m_uiRefCount;

    // 0x40F6E0
    inline void IncRefCount() {
        InterlockedIncrement(&m_uiRefCount);
    }

    // 0x401970
    inline void DecRefCount() {
        if (!InterlockedDecrement(&m_uiRefCount))
            DeleteThis();
    }
};

class NiObject : public NiRefObject {
public:
    virtual const NiRTTI*				GetRTTI() const;												// 02 | Returns NiRTTI of the object
	virtual NiNode*						IsNiNode() const;												// 03 | Returns this if it's a NiNode, otherwise nullptr
	virtual BSFadeNode*					IsFadeNode() const;												// 04 | Returns this if it's a BSFadeNode, otherwise nullptr
	virtual BSMultiBoundNode*			IsMultiBoundNode() const;										// 05 | Returns this if it's a BSMultiBoundNode, otherwise nullptr
	virtual NiGeometry*					IsGeometry() const;												// 06 | Returns this if it's a NiGeometry, otherwise nullptr
	virtual NiTriBasedGeom*				IsTriBasedGeometry() const;										// 07 | Returns this if it's a NiTriBasedGeom, otherwise nullptr
	virtual NiTriStrips*				IsTriStrips() const;											// 08 | Returns this if it's a NiTriStrips, otherwise nullptr
	virtual NiTriShape*					IsTriShape() const;												// 09 | Returns this if it's a NiTriShape, otherwise nullptr
	virtual BSSegmentedTriShape*		IsSegmentedTriShape() const;									// 10 | Returns this if it's a BSSegmentedTriShape, otherwise nullptr
	virtual BSResizableTriShape*		IsResizableTriShape() const;									// 11 | Returns this if it's a BSResizableTriShape, otherwise nullptr
	virtual NiParticles*				IsParticlesGeom() const;										// 12 | Returns this if it's a NiParticles, otherwise nullptr
	virtual NiLines*					IsLinesGeom() const;											// 13 | Returns this if it's a NiLines, otherwise nullptr
	virtual bhkCollisionObject*			IsBhkNiCollisionObject() const;									// 14 | Returns this if it's a bhkCollisionObject, otherwise nullptr
	virtual bhkBlendCollisionObject*	IsBhkBlendCollisionObject() const;								// 15 | Returns this if it's a bhkBlendCollisionObject, otherwise nullptr
	virtual bhkRigidBody*				IsBhkRigidBody() const;											// 16 | Returns this if it's a bhkRigidBody, otherwise nullptr
	virtual bhkLimitedHingeConstraint*	IsBhkLimitedHingeConstraint() const;							// 17 | Returns this if it's a bhkLimitedHingeConstraint, otherwise nullptr
	virtual NiObject*					CreateClone(NiCloningProcess* apCloning);						// 18 | Creates a clone of this object
	virtual void						LoadBinary(NiStream* apStream);									// 19 | Loads objects from disk
	virtual void						LinkObject(NiStream* apStream);									// 20 | Called by the streaming system to resolve links to other objects once it can be guaranteed that all objects have been loaded
	virtual void						RegisterStreamables(NiStream* apStream);						// 21 | When an object is inserted into a stream, it calls register streamables to make sure that any contained objects or objects linked in a scene graph are streamed as well
	virtual void						SaveBinary(NiStream* apStream);									// 22 | Saves objects to disk
	virtual bool						IsEqual(NiObject* apObject) const;								// 23 | Compares this object with another
	virtual void						GetViewerStrings(NiViewerStringsArray* apStrings);				// 24 | Gets strings containing information about the object
	virtual void						AddViewerStrings(NiViewerStringsArray* apStrings);				// 25 | Adds additional strings containing information about the object
	virtual void						ProcessClone(NiCloningProcess* apCloning);						// 26 | Post process for CreateClone
	virtual void						PostLinkObject(NiStream* apStream);								// 27 | Called by the streaming system to resolve any tasks that require other objects to be correctly linked. It is called by the streaming system after LinkObject has been called on all streamed objects
	virtual bool						StreamCanSkip();												// 28
	virtual const NiRTTI*				GetStreamableRTTI();											// 29
	virtual void						SetBound(NiBound* apNewBound);									// 30 | Replaces the bound of the object
	virtual void						GetBlockAllocationSize();										// 31 | Used by geometry data
	virtual NiObjectGroup*				GetGroup();														// 32 | Used by geometry data
	virtual void						SetGroup(NiObjectGroup* apGroup);								// 33 | Used by geometry data
	virtual NiControllerManager*		IsControllerManager() const;									// 34 | Returns this if it's a NiControllerManager, otherwise nullptr

	// 0x6532C0
	bool IsKindOf(const NiRTTI& apRTTI) const {
		for (const NiRTTI* i = GetRTTI(); i; i = i->m_pkBaseRTTI) {
			if (i == &apRTTI)
				return true;
		}
		return false;
	}
};
class NiObjectNET : public NiObject {
public:
	const char*						m_kName;
	NiPointer<NiTimeController>		m_spControllers;
	void**							m_ppkExtra;
	UInt16							m_usExtraDataSize;
	UInt16							m_usMaxSize;
};

class NiAVObject : public NiObjectNET {
public:
	virtual void			UpdateControllers(NiUpdateData& arData);
	virtual void			ApplyTransform(NiMatrix3& akMat, NiPoint3& akTrn, bool abOnLeft);
	virtual void			Unk_39();
	virtual NiAVObject*		GetObject_(const NiFixedString& kName);
	virtual NiAVObject*		GetObjectByName(const NiFixedString& kName);
	virtual void			SetSelectiveUpdateFlags(bool* bSelectiveUpdate, bool bSelectiveUpdateTransforms, bool* bRigid);
	virtual void			UpdateDownwardPass(const NiUpdateData& arData, UInt32 uFlags);
	virtual void			UpdateSelectedDownwardPass(const NiUpdateData& arData, UInt32 uFlags);
	virtual void			UpdateRigidDownwardPass(const NiUpdateData& arData, UInt32 uFlags);
	virtual void			Unk_46(void* arg);
	virtual void			UpdateTransform();
	virtual void			UpdateWorldData(const NiUpdateData& arData);
	virtual void			UpdateWorldBound();
	virtual void			UpdateTransformAndBounds(const NiUpdateData& arData);
	virtual void			PreAttachUpdate(NiNode* pEventualParent, const NiUpdateData& arData);
	virtual void			PreAttachUpdateProperties(NiNode* pEventualParent);
	virtual void			DetachParent();
	virtual void			UpdateUpwardPassParent(void* arg);
	virtual void			OnVisible(NiCullingProcess* kCuller);
	virtual void			PurgeRendererData(NiDX9Renderer* apRenderer);

	NiNode*							m_pkParent;
	bhkNiCollisionObject*			m_spCollisionObject;
	NiBound*						m_kWorldBound;
	NiTListBase<NiProperty*>		m_kPropertyList;
	Bitfield32						m_uiFlags;
	NiTransform						m_kLocal;
	NiTransform						m_kWorld;
};

class NiNode : public NiAVObject {
public:
	virtual void			AttachChild(NiAVObject* apChild, bool abFirstAvail);
	virtual void			InsertChildAt(UInt32 i, NiAVObject* apChild);
	virtual void			DetachChild(NiAVObject* apChild, NiAVObject*& aspAVObject);
	virtual void			DetachChildAlt(NiAVObject* apChild);
	virtual void			DetachChildAt(UInt32 i, NiAVObject*& aspAVObject);
	virtual NiAVObject*		DetachChildAtAlt(UInt32 i);
	virtual void			SetAt(UInt32 i, NiAVObject* apChild, NiAVObject*& aspAVObject);
	virtual void			SetAtAlt(UInt32 i, NiAVObject* apChild);
	virtual void			UpdateUpwardPass();

	NiTArray<NiAVObject*> m_kChildren;

    UInt32 GetChildCount() const {
		return m_kChildren.m_usESize;
    }

	UInt32 GetArrayCount() const {
		return m_kChildren.m_usSize;
	}

	NiAVObject* GetAt(UInt32 index) const {
		return m_kChildren.m_pBase[index];
	}
};

class NiCamera : public NiAVObject {
public:
	NiCamera();
	virtual ~NiCamera();

	float			m_aafWorldToCam[4][4];
	NiFrustum		m_kViewFrustum;
	float			m_fMinNearPlaneDist;
	float			m_fMaxFarNearRatio;
	NiRect<float>	m_kPort;
	float			m_fLODAdjust;
};

class NiPlane {
public:
	NiPoint3	m_kNormal;
	float		m_fConstant;
};

class NiFrustumPlanes {
public:
	NiPlane	m_akCullingPlanes[6];
	UInt32	m_uiActivePlanes;

	void Set(const NiFrustum& arFrustum, const NiTransform& arTransform) {
#ifdef FO3
		ThisStdCall(0x83ECB0, this, &arFrustum, &arTransform);
#else
		ThisStdCall(0xA74E10, this, &arFrustum, &arTransform);
#endif
	}
};

class NiCullingProcess {
public:
	virtual const NiRTTI*				GetRTTI() const;
	virtual NiNode*						IsNiNode() const;
	virtual BSFadeNode*					IsFadeNode() const;
	virtual BSMultiBoundNode*			IsMultiBoundNode() const;
	virtual NiGeometry*					IsGeometry() const;
	virtual NiTriBasedGeom*				IsTriBasedGeometry() const;
	virtual NiTriStrips*				IsTriStrips() const;
	virtual NiTriShape*					IsTriShape() const;
	virtual BSSegmentedTriShape*		IsSegmentedTriShape() const;
	virtual BSResizableTriShape*		IsResizableTriShape() const;
	virtual NiParticles*				IsParticlesGeom() const;
	virtual NiLines*					IsLinesGeom() const;
	virtual bhkNiCollisionObject*		IsBhkNiCollisionObject() const;
	virtual bhkBlendCollisionObject*	IsBhkBlendCollisionObject() const;
	virtual bhkRigidBody*				IsBhkRigidBody() const;
	virtual bhkLimitedHingeConstraint*	IsBhkLimitedHingeConstraint() const;
	virtual								~NiCullingProcess();
	virtual void						Process(NiAVObject* apObject);
	virtual void						ProcessAlt(const NiCamera* apCamera, NiAVObject* apScene, void* apVisibleSet);

	bool				m_bUseVirtualAppend;
	void*				m_pkVisibleSet;
	NiCamera*			m_pkCamera;
	NiFrustum			m_kFrustum;
	NiFrustumPlanes		m_kPlanes;

	void SetFrustum(const NiFrustum& arFrustum) {
		if (m_pkCamera) {
			m_kFrustum = arFrustum;
			m_kPlanes.Set(m_kFrustum, m_pkCamera->m_kWorld);
			m_kPlanes.m_uiActivePlanes = 63;
		}
	}
};

class ShadowSceneLight : public NiRefObject {
public:
	UInt32		filler[250 / 4];
	NiPoint3	kPointPosition;
};
ASSERT_OFFSET(ShadowSceneLight, kPointPosition, 256);

class ShadowSceneNode {
public:
	UInt32		filler[121];
	NiPoint3	kLightingOffset;

	void PreOnVisible(NiCullingProcess* apCuller) {
#ifdef FO3
		ThisStdCall(0xAE5870, this, apCuller);
#else
		ThisStdCall(0xB5E870, this, apCuller);
#endif
	}

	ShadowSceneLight* GetLight(NiAVObject* apLight) {
#ifdef FO3
		return ThisStdCall<ShadowSceneLight*>(0xAE1D30, this, apLight);
#else
		return ThisStdCall<ShadowSceneLight*>(0xB5B4A0, this, apLight);
#endif
	}
};
ASSERT_OFFSET(ShadowSceneNode, kLightingOffset, 0x1E4);

class BSShaderManager {
public:
	static ShadowSceneNode* GetShadowSceneNode(UInt32 aeType) {
#ifdef FO3
		return ((ShadowSceneNode**)0x116EFB8)[aeType];
#else
		return ((ShadowSceneNode**)0x11F91C8)[aeType];
#endif
	}
};


#ifdef FO3
class PlayerCharacter {
public:
	UInt32 filler[0x5F0 / 4];
	NiPointer<NiNode> spPlayerNode;

	static PlayerCharacter* GetSingleton() {
		return *(PlayerCharacter**)0x107A104;
	}
};
ASSERT_OFFSET(PlayerCharacter, spPlayerNode, 0x5F0);
#else
class PlayerCharacter {
public:
	UInt32 filler[0x694 / 4];
	NiPointer<NiNode> spPlayerNode;

	static PlayerCharacter* GetSingleton() {
		return *(PlayerCharacter**)0x11DEA3C;
	}
};
ASSERT_OFFSET(PlayerCharacter, spPlayerNode, 0x694);
#endif