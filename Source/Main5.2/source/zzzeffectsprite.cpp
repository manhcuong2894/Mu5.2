///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
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
#include <cstddef>


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
static GLuint g_GpuSpriteVao = 0;
static GLuint g_GpuSpriteVbo = 0;
static size_t g_GpuSpriteVboBytes = 0;

static bool IsValidSpriteObjectPointer(const OBJECT* object)
{
	return object != NULL && (UINT_PTR)object >= 0x10000;
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
	if (!IsValidSpriteObjectPointer(owner))
		return false;

	if (owner->Kind == KIND_PLAYER && owner->Type == MODEL_PLAYER)
		return true;

	return IsValidSpriteObjectPointer(owner->Owner)
		&& owner->Owner->Kind == KIND_PLAYER
		&& owner->Owner->Type == MODEL_PLAYER;
}

static bool IsGpuBatchableTransientSpriteType(int Type)
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
	case BITMAP_SHINY + 6:
	case BITMAP_SPARK:
	case BITMAP_SPARK + 1:
	case BITMAP_FLARE:
	case BITMAP_FLARE + 1:
	case BITMAP_LIGHTNING:
	case BITMAP_LIGHTNING + 1:
	case BITMAP_PIN_LIGHT:
	case BITMAP_MAGIC:
	case BITMAP_FIRE_CURSEDLICH:
	case BITMAP_LIGHT_MARKS:
	case BITMAP_TARGET_POSITION_EFFECT1:
	case BITMAP_TARGET_POSITION_EFFECT2:
	case BITMAP_LIGHTMARKS:
	case BITMAP_LIGHTMARKS_FOREIGN:
	case BITMAP_TWLIGHT:
	case BITMAP_2LINE_GHOST:
	case BITMAP_LIGHT_RED:
	case BITMAP_GRA:
		return true;
	default:
		return false;
	}
}

static bool CanGpuBatchTransientSprite(const OBJECT* o)
{
	if (o == NULL || o->Type == BITMAP_FORMATION_MARK)
		return false;

	if (o->SubType < 0 || o->SubType > 3)
		return false;

	return IsGpuBatchableTransientSpriteType(o->Type);
}

static void UploadGpuSpriteBatch(int count)
{
	const size_t requiredBytes = (size_t)count * sizeof(GpuSpriteBillboard);
	if (g_GpuSpriteVao == 0)
	{
		glGenVertexArrays(1, &g_GpuSpriteVao);
	}
	if (g_GpuSpriteVbo == 0)
	{
		glGenBuffers(1, &g_GpuSpriteVbo);
	}

	glBindVertexArray(g_GpuSpriteVao);
	glBindBuffer(GL_ARRAY_BUFFER, g_GpuSpriteVbo);
	if (g_GpuSpriteVboBytes < requiredBytes)
	{
		glBufferData(GL_ARRAY_BUFFER, requiredBytes, g_GpuSpriteBillboards, GL_STREAM_DRAW);
		g_GpuSpriteVboBytes = requiredBytes;
	}
	else
	{
		glBufferSubData(GL_ARRAY_BUFFER, 0, requiredBytes, g_GpuSpriteBillboards);
	}

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GpuSpriteBillboard), (void*)offsetof(GpuSpriteBillboard, Center));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GpuSpriteBillboard), (void*)offsetof(GpuSpriteBillboard, Size));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GpuSpriteBillboard), (void*)offsetof(GpuSpriteBillboard, Color));
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GpuSpriteBillboard), (void*)offsetof(GpuSpriteBillboard, Rotation));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(GpuSpriteBillboard), (void*)offsetof(GpuSpriteBillboard, TexRect));
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

static bool RenderGpuSpriteTexture(int texture, int subType, BYTE byRenderOneMore)
{
#ifdef SHADER_VERSION_TEST
	if (!gShaderGL->IsGpuAssistEnabled() || !gShaderGL->CheckedShader(CShaderGL::SHADER_PARTICLE))
		return false;

	BITMAP_t* pBitmap = Bitmaps.GetTexture(texture);
	int count = 0;
	for (int i = 0; i < MAX_SPRITES; ++i)
	{
		OBJECT* o = &Sprites[i];
		if (!ShouldRenderSpriteInPass(o, byRenderOneMore) || !CanGpuBatchTransientSprite(o) ||
			o->Type != texture || o->SubType != subType)
		{
			continue;
		}

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
		GpuSpriteBillboard& billboard = g_GpuSpriteBillboards[count++];
		VectorCopy(o->Position, billboard.Center);
		TEXCOORD(billboard.Size, pBitmap->Width * scale, pBitmap->Height * scale);
		VectorCopy(o->Light, billboard.Color);
		billboard.Color[3] = (pBitmap->Components == 3) ? 1.0f : o->Light[0];
		billboard.Rotation = o->Angle[2];
		TEXCOORD(billboard.TexRect, 0.0f, 0.0f);
		billboard.TexRect[2] = 1.0f;
		billboard.TexRect[3] = 1.0f;
	}

	if (count <= 0)
		return true;


	g_EffectRenderPerfStats.SpriteGpuPass++;
	g_EffectRenderPerfStats.SpriteGpuRender += count;

	EnableGpuSpriteBlendState(subType);
	ZzzGpuAssistResetState();
	const GLuint program = gShaderGL->GetShaderParticleId();
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "texture1"), 0);
	BindTexture(texture);

	UploadGpuSpriteBatch(count);
	glDrawArrays(GL_POINTS, 0, count);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	for (int i = 0; i < MAX_SPRITES; ++i)
	{
		OBJECT* o = &Sprites[i];
		if (ShouldRenderSpriteInPass(o, byRenderOneMore) && CanGpuBatchTransientSprite(o) &&
			o->Type == texture && o->SubType == subType && (byRenderOneMore == 0 || byRenderOneMore == 2))
		{
			o->Live = false;
		}
	}
	return true;
#else
	UNREFERENCED_PARAMETER(texture);
	UNREFERENCED_PARAMETER(subType);
	UNREFERENCED_PARAMETER(byRenderOneMore);
	return false;
#endif // SHADER_VERSION_TEST
}

static bool RenderGpuTransientSprites(BYTE byRenderOneMore)
{
#ifdef SHADER_VERSION_TEST
	if (!gShaderGL->IsGpuAssistEnabled() || !gShaderGL->CheckedShader(CShaderGL::SHADER_PARTICLE))
		return false;

	struct SpriteBatchGroup
	{
		int Texture;
		int SubType;
	};

	SpriteBatchGroup groups[128];
	int groupCount = 0;
	bool groupOverflow = false;
	for (int i = 0; i < MAX_SPRITES; ++i)
	{
		OBJECT* o = &Sprites[i];
		if (!ShouldRenderSpriteInPass(o, byRenderOneMore) || !CanGpuBatchTransientSprite(o))
		{
			continue;
		}

		bool found = false;
		for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex)
		{
			if (groups[groupIndex].Texture == o->Type && groups[groupIndex].SubType == o->SubType)
			{
				found = true;
				break;
			}
		}

		if (!found && groupCount < (int)(sizeof(groups) / sizeof(groups[0])))
		{
			groups[groupCount].Texture = o->Type;
			groups[groupCount].SubType = o->SubType;
			++groupCount;
		}
		else if (!found)
		{
			groupOverflow = true;
			break;
		}
	}

	if (groupOverflow)
	{
		return false;
	}

	for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex)
	{
		RenderGpuSpriteTexture(groups[groupIndex].Texture, groups[groupIndex].SubType, byRenderOneMore);
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
	if (!IsValidSpriteObjectPointer(owner) || heroObject == NULL)
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
	if (!IsValidSpriteObjectPointer(owner) || Hero == NULL)
		return false;

	if (owner == &Hero->Object || owner->Owner == &Hero->Object)
		return false;

	if (owner->Kind == KIND_PLAYER && owner->Type == MODEL_PLAYER)
		return true;

	return IsValidSpriteObjectPointer(owner->Owner)
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
