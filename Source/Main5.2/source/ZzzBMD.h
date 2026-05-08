#ifndef __ZZZBMD_H__
#define __ZZZBMD_H__

#include "TextureScript.h"

#define MAX_BONES    200
#define MAX_MESH     50
#define MAX_VERTICES 15000

#define RENDER_COLOR        0x00000001
#define RENDER_TEXTURE      0x00000002
#define RENDER_CHROME       0x00000004
#define RENDER_METAL        0x00000008
#define RENDER_LIGHTMAP     0x00000010
#define RENDER_SHADOWMAP    0x00000020
#define RENDER_BRIGHT       0x00000040
#define RENDER_DARK         0x00000080
#define RENDER_EXTRA        0x00000100
#define RENDER_CHROME2      0x00000200
#define RENDER_WAVE			0x00000400
#define RENDER_CHROME3      0x00000800 
#define RENDER_CHROME4      0x00001000
#define RENDER_NODEPTH      0x00002000
#define RENDER_CHROME5      0x00004000
#define RENDER_OIL          0x00008000
#define RENDER_CHROME6      0x00010000
#define RENDER_CHROME7      0x00020000
#define RENDER_DOPPELGANGER 0x00040000
#define RENDER_CHROME8      0x00080000
#define RENDER_WAVE_EXT		0x10000000
#define RENDER_BYSCRIPT		0x80000000
#define RNDEXT_WAVE			1
#define RNDEXT_OIL          2
#define RNDEXT_RISE			4

#define MAX_MONSTER_SOUND   11

enum EPartRenderPerfCategory
{
	PART_RENDER_WEAPON = 0,
	PART_RENDER_ARMOR,
	PART_RENDER_WING,
	PART_RENDER_HELPER,
	PART_RENDER_OTHER,
	PART_RENDER_CATEGORY_MAX
};

enum EPartGpuRejectReason
{
	PART_GPU_REJECT_NONE = 0,
	PART_GPU_REJECT_OFF,
	PART_GPU_REJECT_TYPE,
	PART_GPU_REJECT_SCALE,
	PART_GPU_REJECT_FLAG,
	PART_GPU_REJECT_MESH,
	PART_GPU_REJECT_SHADER,
	PART_GPU_REJECT_MAX
};

enum EPartGpuFlagRejectReason
{
	PART_GPU_FLAG_CHROME = 0,
	PART_GPU_FLAG_METAL,
	PART_GPU_FLAG_COLOR,
	PART_GPU_FLAG_WAVE,
	PART_GPU_FLAG_LIGHTMAP,
	PART_GPU_FLAG_OTHER,
	PART_GPU_FLAG_MAX
};

enum EPartGpuOffRejectReason
{
	PART_GPU_OFF_DISABLED = 0,
	PART_GPU_OFF_NOT_REQUESTED,
	PART_GPU_OFF_NO_TRANSFORM,
	PART_GPU_OFF_MAX
};

#define PART_GPU_REQ_TYPE_TOP_MAX 4

typedef struct
{
	double Ms;
	int Calls;
	int MeshCalls;
	int Triangles;
	int GpuHits;
	int GpuFallbacks;
	int Reject[PART_GPU_REJECT_MAX];
	int FlagReject[PART_GPU_FLAG_MAX];
	int OffReject[PART_GPU_OFF_MAX];
	int ReqType[PART_GPU_REQ_TYPE_TOP_MAX];
	int ReqTypeCount[PART_GPU_REQ_TYPE_TOP_MAX];
} PART_RENDER_PERF_BUCKET;

typedef struct
{
	PART_RENDER_PERF_BUCKET Bucket[PART_RENDER_CATEGORY_MAX];
} PART_RENDER_PERF_STATS;

extern PART_RENDER_PERF_STATS g_PartRenderPerfStats;

int ZzzPerfClassifyPartType(int Type);
__int64 ZzzPerfBeginPart(int Category, int Type);
void ZzzPerfEndPart(int Category, __int64 StartCounter);
void ZzzPerfRecordPartMesh(int Category, int Triangles, bool GpuHit, int RejectReason);
int ZzzPerfGetActivePartCategory();
int ZzzPerfGetActivePartType();
void ZzzPerfSnapshotPartStats(PART_RENDER_PERF_STATS* OutStats, bool Reset);
const char* ZzzPerfPartCategoryName(int Category);
void ZzzGpuAssistResetState();

typedef struct
{
	vec3_t Position;
	vec3_t Color;
	float  Range;
} Light_t;

typedef struct
{
	vec3_t* Position;
	vec3_t* Rotation;
	vec4_t* Quaternion;
} BoneMatrix_t;

typedef struct
{
	char			Name[32];
	short			Parent;
	char			Dummy;
	BoneMatrix_t*	BoneMatrixes;
	char			BoundingBox;
	vec3_t			BoundingVertices[8];
} Bone_t;

typedef struct
{
	char FileName[32];
} Texture_t;

typedef struct
{
	unsigned char Width;
	unsigned char Height;
	unsigned char* Buffer;
} Bitmap_t;

typedef struct
{
	short  Node;
	vec3_t Position;
} Vertex_t;

typedef struct
{
	short  Node;
	vec3_t Normal;
	short  BindVertex;
} Normal_t;

typedef struct
{
	float TexCoordU;
	float TexCoordV;
} TexCoord_t;

typedef struct
{
	BYTE m_Colors[3];	//0~255 RGB
} VertexColor_t;

typedef struct
{
	char       Polygon;
	short      VertexIndex[4];
	short      NormalIndex[4];
	short      TexCoordIndex[4];
	short      EdgeTriangleIndex[4];
	bool       Front;
	//TexCoord_t LightMapCoord[4]; //ver1.2
	//short      LightMapIndexes; //ver1.2
} Triangle_t;

typedef struct
{
	char       Polygon;
	short      VertexIndex[4];
	short      NormalIndex[4];
	short      TexCoordIndex[4];
	TexCoord_t LightMapCoord[4]; //ver1.2
	short      LightMapIndexes; //ver1.2
} Triangle_t2;


typedef struct
{
	bool          Loop;
	float         PlaySpeed;
	short         NumAnimationKeys;
	bool          LockPositions;
	vec3_t* Positions;
} Action_t;

typedef struct _Triangle_t3 : public Triangle_t
{
	short	   m_ivIndexAdditional[4];
} Triangle_t3;

typedef struct _Mesh_t
{
	bool			NoneBlendMesh;
	short			Texture;
	short			NumVertices;
	short			NumNormals;
	short			NumTexCoords;
	short			NumVertexColors;	//ver1.3
	short			NumTriangles;
	int				NumCommandBytes; //ver1.1
	Vertex_t*		Vertices;
	Normal_t*		Normals;
	TexCoord_t*		TexCoords;
	VertexColor_t*	VertexColors;	//ver1.3
	Triangle_t*		Triangles;
	unsigned char*	Commands; //ver1.1
	TextureScript*	m_csTScript;
	short*			RenderVertexIndices;
	short*			RenderNormalIndices;
	vec2_t*			RenderTexCoords;
	int				RenderVertexCount;

	GLuint VAO, VBO_Vertices, VBO_Normals, VBO_TexCoords, VBO_Colors, EBO;
	GLuint VBO_VertexNodes, VBO_NormalNodes;
	int GpuVertexCount;

	_Mesh_t()
	{
		Vertices = NULL;
		Normals = NULL;
		Triangles = NULL;
		Commands = NULL;
		m_csTScript = NULL;

		TexCoords = NULL;
		VertexColors = NULL;
		RenderVertexIndices = NULL;
		RenderNormalIndices = NULL;
		RenderTexCoords = NULL;
		RenderVertexCount = 0;

		Texture = -1;
		NumCommandBytes = 0;
		NoneBlendMesh = false;

		NumVertices = NumNormals = NumTexCoords =
			NumVertexColors = NumTriangles = 0;

		VAO = 0;
		VBO_Vertices = 0;
		VBO_Normals = 0;
		VBO_TexCoords = 0;
		VBO_Colors = 0;
		EBO = 0;
		VBO_VertexNodes = 0;
		VBO_NormalNodes = 0;
		GpuVertexCount = 0;
	}

} Mesh_t;

#include "CGMScriptLua.h"


class BMD
{
public:
	/*+000*/ char         FileName[64];

	/*+000*/ char         Name[32];
	/*+032*/ char         Version;
	/*+034*/ short        NumBones;
	/*+036*/ short        NumMeshs;
	/*+038*/ short        NumActions;
	/*+040*/ Mesh_t*      Meshs;
	/*+044*/ Bone_t*      Bones;
	/*+048*/ Action_t*    Actions;
	/*+052*/ Texture_t*   Textures;
	/*+056*/ GLuint*      IndexTexture;
	/*+060*/ GLuint*      LightTexture;
	/*+060*/ short        NumLightMaps;  //ver1.2
	/*+062*/ short        IndexLightMap; //ver1.2
	/*+064*/ Bitmap_t*    LightMaps;    //ver1.2
	/*+068*/ vec3_t       ColorLight;
	/*+068*/ bool         LightEnable;
	/*+069*/ bool         ContrastEnable;
	/*+072*/ vec3_t       BodyLight;
	/*+084*/ int          BoneHead;
	/*+088*/ int          BoneFoot[4];
	/*+104*/ float        BodyScale;
	/*+108*/ vec3_t       BodyOrigin;
	/*+120*/ vec3_t       BodyAngle;
	/*+132*/ float        BodyHeight;
	/*+136*/ char         StreamMesh;
	/*+140*/ vec3_t       ShadowAngle;
	/*+152*/ char         Skin;
	/*+153*/ bool         HideSkin;
	/*+154*/ float        Velocity;
	/*+160*/ WORD         CurrentAction;
	/*+162*/ WORD         PriorAction;
	/*+164*/ float        CurrentAnimation;
	/*+168*/ short        CurrentAnimationFrame;
	/*+170*/ short        Sounds[MAX_MONSTER_SOUND];
	/*+192*/ int          renderCount;
	/*+196*/ float        fTransformedSize;
	/*+224*/ unsigned int m_iBMDSeqID;
	/*+252*/ bool         bLightMap;
	/*+253*/ bool         bOffLight;
	/*+254*/ char         iBillType;
	/*+255*/ bool         m_bCompletedAlloc;
	/*+256*/ bool         LightMapEnable;
	/*+256*/ int          unknown;
	bool         m_bGpuAssistRequested;
	bool         m_bGpuAssistTransformReady;
	bool         m_bGpuAssistCpuDataReady;
	bool         m_bGpuAssistBodyPath;
	bool         m_bGpuAssistAttachmentPath;
	bool         m_bGpuAssistGlobalTransform;
	bool         m_bGpuAssistTranslate;
	float        m_fGpuAssistScale;
	int          m_iGpuAssistObjectType;
	BYTE         m_byGpuAssistObjectKind;
	unsigned int m_uiGpuAssistTransformSerial;
	float      (*m_pGpuAssistBoneMatrices)[3][4];
	OBB_t*       m_pGpuAssistOBB;
	vec3_t       m_vGpuAssistBoundingBoxMin;
	vec3_t       m_vGpuAssistBoundingBoxMax;
	float        m_GpuAssistBones[MAX_BONES][16];

	CGMLuaBase* commandLua;
	BMD() : NumBones(0), NumActions(0), NumMeshs(0),
		Meshs(NULL), Bones(NULL), Actions(NULL), Textures(NULL), IndexTexture(NULL)
	{
		LightEnable = false;
		ContrastEnable = false;
		HideSkin = false;
		bLightMap = false;
		iBillType = -1;
		bOffLight = false;
		m_bCompletedAlloc = false;
		memset(FileName, 0, sizeof(FileName));

		unknown = 0;
		commandLua = NULL;
		m_bGpuAssistRequested = false;
		m_bGpuAssistTransformReady = false;
		m_bGpuAssistCpuDataReady = false;
		m_bGpuAssistBodyPath = false;
		m_bGpuAssistAttachmentPath = false;
		m_bGpuAssistGlobalTransform = false;
		m_bGpuAssistTranslate = false;
		m_fGpuAssistScale = 0.0f;
		m_iGpuAssistObjectType = -1;
		m_byGpuAssistObjectKind = 0;
		m_uiGpuAssistTransformSerial = 0;
		m_pGpuAssistBoneMatrices = NULL;
		m_pGpuAssistOBB = NULL;
		memset(m_vGpuAssistBoundingBoxMin, 0, sizeof(m_vGpuAssistBoundingBoxMin));
		memset(m_vGpuAssistBoundingBoxMax, 0, sizeof(m_vGpuAssistBoundingBoxMax));
		memset(m_GpuAssistBones, 0, sizeof(m_GpuAssistBones));
	}

	~BMD();
	//utility
	void Init(bool Dummy);
	bool Open(char* DirName, char* FileName);
	bool Save(char* DirName, char* FileName);
	bool Open2(char* DirName, char* FileName, bool bReAlloc = true);
	bool OpenPack(unsigned char* Data, long DataBytes, bool bReAlloc = true);
	bool Save2(char* DirName, char* FileName);
	void Release();
	void CreateBoundingBox();
	void CreateVertexBuffer(int i, Mesh_t& mesh);
	void RenderVertexBuffer(int i, Mesh_t* m, int vertex_index, vec3_t* vertices, vec2_t* textCoords, vec4_t* colors);
	void PrepareGpuAssist(OBJECT* pObject, int modelType, int renderTypeHint, bool isBodyModel, bool globalTransform, bool translate);
	void ClearGpuAssist();
	void EnsureCpuTransformData();
	bool CanUseGpuAssistMesh(int renderFlag, int resolvedRenderFlag, bool enableWave, bool enableLight) const;
	void RenderMeshGpuAssist(Mesh_t* m, bool enableLight, float alpha, float texCoordOffsetU, float texCoordOffsetV, int renderMode);
	void CacheGpuAssistBoneMatrices();
	void ComputeGpuAssistLightDirection(vec3_t lightDir) const;
	void PrepareGpuAssistBounds(vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB);
	void TransformCpuInternal(float(*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB, bool Translate = false, float _Scale = 0.0f);

	bool PlayAnimation(float* AnimationFrame, float* PriorAnimationFrame, unsigned short* PriorAction, float Speed, vec3_t Origin, vec3_t Angle);
	void Animation(float(*BoneTransform)[3][4], float AnimationFrame, float PriorAnimationFrame, unsigned short PriorAction, vec3_t Angle, vec3_t HeadAngle, bool Parent = false, bool Translate = true);
	void InterpolationTrans(float(*Mat1)[4], float(*TransMat2)[4], float _Scale);
	void Transform(float(*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB, bool Translate = false, float _Scale = 0.0f);
	void TransformByObjectBone(vec3_t vResultPosition, OBJECT* pObject, int iBoneNumber, vec3_t vRelativePosition = NULL);
	void TransformByBoneMatrix(vec3_t vResultPosition, float(*BoneMatrix)[4], vec3_t vWorldPosition = NULL, vec3_t vRelativePosition = NULL);
	void TransformPosition(float(*Matrix)[4], vec3_t Position, vec3_t WorldPosition, bool Translate = false);
	void RotationPosition(float(*Matrix)[4], vec3_t Position, vec3_t WorldPosition);
public:
	void AnimationTransformWithAttachHighModel_usingGlobalTM(OBJECT* oHighHierarchyModel, BMD* bmdHighHierarchyModel, int iBoneNumberHighHierarchyModel, vec3_t& vOutPosHighHiearachyModelBone, vec3_t* arrOutSetfAllBonePositions, bool bApplyTMtoVertices);
	void AnimationTransformWithAttachHighModel(OBJECT* oHighHierarchyModel, BMD* bmdHighHierarchyModel, int iBoneNumberHighHierarchyModel, vec3_t& vOutPosHighHiearachyModelBone, vec3_t* arrOutSetfAllBonePositions);
	void AnimationTransformOnlySelf(vec3_t* arrOutSetfAllBonePositions, const OBJECT* oSelf);

	void AnimationTransformOnlySelf(vec3_t* arrOutSetfAllBonePositions, const vec3_t& v3Angle, const vec3_t& v3Position, const float& fScale, OBJECT* oRefAnimation = NULL, const float fFrameArea = -1.0f, const float fWeight = -1.0f);

	void Lighting(float*, Light_t*, vec3_t, vec3_t);
	void Chrome(float*, int, vec3_t);

	//render
	void RenderBone(float(*BoneTransform)[3][4]);
	void RenderObjectBoundingBox();
	void BeginRender(float);
	void EndRender();

	bool runtime_linkaction(PART_t* f, OBJECT* pObject);
	bool runtime_render(OBJECT* pObject, float Alpha, int RenderType, BMD* oHighHierarchyModel);
	bool runtime_render_effect(OBJECT* pObject, float Alpha, int RenderType, int Texture, BMD* oHighHierarchyModel);
	
	void decripted_unique_lua(BYTE* DataBytes, int pos, int DataSize);

	void RenderMeshEffect(int i, int iType, int iSubType = 0, vec3_t Angle = 0, VOID* obj = NULL);
	void RenderMesh(int i, int RenderFlag, float Alpha = 1.f, int BlendMesh = -1, float BlendMeshLight = 1.f, float BlendMeshTexCoordU = 0.f, float BlendMeshTexCoordV = 0.f, int Texture = -1);
	void RenderBody(int RenderFlag, float Alpha = 1.f, int BlendMesh = -1, float BlendMeshLight = 1.f, float BlendMeshTexCoordU = 0.f, float BlendMeshTexCoordV = 0.f, int HiddenMesh = -1, int Texture = -1);
	void RenderMeshTranslate(int i, int RenderFlag, float Alpha = 1.f, int BlendMesh = -1, float BlendMeshLight = 1.f, float BlendMeshTexCoordU = 0.f, float BlendMeshTexCoordV = 0.f, int Texture = -1);
	void RenderBodyTranslate(int RenderFlag, float Alpha = 1.f, int BlendMesh = -1, float BlendMeshLight = 1.f, float BlendMeshTexCoordU = 0.f, float BlendMeshTexCoordV = 0.f, int HiddenMesh = -1, int Texture = -1);

	void AddClothesShadowTriangles(void* pClothes, const int clothesCount, const float sx, const float sy) const;
	void AddMeshShadowTriangles(const int blendMesh, const int hiddenMesh, const int startMesh, const int endMesh, const float sx, const float sy) const;

	void RenderBodyShadow(int BlendMesh = -1, int HiddenMesh = -1, int StartMeshNumber = -1, int EndMeshNumber = -1, void* pClothes = NULL, int ClothesCount = 0);


	float GetBodyLight(int index) {
		return BodyLight[index];
	};
	void SetBodyLight(vec3_t right) {
		VectorCopy(right, BodyLight);
	};
	void SetBodyLight(float c1, float c2, float c3) {
		BodyLight[0] = c1;
		BodyLight[1] = c2;
		BodyLight[2] = c3;
	};

	//lightmap(ver1.2)
	bool CollisionDetectLineToMesh(vec3_t, vec3_t, bool Collision = true, int Mesh = -1, int Triangle = -1);
	void CreateLightMapSurface(Light_t*, Mesh_t*, int, int, int, int, int, int, vec3_t, vec3_t, int);
	void CreateLightMaps();
	void BindLightMaps();
	void ReleaseLightMaps();
	void FindNearTriangle(void);
	void FindTriangleForEdge(int iMesh, int iTri, int iIndex11);

	int FindBoneName(char* name);
	bool GetBonePosition(OBJECT* pObject, char* name, OUT vec3_t Position);
	Action_t* GetAction(int i);
	Mesh_t* GetMesh(int i);
	bool IsTextureHide(int index);

	void runtime_render_level(OBJECT* pObj, int GroupId, float r, float g, float b);
	bool runtime_make_render(int EffectIndex, OBJECT* pObj);
	bool runtime_make_effect(int EffectIndex, OBJECT* pObj, float Alpha, int RenderType, int Texture);

};

extern float BoneTransform[MAX_BONES][3][4];
extern vec3_t VertexTransform[MAX_MESH][MAX_VERTICES];
extern vec3_t NormalTransform[MAX_MESH][MAX_VERTICES];
extern vec3_t LightTransform[MAX_MESH][MAX_VERTICES];
extern float  IntensityTransform[MAX_MESH][MAX_VERTICES];
extern float g_chrome[MAX_VERTICES][2];
#endif
