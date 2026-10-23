// Scene of one game frame (port/docs/ROADMAP.md stage 3).
//
// BuildScene() is a port-side derivative of CDecor::Build, DecorNextAction,
// DrawInfo and VoyageDraw (src/decblock.cpp, src/decor.cpp). It reads the game
// state without modifying it and records what Build draws, in Build's order,
// as items that know which layer and entity they belong to and where they are
// in the world. FlattenScene() turns a scene back into the CPixmap calls Build
// made, which gives the faithful 640x480 picture; later renderers can instead
// use the world positions (wider viewports, interpolation).
//
// The state changes Build made while drawing (m_time, the camera-shake step,
// m_blupiSec, the mission-10 position clamp in DrawInfo, the jauge redraw
// flags) are NOT made here; EndOfTick() (game/end_of_tick.cpp) makes them.
#ifndef BLUPI_GAME_SCENE_H
#define BLUPI_GAME_SCENE_H

#include "def.h"
#include "decor.h"
#include "scene/commands.h"

#include <string>
#include <vector>

namespace blupi::scene
{
enum class Entity : unsigned char
{
	None,
	Parallax,
	BigDecor,     // index = cell x * MAXCELY + y
	Decor,        // index = cell x * MAXCELY + y
	Overlay,      // animated overlay tile, index = cell
	StartMarker,  // editor, index = team
	NetBlupi,     // index = player
	Blupi,
	BlupiEffect,  // shield, power, cloud, hide around Blupi
	MoveObject,   // index = m_moveObject slot
	Lift,         // index = m_moveObject slot
	PathMarker,   // editor: mover start/end marks, index = m_moveObject slot
	CellHili,     // editor: highlighted cells (placed with the unshaken camera)
	Hud,          // lives, bullets, character and treasure counters
	Text,         // notification (index = line), tutorial hint, phase caption
	Jauge,        // index = jauge
	Voyage,       // QuickIcon; also rect = start and end points, mode = phase, font = total
};

struct Item : commands::Command<POINT, RECT, BOOL>
{
	Layer  layer = Layer::Screen;
	Entity entity = Entity::None;
	int    index = 0;
	POINT  world = {0, 0};  // World: pos + view (CellHili: pos + camera)

	// Visibility rule of Build for this item, evaluated against any view by
	// Visible() (ROADMAP.md stage 7):
	//   None     always drawn (Blupi, network Blupis, markers, overlays)
	//   BigCell  big-decor tile loop range, cell = index
	//   Cell     decor and overlay tile loop range, cell = index
	//   Point    move-object bounds test on ref (posCurrent, posStart or posEnd)
	enum class Cull : unsigned char { None, BigCell, Cell, Point };
	Cull   cull = Cull::None;
	POINT  ref = {0, 0};     // Cull::Point: the world point Build tests
	bool   simCamera = false; // World item placed with the camera, not the shaken view
	bool   extended = false;  // outside Build's set at this view: never drawn by
	                          // FlattenScene, only by views that need it
};

struct Scene
{
	POINT camera = {0, 0};   // m_posDecor (simulation camera)
	POINT view = {0, 0};     // camera + shake (DecorNextAction), used for drawing
	POINT maxView = {0, 0};  // the bounds DecorNextAction clamps the view to
	int   phase = 0;         // m_phase
	std::vector<Item> items;
};

// World area, in pixels beyond Build's own view on each side, for which
// BuildScene also records the items Build would draw at a shifted view
// (flagged extended). Zero: exactly Build's items.
// Pure: does not modify decor (const access only).
void BuildScene(const CDecor& decor, RECT rect, POINT mousePos, Scene& out,
				const Coverage& coverage = Coverage());

// Issues the CPixmap calls the scene records, in order, skipping extended
// items. Reads no game state: a scene can be drawn any number of times.
void FlattenScene(const Scene& scene, CPixmap* pixmap);

// Build's visibility rule of an item whose rule point is at ref, for a view
// of the given size at view.
bool Visible(const Item& item, POINT ref, POINT view, int width, int height);

// Interpolation (ROADMAP.md stage 7), visual only. For each item of the
// current scene, where it was in the previous one: the same entity identity
// (entity, index, occurrence) on the world layer, or the voyage icon. An item
// without a counterpart, or that moved more than kSnapDistance, starts where
// it is (snap). The whole scene snaps when the phase changed or the camera
// moved more than kSnapDistance.
constexpr int kSnapDistance = 64;
constexpr int kInterpolationMargin = 2 * kSnapDistance;  // Coverage needed around the view

struct Motion
{
	bool valid = false;
	POINT cameraFrom = {0, 0};
	std::vector<POINT> from;   // per item of the current scene (world, or pos for Screen)
};

void PairScenes(const Scene& prev, const Scene& curr, Motion& out);

// Draws the scene as seen at alpha in [0, 1] between the previous tick
// (alpha 0, positions from motion) and this one (alpha 1). Positions and the
// camera are interpolated; sprites, animation frames, HUD and texts are the
// current tick's. The camera shake of the current tick is added on top of the
// interpolated camera. Items are kept or dropped by Build's own rules
// evaluated at the drawn view, and the parallax background is drawn for that
// view. With motion == NULL or alpha == 1, at the faithful size, it issues
// exactly FlattenScene's calls (--check-view).
//
// Widescreen (ROADMAP.md stage 8): a wider view keeps the simulation camera's
// 640x480 frame and shows more of the world around it, centred on it and
// clamped to the world; a world narrower than the view is centred, the rest
// left unpainted. The HUD and texts stay in the centred 640x480 frame; a
// voyage icon travels from the object's place in the view to its HUD counter.
// The parallax background is placed by the view camera.
//
// The editor places its world the same way and puts its screen layer (the
// toolbar, the caption) at the view's left edge instead (uiLeft); the port
// routes the mouse between the two (game/main.cpp, editor_wide.cpp).
struct ViewSpec
{
	int  width = LXIMAGE;        // view width (the target is width x LYIMAGE)
	bool cullAtFrame = false;    // test: Build's rules at the frame (--check-wide)
	bool uiAtFrame = false;      // test: the screen layer with the frame (--check-wide)
	bool uiLeft = false;         // the screen layer at the view's left edge (the editor)
};

// Screen x of the screen layer in a view of that width: the centred frame, or
// the left edge (uiLeft). The mouse's coordinates are relative to it.
inline int FrameOffset(int width, bool uiLeft)
{
	return uiLeft ? 0 : (width - LXIMAGE) / 2;
}

// Where the simulation frame lies in a view: the view camera and the frame's
// x offset in it (0 at the faithful size).
struct ViewPlacement
{
	POINT camera;   // world position of the view's left/top corner (shaken)
	int   frameX;   // screen x of the frame's left edge
	int   left, right;  // screen columns that show the world
};
ViewPlacement PlaceView(const Scene& scene, POINT view, int width);

void DrawView(const Scene& scene, const Motion* motion, double alpha, CPixmap* pixmap,
			  const ViewSpec& spec = ViewSpec());
}  // namespace blupi::scene

#endif
