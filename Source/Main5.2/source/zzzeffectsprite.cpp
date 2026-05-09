///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stddef.h>
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAi.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "WSClient.h"
#include "NewUISystem.h"
#include "CShaderGL.h"
#include "CGMCharacter.h"
#include "ZzzScene.h"


OBJECT Sprites[MAX_SPRITES];
struct GpuSpriteBillboard
{
	vec3_t Center;
	vec2_t Size;
	vec4_t Color;
	float Rotation;
	vec4_t TexRect;
};

static GpuSpriteBillboard g_GpuSpriteBillboards[MAX_SPRITES];
static GLuint g_GpuSpriteVAO = 0;
static GLuint g_GpuSpriteVBO = 0;

static const int GPU_SPRITE_TEXTURE_COUNT = 16;
static const int GPU_SPRITE_SUBTYPE_COUNT = 4;
static const int GPU_SPRITE_BUCKET_COUNT = GPU_SPRITE_TEXTURE_COUNT * GPU_SPRITE_SUBTYPE_COUNT;
static const int g_GpuSpriteTextures[GPU_SPRITE_TEXTURE_COUNT] =
{
	BITMAP_LIGHT,
	BITMAP_LIGHT + 1,
	BITMAP_LIGHT + 2,
	BITMAP_SHINY,
	BITMAP_SHINY + 1,
	BITMAP_SHINY + 2,
	BITMAP_SHINY + 3,
	BITMAP_SHINY + 6,
	BITMAP_SPARK,
	BITMAP_SPARK + 1,
	BITMAP_FLARE,
	BITMAP_FLARE + 1,
	BITMAP_LIGHTNING,
	BITMAP_LIGHTNING + 1,
	BITMAP_PIN_LIGHT,
	BITMAP_MAGIC,
};
static int g_GpuSpriteBucketCounts[GPU_SPRITE_BUCKET_COUNT];
static int g_GpuSpriteBucketIndices[GPU_SPRITE_BUCKET_COUNT][MAX_SPRITES];

static bool EnsureGpuSpriteBuffers()
{
#ifdef SHADER_VERSION_TEST
	if (g_GpuSpriteVAO != 0 && g_GpuSpriteVBO != 0)
		return true;

	glGenVertexArrays(1, &g_GpuSpriteVAO);
	glGenBuffers(1, &g_GpuSpriteVBO);
	if (g_GpuSpriteVAO == 0 || g_GpuSpriteVBO == 0)
		return false;

	glBindVertexArray(g_GpuSpriteVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_GpuSpriteVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_GpuSpriteBillboards), NULL, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GpuSpriteBillboard), (const GLvoid*)offsetof(GpuSpriteBillboard, Center));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GpuSpriteBillboard), (const GLvoid*)offsetof(GpuSpriteBillboard, Size));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GpuSpriteBillboard), (const GLvoid*)offsetof(GpuSpriteBillboard, Color));
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GpuSpriteBillboard), (const GLvoid*)offsetof(GpuSpriteBillboard, Rotation));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(GpuSpriteBillboard), (const GLvoid*)offsetof(GpuSpriteBillboard, TexRect));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return true;
#else
	return false;
#endif // SHADER_VERSION_TEST
}

static bool ShouldRenderSpriteInPass(const OBJECT* o, BYTE byRenderOneMore)
{
	if (o == NULL || !o->Live)
		return false;

	if (byRenderOneMore == 1)
	{
		return o->Position[2] <= 350.0f;
	}
	else if (byRenderOneMore == 2)
	{
		return o->Position[2] > 300.0f;
	}

	return true;
}

static bool IsGpuBatchablePlayerSpriteOwner(const OBJECT* owner)
{
	if (owner == NULL)
		return false;

	if (owner->Kind == KIND_PLAYER && owner->Type == MODEL_PLAYER)
		return true;

	return owner->Owner != NULL
		&& owner->Owner->Kind == KIND_PLAYER
		&& owner->Owner->Type == MODEL_PLAYER;
}

static int GetGpuSpriteTextureSlot(int Type)
{
	for (int i = 0; i < GPU_SPRITE_TEXTURE_COUNT; ++i)
	{
		if (g_GpuSpriteTextures[i] == Type)
			return i;
	}
	return -1;
}

static int GetGpuSpriteBucketIndex(const OBJECT* o)
{
	if (o == NULL || o->Type == BITMAP_FORMATION_MARK)
		return -1;

	if (!IsGpuBatchablePlayerSpriteOwner(o->Owner))
		return -1;

	if (o->SubType < 0 || o->SubType >= GPU_SPRITE_SUBTYPE_COUNT)
		return -1;

	const int textureSlot = GetGpuSpriteTextureSlot(o->Type);
	if (textureSlot < 0)
		return -1;

	return textureSlot * GPU_SPRITE_SUBTYPE_COUNT + o->SubType;
}

static bool CanGpuBatchTransientSprite(const OBJECT* o)
{
	return GetGpuSpriteBucketIndex(o) >= 0;
}

static void EnableGpuSpriteBlendState(int subType)
{
	switch (subType)
	{
	case 1:
		EnableAlphaBlendMinus();
		break;
	case 2:
		EnableAlphaTest();
		break;
	case 3:
		EnableAlphaBlend2();
		break;
	default:
		EnableAlphaBlend();
		break;
	}
}

static void CollectGpuSpriteBuckets(BYTE byRenderOneMore)
{
	memset(g_GpuSpriteBucketCounts, 0, sizeof(g_GpuSpriteBucketCounts));

	for (int i = 0; i < MAX_SPRITES; ++i)
	{
		OBJECT* o = &Sprites[i];
		if (!ShouldRenderSpriteInPass(o, byRenderOneMore))
			continue;

		const int bucketIndex = GetGpuSpriteBucketIndex(o);
		if (bucketIndex < 0)
			continue;

		const int bucketOffset = g_GpuSpriteBucketCounts[bucketIndex]++;
		g_GpuSpriteBucketIndices[bucketIndex][bucketOffset] = i;
	}
}

static bool RenderGpuSpriteBucket(int bucketIndex, BYTE byRenderOneMore)
{
#ifdef SHADER_VERSION_TEST
	if (bucketIndex < 0 || bucketIndex >= GPU_SPRITE_BUCKET_COUNT)
		return false;

	const int count = g_GpuSpriteBucketCounts[bucketIndex];
	if (count <= 0)
		return true;

	const int texture = g_GpuSpriteTextures[bucketIndex / GPU_SPRITE_SUBTYPE_COUNT];
	const int subType = bucketIndex % GPU_SPRITE_SUBTYPE_COUNT;
	BITMAP_t* pBitmap = Bitmaps.GetTexture(texture);
	for (int i = 0; i < count; ++i)
	{
		OBJECT* o = &Sprites[g_GpuSpriteBucketIndices[bucketIndex][i]];

		if (o->Visible)
		{
			o->AnimationFrame += 0.1f;
			if (o->AnimationFrame > 1.0f)
				o->AnimationFrame = 1.0f;
		}
		else
		{
			o->AnimationFrame -= 0.1f;
			if (o->AnimationFrame < 0.2f)
				o->AnimationFrame = 0.2f;
		}

		const float scale = o->AnimationFrame * o->Scale;
		GpuSpriteBillboard& billboard = g_GpuSpriteBillboards[i];
		VectorCopy(o->Position, billboard.Center);
		TEXCOORD(billboard.Size, pBitmap->Width * scale, pBitmap->Height * scale);
		VectorCopy(o->Light, billboard.Color);
		billboard.Color[3] = (pBitmap->Components == 3) ? 1.0f : o->Light[0];
		billboard.Rotation = o->Angle[2];
		TEXCOORD(billboard.TexRect, 0.0f, 0.0f);
		billboard.TexRect[2] = 1.0f;
		billboard.TexRect[3] = 1.0f;
	}

	g_EffectRenderPerfStats.SpriteGpuPass++;
	g_EffectRenderPerfStats.SpriteGpuRender += count;

	EnableGpuSpriteBlendState(subType);
	ZzzGpuAssistResetState();
	const GLuint program = gShaderGL->GetShaderParticleId();
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "texture1"), 0);
	BindTexture(texture);

	glBindVertexArray(g_GpuSpriteVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_GpuSpriteVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(GpuSpriteBillboard), g_GpuSpriteBillboards);
	glDrawArrays(GL_POINTS, 0, count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	if (byRenderOneMore == 0 || byRenderOneMore == 2)
	{
		for (int i = 0; i < count; ++i)
		{
			Sprites[g_GpuSpriteBucketIndices[bucketIndex][i]].Live = false;
		}
	}
	return true;
#else
	UNREFERENCED_PARAMETER(bucketIndex);
	UNREFERENCED_PARAMETER(byRenderOneMore);
	return false;
#endif // SHADER_VERSION_TEST
}

static bool RenderGpuTransientSprites(BYTE byRenderOneMore)
{
#ifdef SHADER_VERSION_TEST
	if (!gShaderGL->IsGpuAssistEnabled() || !gShaderGL->CheckedShader(CShaderGL::SHADER_PARTICLE))
		return false;

	if (!EnsureGpuSpriteBuffers())
		return false;

	CollectGpuSpriteBuckets(byRenderOneMore);
	for (int bucketIndex = 0; bucketIndex < GPU_SPRITE_BUCKET_COUNT; ++bucketIndex)
	{
		RenderGpuSpriteBucket(bucketIndex, byRenderOneMore);
	}
	return true;
#else
	UNREFERENCED_PARAMETER(byRenderOneMore);
	return false;
#endif // SHADER_VERSION_TEST
}

static const int SPRITE_HERO_WING_RESERVE = 256;
static const float HERO_WING_PRIORITY_RADIUS = 220.f;

static bool IsValidSpriteVec3(const vec3_t position)
{
	return position != NULL && (UINT_PTR)position >= 0x10000;
}

static OBJECT* GetLiveHeroSpriteObject()
{
	if (SceneFlag != MAIN_SCENE || Hero == NULL || !CGMCharacter::IsInitialized())
		return NULL;

	const int heroIndex = gmCharacters->GetCharacterIndex(Hero);
	if (heroIndex < 0 || heroIndex >= MAX_CHARACTERS_CLIENT)
		return NULL;

	CHARACTER* hero = gmCharacters->GetCharacter(heroIndex);
	if (hero != Hero)
		return NULL;

	OBJECT* heroObject = &hero->Object;
	if (!heroObject->Live || heroObject->Type != MODEL_PLAYER)
		return NULL;

	return heroObject;
}

static bool IsNearHeroSpritePosition(const vec3_t position, const OBJECT* heroObject)
{
	if (!IsValidSpriteVec3(position) || heroObject == NULL)
		return false;

	const float dx = position[0] - heroObject->Position[0];
	const float dy = position[1] - heroObject->Position[1];
	const float dz = position[2] - heroObject->Position[2];
	return (dx * dx + dy * dy + dz * dz) <= (HERO_WING_PRIORITY_RADIUS * HERO_WING_PRIORITY_RADIUS);
}

static bool IsHeroOrWingSpriteOwner(OBJECT* owner, const OBJECT* heroObject)
{
	if (owner == NULL || heroObject == NULL)
		return false;

	if (owner == heroObject)
		return true;

	if (owner->Owner == heroObject)
		return true;

	return owner->Type >= MODEL_WING && owner->Type < MODEL_HELPER
		&& IsNearHeroSpritePosition(owner->Position, heroObject);
}

static bool IsWingGlowSpriteType(int Type, int SubType)
{
	switch (Type)
	{
	case BITMAP_FLARE_BLUE:
	case BITMAP_LIGHT:
	case BITMAP_BLUE_BLUR:
		return true;
	case BITMAP_SHINY + 1:
		return SubType == 0 || SubType == 3;
	case BITMAP_FLARE:
	case BITMAP_FLARE + 1:
		return SubType == 0 || SubType == 3 || SubType == 6 || SubType == 8;
	default:
		return false;
	}
}

static bool IsPriorityWingSprite(int Type, int SubType, const vec3_t Position, OBJECT* Owner)
{
	const OBJECT* heroObject = GetLiveHeroSpriteObject();
	if (heroObject == NULL)
		return false;

	return IsHeroOrWingSpriteOwner(Owner, heroObject)
		|| (IsWingGlowSpriteType(Type, SubType) && IsNearHeroSpritePosition(Position, heroObject));
}

extern int g_iCrowdVisiblePlayerCount;
extern unsigned int g_uiCrowdAnimationFrameId;
extern int g_iRemotePlayerSpriteEffectBudget;
extern int g_iRemotePlayerSpriteEffectUsed;

static bool IsRemotePlayerEffectOwner(OBJECT* owner)
{
	if (owner == NULL || Hero == NULL)
		return false;

	if (owner == &Hero->Object || owner->Owner == &Hero->Object)
		return false;

	if (owner->Kind == KIND_PLAYER && owner->Type == MODEL_PLAYER)
		return true;

	return owner->Owner != NULL
		&& owner->Owner != &Hero->Object
		&& owner->Owner->Kind == KIND_PLAYER
		&& owner->Owner->Type == MODEL_PLAYER;
}

static bool IsThrottleableRemoteSprite(int Type)
{
	switch (Type)
	{
	case BITMAP_LIGHT:
	case BITMAP_LIGHT + 1:
	case BITMAP_LIGHT + 2:
	case BITMAP_SHINY:
	case BITMAP_SHINY + 1:
	case BITMAP_SHINY + 2:
	case BITMAP_SHINY + 3:
	case BITMAP_SPARK:
	case BITMAP_SPARK + 1:
	case BITMAP_FLARE:
	case BITMAP_FLARE + 1:
	case BITMAP_LIGHTNING:
	case BITMAP_LIGHTNING + 1:
	case BITMAP_PIN_LIGHT:
		return true;
	default:
		return false;
	}
}

static bool ShouldThrottleRemoteSprite(int Type, int SubType, OBJECT* Owner)
{
	UNREFERENCED_PARAMETER(SubType);

	if (!IsRemotePlayerEffectOwner(Owner) || !IsThrottleableRemoteSprite(Type))
		return false;

	if (g_iRemotePlayerSpriteEffectBudget < 0)
		return false;

	if (g_iRemotePlayerSpriteEffectUsed >= g_iRemotePlayerSpriteEffectBudget)
		return true;

	++g_iRemotePlayerSpriteEffectUsed;
	return false;
}

int CreateSprite(int Type, vec3_t Position, float Scale, vec3_t Light, OBJECT* Owner, float Rotation, int SubType)
{
	g_EffectRenderPerfStats.SpriteCreate++;
	if (!IsValidSpriteVec3(Position) || !IsValidSpriteVec3(Light))
		return false;

	if (ShouldThrottleRemoteSprite(Type, SubType, Owner))
	{
		g_EffectRenderPerfStats.SpriteThrottled++;
		return false;
	}

	const bool priorityEffect = IsPriorityWingSprite(Type, SubType, Position, Owner);
	const int searchLimit = priorityEffect ? MAX_SPRITES : (MAX_SPRITES - SPRITE_HERO_WING_RESERVE);

	for (int i = 0; i < searchLimit; i++)
	{
		OBJECT* o = &Sprites[i];
		if (!o->Live)
		{
			o->Live = true;
			g_EffectRenderPerfStats.SpriteCreated++;
			o->Type = Type;
			o->SubType = SubType;
			o->Owner = Owner;
			o->AnimationFrame = 1.f;
			o->Scale = Scale;
			o->Angle[2] = Rotation;
			VectorCopy(Position, o->Position);
			VectorCopy(Position, o->StartPosition);
			VectorCopy(Light, o->Light);
			return i;
		}
	}
	return false;
}

void RenderSprite(OBJECT* o, OBJECT* Owner)
{
	if (o->Visible)
	{
		o->AnimationFrame += 0.1f;
		if (o->AnimationFrame > 1.f)
		{
			o->AnimationFrame = 1.f;
		}
	}
	else
	{
		o->AnimationFrame -= 0.1f;
		if (o->AnimationFrame < 0.2f)
		{
			o->AnimationFrame = 0.2f;
		}
	}

	float Scale = o->AnimationFrame * o->Scale;
	BITMAP_t* pBitmap = Bitmaps.GetTexture(o->Type);
	float Width = pBitmap->Width * Scale;
	float Height = pBitmap->Height * Scale;

	if (o->Type == BITMAP_FORMATION_MARK)
	{
		float u = 0.0f, v = 0.0f, uw, vw;
		uw = 0.33f; vw = 0.33f;
		switch (o->SubType)
		{
		case 0:
			u = 0.f; v = 0.f;
			break;

		case 1:
			u = 0.33f; v = 0.f;
			break;

		case 2:
			u = 0.66f; v = 0.f;
			break;

		case 3:
			u = 0.f; v = 0.33f;
			break;

		case 4:
			u = 0.33f; v = 0.33f;
			break;

		case 5:
			u = 0.66f; v = 0.33f;
			break;

		case 6:
			u = 0.f; v = 0.66f;
			break;

		case 7:
			u = 0.33f; v = 0.66f;
			break;
		}
		RenderSprite(o->Type, o->Position, 64, 64, o->Light, o->Angle[2], u, v, uw, vw);
	}
	else
	{
		RenderSprite(o->Type, o->Position, Width, Height, o->Light, o->Angle[2]);
	}
}

void RenderSprites(BYTE byRenderOneMore)
{
	if (g_pOption->GetRenderEffect() == false)
	{
		return;
	}
	const bool gpuTransientSpritesRendered = RenderGpuTransientSprites(byRenderOneMore);

	for (int i = 0; i < MAX_SPRITES; i++)
	{
		OBJECT* o = &Sprites[i];
		if (!ShouldRenderSpriteInPass(o, byRenderOneMore))
		{
			if (byRenderOneMore == 2 && o->Live)
				o->Live = false;
			continue;
		}

		if (gpuTransientSpritesRendered && CanGpuBatchTransientSprite(o))
			continue;

		if (o->Live)
		{
			g_EffectRenderPerfStats.SpriteLive++;
			if (o->Type == BITMAP_FORMATION_MARK)
			{
				EnableAlphaTest();
			}
			else if (o->SubType == 0)
			{
				EnableAlphaBlend();
			}
			else if (o->SubType == 1)
			{
				EnableAlphaBlendMinus();
			}
			else if (o->SubType == 2)
			{
				EnableAlphaTest();
			}
			else if (o->SubType == 3)
			{
				EnableAlphaBlend2();
			}
			g_EffectRenderPerfStats.SpriteCpuRender++;
			ZzzPerfAddTopType(o->Type, g_EffectRenderPerfStats.SpriteCpuType, g_EffectRenderPerfStats.SpriteCpuTypeCount);
			RenderSprite(o, o->Owner);

			if (byRenderOneMore == 0 || byRenderOneMore == 2)
			{
				o->Live = false;
			}
		}
	}
}

void CheckSprites()
{
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		OBJECT* o = &Sprites[i];
		if (o->Live)
		{
			o->Visible = true;
		}
	}
}
