// SB2 scene motion identities, snap boundaries and Build visibility rules.
// Pixel interpolation and faithful equivalence are exercised by sb2_presentation.
#include "game/scene.h"
#include <cstdio>

using namespace blupi::scene;
static int failures;
static void check(bool ok, const char* why)
{
    if (!ok) { printf("FAIL: %s\n", why); ++failures; }
}
static Item actor(Entity entity, int x)
{
    Item item;
    item.entity = entity;
    item.layer = Layer::World;
    item.world = POINT{x, 100};
    return item;
}
int main()
{
    Scene previous, current;
    previous.items = {actor(Entity::Blupi, 100), actor(Entity::BlupiEffect, 100), actor(Entity::BlupiEffect, 110)};
    current.items = {actor(Entity::Blupi, 116), actor(Entity::BlupiEffect, 116), actor(Entity::BlupiEffect, 126)};
    current.camera.x = 16;
    Motion motion;
    PairScenes(previous, current, motion);
    check(motion.valid && motion.cameraFrom.x == 0, "camera pairs across a tick");
    check(motion.from[0].x == 100 && motion.from[1].x == 100 && motion.from[2].x == 110,
          "repeated effects pair by occurrence, independently of other entities");
    current.items[0].world.x = 165;
    PairScenes(previous, current, motion);
    check(motion.valid && motion.from[0].x == 165 && motion.from[1].x == 100, "actor teleport snaps only that actor");
    current.camera.x = 65;
    PairScenes(previous, current, motion);
    check(!motion.valid, "camera teleport snaps whole scene");
    current.camera.x = 64;
    PairScenes(previous, current, motion);
    check(motion.valid, "camera snap threshold is inclusive");
    current.phase = 1;
    PairScenes(previous, current, motion);
    check(!motion.valid, "phase changes snap whole scene");
    current.phase = 0;
    current.items.push_back(actor(Entity::Lift, 500));
    PairScenes(previous, current, motion);
    check(motion.from.back().x == 500, "new entity starts at its current position");

    Item item;
    item.cull = Item::Cull::Point;
    check(Visible(item, POINT{704, 544}, POINT{64, 64}, 640, 480), "mover inclusive right/bottom bounds");
    check(!Visible(item, POINT{705, 544}, POINT{64, 64}, 640, 480), "mover outside faithful view");
    check(Visible(item, POINT{705, 544}, POINT{64, 64}, 854, 480), "same mover visible in extended view");
    item.cull = Item::Cull::BigCell;
    item.index = 14 * MAXCELY + 1;
    check(!Visible(item, {}, POINT{64, 64}, 640, 480) && Visible(item, {}, POINT{64, 64}, 854, 480),
          "big-decor Build range extends with the view");
    current.maxView.x = 6400 - 640;
    auto left = PlaceView(current, POINT{0, 0}, 854);
    auto right = PlaceView(current, POINT{5760, 0}, 854);
    check(left.camera.x == 0 && right.camera.x == 6400 - 854, "wide view clamps at both world edges");
    current.maxView.x = 0;
    auto small = PlaceView(current, POINT{0, 0}, 854);
    check(small.left == 107 && small.right == 747, "one-screen world stays centred");
    return failures ? 1 : 0;
}
