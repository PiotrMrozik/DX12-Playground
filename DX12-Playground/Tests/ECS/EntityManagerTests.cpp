#include <gtest/gtest.h>
#include <unordered_set>

#include <ECS/Entity/EntityManager.h>

// -----------------------------------------------------------------------
// EntityManager - unit tests
//
// EntityManager owns the pool of valid entity IDs [0, MAX_ENTITIES) and
// the per-entity signature bitsets.  Tests verify creation, destruction,
// ID reuse, and signature round-trips.
// -----------------------------------------------------------------------

TEST(EntityManagerTest, CreateEntity_ReturnsNonInvalidId)
{
    EntityManager mgr;
    EXPECT_NE(mgr.CreateEntity(), INVALID_ENTITY);
}

TEST(EntityManagerTest, CreateEntity_IdIsInRange)
{
    EntityManager mgr;
    Entity e = mgr.CreateEntity();
    EXPECT_LT(e, MAX_ENTITIES);
}

TEST(EntityManagerTest, CreateEntity_TwoCalls_UniqueIds)
{
    EntityManager mgr;
    Entity e1 = mgr.CreateEntity();
    Entity e2 = mgr.CreateEntity();
    EXPECT_NE(e1, e2);
}

TEST(EntityManagerTest, DestroyEntity_IdBecomesReusable)
{
    EntityManager mgr;

    // Exhaust the entire pool so the destroyed ID is the only free slot,
    // guaranteeing the next CreateEntity must return it regardless of
    // where DestroyEntity places it in the queue.
    std::vector<Entity> all;
    all.reserve(MAX_ENTITIES);
    for (std::uint32_t i = 0; i < MAX_ENTITIES; ++i)
        all.push_back(mgr.CreateEntity());

    Entity target = all[42];
    mgr.DestroyEntity(target);

    EXPECT_EQ(mgr.CreateEntity(), target);
}

TEST(EntityManagerTest, SetAndGetSignature_RoundTrip)
{
    EntityManager mgr;
    Entity e = mgr.CreateEntity();

    EntitySignature sig;
    sig.set(3);
    sig.set(7);
    mgr.SetSignature(e, sig);

    EXPECT_EQ(mgr.GetSignature(e), sig);
}

TEST(EntityManagerTest, DefaultSignature_IsAllZeros)
{
    EntityManager mgr;
    Entity e = mgr.CreateEntity();
    EXPECT_TRUE(mgr.GetSignature(e).none());
}

TEST(EntityManagerTest, DestroyEntity_ClearsSignature)
{
    EntityManager mgr;

    // Exhaust the pool so the destroyed entity's ID is the only free slot.
    std::vector<Entity> all;
    all.reserve(MAX_ENTITIES);
    for (std::uint32_t i = 0; i < MAX_ENTITIES; ++i)
        all.push_back(mgr.CreateEntity());

    Entity e = all[0];
    EntitySignature sig;
    sig.set(5);
    mgr.SetSignature(e, sig);

    mgr.DestroyEntity(e);

    // Only one slot is free → must get e back.
    Entity same = mgr.CreateEntity();
    ASSERT_EQ(same, e);
    EXPECT_TRUE(mgr.GetSignature(same).none());
}

TEST(EntityManagerTest, CreateMaxEntities_AllUniqueAndValid)
{
    EntityManager mgr;
    std::unordered_set<Entity> seen;
    seen.reserve(MAX_ENTITIES);

    for (std::uint32_t i = 0; i < MAX_ENTITIES; ++i)
    {
        Entity e = mgr.CreateEntity();
        EXPECT_NE(e, INVALID_ENTITY);
        EXPECT_LT(e, MAX_ENTITIES);
        seen.insert(e);
    }

    EXPECT_EQ(seen.size(), static_cast<std::size_t>(MAX_ENTITIES));
}

TEST(EntityManagerTest, DestroyAll_ThenRecreate_Works)
{
    EntityManager mgr;

    std::vector<Entity> batch;
    batch.reserve(MAX_ENTITIES);
    for (std::uint32_t i = 0; i < MAX_ENTITIES; ++i)
        batch.push_back(mgr.CreateEntity());

    for (Entity e : batch)
        mgr.DestroyEntity(e);

    // After destroying everything we should be able to create MAX_ENTITIES again.
    for (std::uint32_t i = 0; i < MAX_ENTITIES; ++i)
        EXPECT_NE(mgr.CreateEntity(), INVALID_ENTITY);
}
