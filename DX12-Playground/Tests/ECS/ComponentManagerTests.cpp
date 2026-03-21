#include <gtest/gtest.h>

#include <ECS/Component/ComponentManager.h>

// -----------------------------------------------------------------------
// Local test-only components — plain POD, no engine dependencies.
// Anonymous namespace prevents ODR conflicts with other translation units.
// -----------------------------------------------------------------------
namespace
{

struct Position
{
    float x = 0.f, y = 0.f, z = 0.f;
};
struct Velocity
{
    float vx = 0.f, vy = 0.f;
};
struct Health
{
    int hp = 0;
};

} // namespace

// -----------------------------------------------------------------------
// Fixture — registers three components once per test.
// -----------------------------------------------------------------------
class ComponentManagerTest : public ::testing::Test
{
protected:
    ComponentManager mgr;

    void SetUp() override
    {
        mgr.RegisterComponent<Position>();
        mgr.RegisterComponent<Velocity>();
        mgr.RegisterComponent<Health>();
    }
};

// -----------------------------------------------------------------------
// Component type IDs
// -----------------------------------------------------------------------

TEST_F(ComponentManagerTest, ComponentTypes_AreUnique)
{
    EXPECT_NE(mgr.GetComponentType<Position>(), mgr.GetComponentType<Velocity>());
    EXPECT_NE(mgr.GetComponentType<Velocity>(), mgr.GetComponentType<Health>());
    EXPECT_NE(mgr.GetComponentType<Position>(), mgr.GetComponentType<Health>());
}

TEST_F(ComponentManagerTest, GetComponentType_IsStable)
{
    ComponentType t1 = mgr.GetComponentType<Position>();
    ComponentType t2 = mgr.GetComponentType<Position>();
    EXPECT_EQ(t1, t2);
}

// -----------------------------------------------------------------------
// Add / Get
// -----------------------------------------------------------------------

TEST_F(ComponentManagerTest, AddAndGet_ReturnsCorrectData)
{
    Entity e = 0;
    mgr.AddComponent<Position>(e, {1.f, 2.f, 3.f});

    auto& pos = mgr.GetComponent<Position>(e);
    EXPECT_FLOAT_EQ(pos.x, 1.f);
    EXPECT_FLOAT_EQ(pos.y, 2.f);
    EXPECT_FLOAT_EQ(pos.z, 3.f);
}

TEST_F(ComponentManagerTest, GetComponent_ReturnsLiveReference_ModificationPersists)
{
    Entity e = 0;
    mgr.AddComponent<Health>(e, {100});

    mgr.GetComponent<Health>(e).hp -= 30;

    EXPECT_EQ(mgr.GetComponent<Health>(e).hp, 70);
}

TEST_F(ComponentManagerTest, MultipleEntities_IndependentData)
{
    mgr.AddComponent<Health>(0, {100});
    mgr.AddComponent<Health>(1, {50});

    EXPECT_EQ(mgr.GetComponent<Health>(0).hp, 100);
    EXPECT_EQ(mgr.GetComponent<Health>(1).hp, 50);
}

// -----------------------------------------------------------------------
// Remove — ComponentArray uses a swap-with-last strategy to stay packed.
// After removing entity N the remaining entities must still be reachable
// with correct data.
// -----------------------------------------------------------------------

TEST_F(ComponentManagerTest, RemoveMiddleEntity_OtherEntitiesAccessible)
{
    mgr.AddComponent<Position>(0, {1.f, 0.f, 0.f});
    mgr.AddComponent<Position>(1, {2.f, 0.f, 0.f});
    mgr.AddComponent<Position>(2, {3.f, 0.f, 0.f});

    mgr.RemoveComponent<Position>(1);

    // Both survivors must still be addressable with their original values.
    EXPECT_FLOAT_EQ(mgr.GetComponent<Position>(0).x, 1.f);
    EXPECT_FLOAT_EQ(mgr.GetComponent<Position>(2).x, 3.f);
}

TEST_F(ComponentManagerTest, RemoveFirstEntity_LastEntityAccessible)
{
    mgr.AddComponent<Position>(0, {10.f, 0.f, 0.f});
    mgr.AddComponent<Position>(1, {20.f, 0.f, 0.f});

    mgr.RemoveComponent<Position>(0);

    EXPECT_FLOAT_EQ(mgr.GetComponent<Position>(1).x, 20.f);
}

TEST_F(ComponentManagerTest, RemoveComponent_DoesNotAffectOtherComponentTypes)
{
    mgr.AddComponent<Position>(0, {5.f, 0.f, 0.f});
    mgr.AddComponent<Health>(0, {75});

    mgr.RemoveComponent<Position>(0);

    // Health on the same entity must survive.
    EXPECT_EQ(mgr.GetComponent<Health>(0).hp, 75);
}

// -----------------------------------------------------------------------
// EntityDestroyed — must clean up all component arrays for that entity.
// -----------------------------------------------------------------------

TEST_F(ComponentManagerTest, EntityDestroyed_AllowsReinsertion)
{
    Entity e = 0;
    mgr.AddComponent<Position>(e, {1.f, 0.f, 0.f});
    mgr.AddComponent<Health>(e, {80});

    mgr.EntityDestroyed(e);

    // The slot is clean; re-inserting should not trigger the double-add assert.
    mgr.AddComponent<Position>(e, {9.f, 0.f, 0.f});
    EXPECT_FLOAT_EQ(mgr.GetComponent<Position>(e).x, 9.f);
}

TEST_F(ComponentManagerTest, EntityDestroyed_DoesNotAffectOtherEntities)
{
    mgr.AddComponent<Health>(0, {100});
    mgr.AddComponent<Health>(1, {200});

    mgr.EntityDestroyed(0);

    // Entity 1 should be completely unaffected.
    EXPECT_EQ(mgr.GetComponent<Health>(1).hp, 200);
}
