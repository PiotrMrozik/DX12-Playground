#include <gtest/gtest.h>
#include <vector>

#include <ECS/World.h>
#include <ECS/System/System.h>

// -----------------------------------------------------------------------
// Local test-only components - plain POD, no engine dependencies.
// -----------------------------------------------------------------------
namespace {

struct PosComp { float x = 0.f, y = 0.f; };
struct VelComp { float vx = 0.f, vy = 0.f; };
struct TagComp { int id = 0; };

// Minimal concrete systems used only to verify entity tracking.
class MovementSystem : public System {};
class RenderSystem   : public System {};
class PhysicsSystem  : public System {};

} // namespace

// -----------------------------------------------------------------------
// Fixture - registers three components once per test.
// -----------------------------------------------------------------------
class WorldTest : public ::testing::Test
{
protected:
    World world;

    void SetUp() override
    {
        world.RegisterComponent<PosComp>();
        world.RegisterComponent<VelComp>();
        world.RegisterComponent<TagComp>();
    }
};

// -----------------------------------------------------------------------
// Entity creation / destruction
// -----------------------------------------------------------------------

TEST_F(WorldTest, CreateEntity_ReturnsValidId)
{
    EXPECT_NE(world.CreateEntity(), INVALID_ENTITY);
}

TEST_F(WorldTest, CreateEntity_TwoCalls_UniqueIds)
{
    Entity e1 = world.CreateEntity();
    Entity e2 = world.CreateEntity();
    EXPECT_NE(e1, e2);
}

TEST_F(WorldTest, DestroyAndRecreate_ReusesSameId)
{
    // Exhaust the pool so the destroyed ID is the only free slot,
    // making the next CreateEntity deterministically return it.
    std::vector<Entity> all;
    all.reserve(MAX_ENTITIES);
    for (std::uint32_t i = 0; i < MAX_ENTITIES; ++i)
        all.push_back(world.CreateEntity());

    Entity target = all[0];
    world.DestroyEntity(target);

    EXPECT_EQ(world.CreateEntity(), target);
}

// -----------------------------------------------------------------------
// HasComponent
// -----------------------------------------------------------------------

TEST_F(WorldTest, HasComponent_BeforeAdd_ReturnsFalse)
{
    Entity e = world.CreateEntity();
    EXPECT_FALSE(world.HasComponent<PosComp>(e));
}

TEST_F(WorldTest, HasComponent_AfterAdd_ReturnsTrue)
{
    Entity e = world.CreateEntity();
    world.AddComponent<PosComp>(e, {});
    EXPECT_TRUE(world.HasComponent<PosComp>(e));
}

TEST_F(WorldTest, HasComponent_AfterRemove_ReturnsFalse)
{
    Entity e = world.CreateEntity();
    world.AddComponent<PosComp>(e, {});
    world.RemoveComponent<PosComp>(e);
    EXPECT_FALSE(world.HasComponent<PosComp>(e));
}

TEST_F(WorldTest, HasComponent_IndependentPerComponent)
{
    Entity e = world.CreateEntity();
    world.AddComponent<PosComp>(e, {});

    EXPECT_TRUE (world.HasComponent<PosComp>(e));
    EXPECT_FALSE(world.HasComponent<VelComp>(e));
}

// -----------------------------------------------------------------------
// GetComponent
// -----------------------------------------------------------------------

TEST_F(WorldTest, GetComponent_ReturnsCorrectValues)
{
    Entity e = world.CreateEntity();
    world.AddComponent<PosComp>(e, { 3.f, 7.f });

    auto& pos = world.GetComponent<PosComp>(e);
    EXPECT_FLOAT_EQ(pos.x, 3.f);
    EXPECT_FLOAT_EQ(pos.y, 7.f);
}

TEST_F(WorldTest, GetComponent_ReturnsLiveReference)
{
    Entity e = world.CreateEntity();
    world.AddComponent<TagComp>(e, { 42 });

    world.GetComponent<TagComp>(e).id = 99;

    EXPECT_EQ(world.GetComponent<TagComp>(e).id, 99);
}

// -----------------------------------------------------------------------
// System entity tracking - the core ECS contract:
//   An entity is in a system's set iff its signature is a superset of
//   the system's required signature.
// -----------------------------------------------------------------------

TEST_F(WorldTest, System_EntityAddedOnlyWhenAllRequiredComponentsPresent)
{
    auto sys = world.RegisterSystem<MovementSystem>();

    EntitySignature sig;
    sig.set(world.GetComponentType<PosComp>());
    sig.set(world.GetComponentType<VelComp>());
    world.SetSystemSignature<MovementSystem>(sig);

    Entity e = world.CreateEntity();

    // Only Position - should NOT be in the movement system yet.
    world.AddComponent<PosComp>(e, {});
    EXPECT_EQ(sys->entities.count(e), 0u);

    // Add Velocity - now the signature matches, entity enters the system.
    world.AddComponent<VelComp>(e, {});
    EXPECT_EQ(sys->entities.count(e), 1u);
}

TEST_F(WorldTest, System_EntityRemovedWhenRequiredComponentRemoved)
{
    auto sys = world.RegisterSystem<MovementSystem>();

    EntitySignature sig;
    sig.set(world.GetComponentType<PosComp>());
    world.SetSystemSignature<MovementSystem>(sig);

    Entity e = world.CreateEntity();
    world.AddComponent<PosComp>(e, {});
    ASSERT_EQ(sys->entities.count(e), 1u);

    world.RemoveComponent<PosComp>(e);
    EXPECT_EQ(sys->entities.count(e), 0u);
}

TEST_F(WorldTest, System_EntityRemovedOnDestroy)
{
    auto sys = world.RegisterSystem<MovementSystem>();

    EntitySignature sig;
    sig.set(world.GetComponentType<PosComp>());
    world.SetSystemSignature<MovementSystem>(sig);

    Entity e = world.CreateEntity();
    world.AddComponent<PosComp>(e, {});
    ASSERT_EQ(sys->entities.count(e), 1u);

    world.DestroyEntity(e);
    EXPECT_EQ(sys->entities.count(e), 0u);
}

TEST_F(WorldTest, MultipleSystems_EachTracksOnlyMatchingEntities)
{
    auto movSys = world.RegisterSystem<MovementSystem>();
    auto renSys = world.RegisterSystem<RenderSystem>();

    // Movement needs Pos + Vel; Render needs only Pos.
    EntitySignature movSig;
    movSig.set(world.GetComponentType<PosComp>());
    movSig.set(world.GetComponentType<VelComp>());
    world.SetSystemSignature<MovementSystem>(movSig);

    EntitySignature renSig;
    renSig.set(world.GetComponentType<PosComp>());
    world.SetSystemSignature<RenderSystem>(renSig);

    // e1: Pos only  - render only
    Entity e1 = world.CreateEntity();
    world.AddComponent<PosComp>(e1, {});

    // e2: Pos + Vel - both systems
    Entity e2 = world.CreateEntity();
    world.AddComponent<PosComp>(e2, {});
    world.AddComponent<VelComp>(e2, {});

    EXPECT_EQ(movSys->entities.count(e1), 0u);
    EXPECT_EQ(movSys->entities.count(e2), 1u);
    EXPECT_EQ(renSys->entities.count(e1), 1u);
    EXPECT_EQ(renSys->entities.count(e2), 1u);
}

TEST_F(WorldTest, SystemWithEmptySignature_TracksAllEntities)
{
    auto sys = world.RegisterSystem<PhysicsSystem>();
    // Empty signature - every entity matches.
    world.SetSystemSignature<PhysicsSystem>(EntitySignature{});

    Entity e1 = world.CreateEntity();
    Entity e2 = world.CreateEntity();
    world.AddComponent<PosComp>(e1, {});
    world.AddComponent<VelComp>(e2, {});

    EXPECT_EQ(sys->entities.count(e1), 1u);
    EXPECT_EQ(sys->entities.count(e2), 1u);
}

TEST_F(WorldTest, DestroyEntity_ComponentsAlsoRemoved)
{
    // Exhaust the pool so the destroyed ID is the only free slot,
    // guaranteeing the next CreateEntity returns the same ID.
    std::vector<Entity> all;
    all.reserve(MAX_ENTITIES);
    for (std::uint32_t i = 0; i < MAX_ENTITIES; ++i)
        all.push_back(world.CreateEntity());

    Entity target = all[0];
    world.AddComponent<PosComp>(target, { 5.f, 5.f });

    world.DestroyEntity(target);

    // Only one slot is free → must get target back.
    // Re-adding PosComp must not trigger the double-insert assert in ComponentArray.
    Entity same = world.CreateEntity();
    ASSERT_EQ(same, target);
    world.AddComponent<PosComp>(same, { 1.f, 2.f });
    EXPECT_FLOAT_EQ(world.GetComponent<PosComp>(same).x, 1.f);
}
