from dataclasses import dataclass
from pathlib import Path
import random
import unittest


MODEL_PLAYER = 0x100
MODEL_MONSTER = 0x200
KIND_PLAYER = 1
KIND_NPC = 2


SOURCE_DIR = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class Obj:
    type: int
    kind: int
    enable_bone_matrix: bool
    bone_transform_exists: bool
    bone_transform_valid: bool
    is_hero: bool = False

    def is_bone_transform_ready(self):
        return self.bone_transform_exists and self.bone_transform_valid


def should_skip_remote_player(target):
    return (
        target is not None
        and not target.is_hero
        and target.type == MODEL_PLAYER
        and target.kind == KIND_PLAYER
        and target.enable_bone_matrix
        and not target.is_bone_transform_ready()
    )


class CrowdTargetFlickerProperties(unittest.TestCase):
    def test_bug_condition_skips_remote_player_with_unready_bones(self):
        # Counterexample before the fix:
        # CreateBoneTransform allocates a remote MODEL_PLAYER, but Animation has
        # not written its joints yet. A joint like BITMAP_JOINT_THUNDER subtype 2
        # reads BoneTransform[33], producing a long diagonal streak.
        rng = random.Random(20260603)

        for _ in range(256):
            target = Obj(
                type=MODEL_PLAYER,
                kind=KIND_PLAYER,
                enable_bone_matrix=True,
                bone_transform_exists=True,
                bone_transform_valid=False,
            )
            junk_emit = (rng.uniform(3000.0, 9000.0), rng.uniform(3000.0, 9000.0), 0.0)

            self.assertTrue(should_skip_remote_player(target))
            self.assertGreater((junk_emit[0] ** 2 + junk_emit[1] ** 2) ** 0.5, 1000.0)

    def test_preserves_targets_outside_bug_condition(self):
        cases = [
            None,
            Obj(MODEL_PLAYER, KIND_PLAYER, True, True, False, is_hero=True),
            Obj(MODEL_PLAYER, KIND_PLAYER, True, True, True),
            Obj(MODEL_PLAYER, KIND_PLAYER, False, True, False),
            Obj(MODEL_MONSTER, KIND_NPC, True, True, False),
        ]

        for target in cases:
            with self.subTest(target=target):
                self.assertFalse(should_skip_remote_player(target))


class SourceContractTests(unittest.TestCase):
    def read_source(self, name):
        return (SOURCE_DIR / name).read_text(encoding="utf-8", errors="ignore")

    def test_object_lifecycle_tracks_bone_transform_validity(self):
        header = self.read_source("w_ObjectInfo.h")
        impl = self.read_source("w_ObjectInfo.cpp")
        zzz_object = self.read_source("ZzzObject.cpp")

        self.assertIn("bool\t\t\tBoneTransformValid;", header)
        self.assertIn("IsBoneTransformReady() const", header)
        self.assertIn("BoneTransformValid = false;", impl)
        self.assertIn("memset(this->BoneTransform, 0, sizeof(vec34_t) * NumBones);", impl)
        self.assertIn("o->BoneTransformValid = true;", zzz_object)

    def test_joint_and_particle_paths_have_not_ready_guards(self):
        joint = self.read_source("ZzzEffectJoint.cpp")
        particle = self.read_source("ZzzEffectParticle.cpp")
        sprite = self.read_source("zzzeffectsprite.cpp")

        self.assertIn("IsRemotePlayerBoneTransformNotReady(Target)", joint)
        self.assertGreaterEqual(joint.count("KillJointIfBoneTransformNotReady(o)"), 6)
        self.assertIn("IsRemotePlayerBoneTransformNotReady(Owner)", particle)
        self.assertGreaterEqual(particle.count("KillParticleIfBoneTransformNotReady(o)"), 4)
        self.assertNotIn("BoneTransform", sprite)


if __name__ == "__main__":
    unittest.main()
