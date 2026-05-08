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

extern int g_DebugPartFxScopeDepth;
extern int g_DebugPlayerPartFxScopeDepth;
extern void DebugAddPartFxSpriteCount();
extern void DebugAddPlayerPartFxSpriteCount();
extern void DebugAddRenderSpriteCount();
extern void DebugAddRenderSpriteStateChange();
extern void DebugAddRenderSpriteTextureChange();
extern void DebugAddRenderSpriteDrawCall();


OBJECT Sprites[MAX_SPRITES];

static int g_iSpriteFreeCursor = 0;
static bool g_bSpritePoolFull = false;
static vec3_t s_SpriteBatchVertices[MAX_SPRITES * 4];
static vec2_t s_SpriteBatchTexCoords[MAX_SPRITES * 4];
static vec4_t s_SpriteBatchColors[MAX_SPRITES * 4];
static const int SPRITE_ALPHA_MODE_COUNT = 4;
static const int SPRITE_TEXTURE_BUCKETS = 1024;
static int s_SpriteBucketHead[SPRITE_ALPHA_MODE_COUNT][SPRITE_TEXTURE_BUCKETS];
static int s_SpriteBucketTail[SPRITE_ALPHA_MODE_COUNT][SPRITE_TEXTURE_BUCKETS];
static int s_SpriteBucketNext[MAX_SPRITES];
static int s_SpriteBucketTexture[SPRITE_ALPHA_MODE_COUNT][SPRITE_TEXTURE_BUCKETS];
static int g_iHeroSpritePriorityDepth = 0;

static bool IsHeroSpriteOwner(OBJECT* Owner)
{
	if (Hero == NULL)
	{
		return false;
	}

	OBJECT* current = Owner;
	for (int i = 0; i < 4 && current != NULL; ++i)
	{
		if (current == &Hero->Object)
		{
			return true;
		}
		current = current->Owner;
	}

	return false;
}

static bool IsHeroSpriteRequest(OBJECT* Owner)
{
	return (g_iHeroSpritePriorityDepth > 0 || IsHeroSpriteOwner(Owner));
}

static void MarkSpriteSlotFree(int index)
{
	if (index >= 0 && index < MAX_SPRITES && index < g_iSpriteFreeCursor)
	{
		g_iSpriteFreeCursor = index;
	}
	g_bSpritePoolFull = false;
}

bool IsSpritePoolFull()
{
	return g_bSpritePoolFull;
}

bool IsSpritePoolFullForOwner(OBJECT* Owner)
{
	return g_bSpritePoolFull && !IsHeroSpriteRequest(Owner);
}

bool IsHeroEffectOwner(OBJECT* Owner)
{
	return IsHeroSpriteOwner(Owner);
}

bool IsHeroEffectPriorityActive()
{
	return g_iHeroSpritePriorityDepth > 0;
}

void BeginHeroSpritePriority()
{
	++g_iHeroSpritePriorityDepth;
}

void EndHeroSpritePriority()
{
	if (g_iHeroSpritePriorityDepth > 0)
	{
		--g_iHeroSpritePriorityDepth;
	}
}

static int UseSpriteSlot(int index, int Type, vec3_t Position, float Scale, vec3_t Light, OBJECT* Owner, float Rotation, int SubType)
{
	OBJECT* o = &Sprites[index];
	OBJECT* spriteOwner = (g_iHeroSpritePriorityDepth > 0 && Hero != NULL) ? &Hero->Object : Owner;
	o->Live = true;
	o->Type = Type;
	o->SubType = SubType;
	o->Owner = spriteOwner;
	o->AnimationFrame = 1.f;
	o->Scale = Scale;
	o->Angle[2] = Rotation;
	VectorCopy(Position, o->Position);
	VectorCopy(Position, o->StartPosition);
	VectorCopy(Light, o->Light);
	g_iSpriteFreeCursor = index + 1;
	if (g_iSpriteFreeCursor >= MAX_SPRITES)
	{
		g_iSpriteFreeCursor = 0;
	}
	return index;
}

int CreateSprite(int Type, vec3_t Position, float Scale, vec3_t Light, OBJECT* Owner, float Rotation, int SubType)
{
	if (g_DebugPartFxScopeDepth > 0)
	{
		DebugAddPartFxSpriteCount();
		if (g_DebugPlayerPartFxScopeDepth > 0)
		{
			DebugAddPlayerPartFxSpriteCount();
		}
	}

	if (g_bSpritePoolFull)
	{
		if (g_iSpriteFreeCursor >= 0 && g_iSpriteFreeCursor < MAX_SPRITES && !Sprites[g_iSpriteFreeCursor].Live)
		{
			g_bSpritePoolFull = false;
		}
		else if (IsHeroSpriteRequest(Owner))
		{
			for (int i = 0; i < MAX_SPRITES; ++i)
			{
				if (Sprites[i].Live && !IsHeroSpriteOwner(Sprites[i].Owner))
				{
					Sprites[i].Live = false;
					g_iSpriteFreeCursor = i;
					g_bSpritePoolFull = false;
					break;
				}
			}

			if (g_bSpritePoolFull)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	if (g_iSpriteFreeCursor < 0 || g_iSpriteFreeCursor >= MAX_SPRITES)
	{
		g_iSpriteFreeCursor = 0;
	}

	for (int pass = 0; pass < 2; ++pass)
	{
		const int begin = (pass == 0) ? g_iSpriteFreeCursor : 0;
		const int end = (pass == 0) ? MAX_SPRITES : g_iSpriteFreeCursor;

		for (int i = begin; i < end; ++i)
		{
			OBJECT* o = &Sprites[i];
			if (!o->Live)
			{
				return UseSpriteSlot(i, Type, Position, Scale, Light, Owner, Rotation, SubType);
			}
		}
	}

	if (IsHeroSpriteRequest(Owner))
	{
		for (int i = 0; i < MAX_SPRITES; ++i)
		{
			if (Sprites[i].Live && !IsHeroSpriteOwner(Sprites[i].Owner))
			{
				return UseSpriteSlot(i, Type, Position, Scale, Light, Owner, Rotation, SubType);
			}
		}
	}

	g_bSpritePoolFull = true;
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

static int GetSpriteAlphaMode(const OBJECT* o)
{
	if (o->Type == BITMAP_FORMATION_MARK)
	{
		return 2;
	}
	if (o->SubType == 0)
	{
		return 0;
	}
	if (o->SubType == 1)
	{
		return 1;
	}
	if (o->SubType == 2)
	{
		return 2;
	}
	if (o->SubType == 3)
	{
		return 3;
	}
	return 0;
}

static void ClearSpriteRenderBuckets()
{
	for (int alpha = 0; alpha < SPRITE_ALPHA_MODE_COUNT; ++alpha)
	{
		for (int bucket = 0; bucket < SPRITE_TEXTURE_BUCKETS; ++bucket)
		{
			s_SpriteBucketHead[alpha][bucket] = -1;
			s_SpriteBucketTail[alpha][bucket] = -1;
			s_SpriteBucketTexture[alpha][bucket] = -1;
		}
	}
}

static int FindSpriteTextureBucket(int alphaMode, int texture)
{
	int bucket = texture & (SPRITE_TEXTURE_BUCKETS - 1);
	for (int probe = 0; probe < SPRITE_TEXTURE_BUCKETS; ++probe)
	{
		const int currentTexture = s_SpriteBucketTexture[alphaMode][bucket];
		if (currentTexture == texture || currentTexture == -1)
		{
			if (currentTexture == -1)
			{
				s_SpriteBucketTexture[alphaMode][bucket] = texture;
			}
			return bucket;
		}
		bucket = (bucket + 1) & (SPRITE_TEXTURE_BUCKETS - 1);
	}
	return texture & (SPRITE_TEXTURE_BUCKETS - 1);
}

static void AddSpriteRenderBucket(int alphaMode, int texture, int index)
{
	if (alphaMode < 0 || alphaMode >= SPRITE_ALPHA_MODE_COUNT)
	{
		return;
	}

	const int bucket = FindSpriteTextureBucket(alphaMode, texture);
	s_SpriteBucketNext[index] = -1;
	if (s_SpriteBucketHead[alphaMode][bucket] == -1)
	{
		s_SpriteBucketHead[alphaMode][bucket] = index;
		s_SpriteBucketTail[alphaMode][bucket] = index;
	}
	else
	{
		s_SpriteBucketNext[s_SpriteBucketTail[alphaMode][bucket]] = index;
		s_SpriteBucketTail[alphaMode][bucket] = index;
	}
}
static void FillSpriteBatchVertex(OBJECT* o, int vertexOffset)
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
	float Width = pBitmap->Width * Scale * 0.5f;
	float Height = pBitmap->Height * Scale * 0.5f;

	vec3_t p2;
	VectorTransform(o->Position, CameraMatrix, p2);
	float x = p2[0];
	float y = p2[1];
	float z = p2[2];

	vec3_t p[4];
	if (o->Angle[2] == 0)
	{
		Vector(x - Width, y - Height, z, p[0]);
		Vector(x + Width, y - Height, z, p[1]);
		Vector(x + Width, y + Height, z, p[2]);
		Vector(x - Width, y + Height, z, p[3]);
	}
	else
	{
		vec3_t local[4];
		Vector(-Width, -Height, z, local[0]);
		Vector(Width, -Height, z, local[1]);
		Vector(Width, Height, z, local[2]);
		Vector(-Width, Height, z, local[3]);
		vec3_t Angle;
		Vector(0.f, 0.f, o->Angle[2], Angle);
		float Matrix[3][4];
		AngleMatrix(Angle, Matrix);

		for (int i = 0; i < 4; i++)
		{
			VectorRotate(local[i], Matrix, p[i]);
			p[i][0] += x;
			p[i][1] += y;
		}
	}

	float u = 0.f;
	float v = 0.f;
	float uWidth = 1.f;
	float vHeight = 1.f;
	if (o->Type == BITMAP_FORMATION_MARK)
	{
		uWidth = 0.33f;
		vHeight = 0.33f;
		switch (o->SubType)
		{
		case 1: u = 0.33f; v = 0.f; break;
		case 2: u = 0.66f; v = 0.f; break;
		case 3: u = 0.f; v = 0.33f; break;
		case 4: u = 0.33f; v = 0.33f; break;
		case 5: u = 0.66f; v = 0.33f; break;
		case 6: u = 0.f; v = 0.66f; break;
		case 7: u = 0.33f; v = 0.66f; break;
		}
	}

	VectorCopy(p[0], s_SpriteBatchVertices[vertexOffset + 0]);
	VectorCopy(p[1], s_SpriteBatchVertices[vertexOffset + 1]);
	VectorCopy(p[2], s_SpriteBatchVertices[vertexOffset + 2]);
	VectorCopy(p[3], s_SpriteBatchVertices[vertexOffset + 3]);

	TEXCOORD(s_SpriteBatchTexCoords[vertexOffset + 3], u, v);
	TEXCOORD(s_SpriteBatchTexCoords[vertexOffset + 2], u + uWidth, v);
	TEXCOORD(s_SpriteBatchTexCoords[vertexOffset + 1], u + uWidth, v + vHeight);
	TEXCOORD(s_SpriteBatchTexCoords[vertexOffset + 0], u, v + vHeight);

	for (int i = 0; i < 4; i++)
	{
		VectorCopy(o->Light, s_SpriteBatchColors[vertexOffset + i]);
		if (Bitmaps[o->Type].Components == 3)
		{
			s_SpriteBatchColors[vertexOffset + i][3] = 1.f;
		}
		else
		{
			if (o->Type == BITMAP_BLOOD + 1 || o->Type == BITMAP_FONT_HIT)
			{
				s_SpriteBatchColors[vertexOffset + i][3] = 1.f;
			}
			else
			{
				s_SpriteBatchColors[vertexOffset + i][3] = o->Light[0];
			}
		}
	}
}

static void RenderSpriteBatchRange(int texture, int head)
{
	if (head < 0)
	{
		return;
	}

	int vertexCount = 0;
	for (int index = head; index >= 0; index = s_SpriteBucketNext[index])
	{
		OBJECT* o = &Sprites[index];
		if (!o->Live)
		{
			continue;
		}
		FillSpriteBatchVertex(o, vertexCount);
		vertexCount += 4;
	}

	if (vertexCount <= 0)
	{
		return;
	}

	BindTexture(texture);
	glVertexPointer(3, GL_FLOAT, 0, s_SpriteBatchVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, s_SpriteBatchTexCoords);
	glColorPointer(4, GL_FLOAT, 0, s_SpriteBatchColors);
	glDrawArrays(GL_QUADS, 0, vertexCount);
	DebugAddRenderSpriteDrawCall();
}
void RenderSprites(BYTE byRenderOneMore)
{
	if (g_pOption->GetRenderEffect() == false)
	{
		return;
	}

	ClearSpriteRenderBuckets();
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		OBJECT* o = &Sprites[i];
		if (byRenderOneMore == 1)
		{
			if (o->Position[2] > 350.f)
			{
				continue;
			}
		}
		else if (byRenderOneMore == 2)
		{
			if (o->Position[2] <= 300.f)
			{
				o->Live = false;
				MarkSpriteSlotFree(i);
				continue;
			}
		}

		if (o->Live)
		{
			AddSpriteRenderBucket(GetSpriteAlphaMode(o), o->Type, i);
		}
	}

	BeginSpriteRenderBatch();
	for (int alphaMode = 0; alphaMode < SPRITE_ALPHA_MODE_COUNT; ++alphaMode)
	{
		bool alphaModeEnabled = false;
		for (int bucket = 0; bucket < SPRITE_TEXTURE_BUCKETS; ++bucket)
		{
			const int head = s_SpriteBucketHead[alphaMode][bucket];
			if (head < 0)
			{
				continue;
			}

			if (!alphaModeEnabled)
			{
				switch (alphaMode)
				{
				case 0:
					EnableAlphaBlend();
					break;
				case 1:
					EnableAlphaBlendMinus();
					break;
				case 2:
					EnableAlphaTest();
					break;
				case 3:
					EnableAlphaBlend2();
					break;
				}
				DebugAddRenderSpriteStateChange();
				alphaModeEnabled = true;
			}

			DebugAddRenderSpriteTextureChange();
			for (int index = head; index >= 0; index = s_SpriteBucketNext[index])
			{
				DebugAddRenderSpriteCount();
			}
			RenderSpriteBatchRange(s_SpriteBucketTexture[alphaMode][bucket], head);

			if (byRenderOneMore == 0 || byRenderOneMore == 2)
			{
				for (int index = head; index >= 0;)
				{
					const int next = s_SpriteBucketNext[index];
					Sprites[index].Live = false;
					MarkSpriteSlotFree(index);
					index = next;
				}
			}
		}
	}
	EndSpriteRenderBatch();
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
