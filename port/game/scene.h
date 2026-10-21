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
// flags) are NOT made here; see ROADMAP.md stage 4.
#ifndef BLUPI_GAME_SCENE_H
#define BLUPI_GAME_SCENE_H

#include "def.h"
#include "decor.h"

#include <string>
#include <vector>

namespace blupi::scene
{
enum class Layer : unsigned char
{
	Background,   // parallax decor image, scrolled at 2/3 of the camera
	World,        // positioned in world coordinates (screen = world - camera)
	Screen,       // fixed 640x480 overlay: HUD, texts, jauges, voyage icon
	Target,       // drawing into an offscreen surface (CHTEMP composition)
};

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
	Voyage,
};

struct Item
{
	enum class Op : unsigned char
	{
		SetClip,      // rect
		DrawPart,     // chDst, channel, pos, rect, mode, mask
		QuickIcon,    // channel, rank, pos
		DrawIcon,     // chDst, channel, rank, pos, mode, mask
		DrawText,     // DrawText(pixmap, pos, text, font) of src/text.h
		DrawTextCenter, // DrawTextCenter(pixmap, pos, text, font) of src/text.h
		JaugeDraw,    // index = jauge (calls the historical CJauge::Draw body)
	};

	Op     op;
	Layer  layer = Layer::Screen;
	Entity entity = Entity::None;
	int    index = 0;
	int    chDst = -1, channel = 0, rank = 0, mode = 0, font = 0;
	BOOL   mask = FALSE;
	POINT  pos = {0, 0};    // position as passed to CPixmap (faithful 640x480)
	POINT  world = {0, 0};  // Layer::World: pos + view (CellHili: pos + camera)
	RECT   rect = {0, 0, 0, 0};
	std::string text;
};

struct Scene
{
	POINT camera = {0, 0};   // m_posDecor (simulation camera)
	POINT view = {0, 0};     // camera + shake (DecorNextAction), used for drawing
	std::vector<Item> items;
};

// Pure: does not modify decor (const access only).
void BuildScene(const CDecor& decor, RECT rect, POINT mousePos, Scene& out);

// Issues the CPixmap calls the scene records, in order.
void FlattenScene(const Scene& scene, CDecor& decor, CPixmap* pixmap);
}  // namespace blupi::scene

#endif
