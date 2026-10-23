// SB2 scene, extending SB1's scene/end-of-tick split (SB2_PORT.md M4a).
//
// BuildScene() is a port-side derivative of CDecor::Build, DecorNextAction,
// DrawInfo and VoyageDraw (games/sb2/src/decblock.cpp, games/sb2/src/decor.cpp). It reads the game
// state without modifying it and records what Build draws, in Build's order,
// as items that know which layer and entity they belong to and where they are
// in the world. FlattenScene() turns a scene back into the CPixmap calls Build
// made, which gives the faithful 640x480 picture; later renderers can instead
// use the world positions (wider viewports, interpolation).
//
// The state changes Build made while drawing (m_time, the camera-shake step,
// m_blupiSec, tile drops/lightning, voyage particles, the tutorial clamp and jauge redraw
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
	// Visible() (the SB1 scene API):
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

// M4a records exactly Build's items. The reserved Coverage argument must be
// zero; extending it and flagging extra items belongs to M4b.
// Pure: does not modify decor (const access only).
void BuildScene(const CDecor& decor, RECT rect, POINT mousePos, Scene& out,
				const Coverage& coverage = Coverage());

// Issues the CPixmap calls the scene records, in order, skipping extended
// items. Reads no game state: a scene can be drawn any number of times.
void FlattenScene(const Scene& scene, CPixmap* pixmap);

// Build's visibility rule of an item whose rule point is at ref, for a view
// of the given size at view.
bool Visible(const Item& item, POINT ref, POINT view, int width, int height);

// M4a supports the faithful view only. Motion and ViewSpec keep SB1's API
// shape for M4b; no interpolation, coverage extension or wider view yet.
struct Motion
{
	bool valid = false;
	POINT cameraFrom = {0, 0};
	std::vector<POINT> from;
};
struct ViewSpec { int width = LXIMAGE; };
void DrawView(const Scene& scene, const Motion* motion, double alpha, CPixmap* pixmap,
			  const ViewSpec& spec = ViewSpec());
} // namespace blupi::scene
#endif
