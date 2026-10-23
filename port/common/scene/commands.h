// Game-independent scene command storage and replay, shared by SB1 and SB2.
// Derived from SB1 scene.cpp's Issue and the identical CJauge::Draw bodies.
// Point/rectangle/flag types, channels, gauge metrics and text functions are
// supplied by each game's adapter. No historical or platform headers here.
#ifndef BLUPI_SCENE_COMMANDS_H
#define BLUPI_SCENE_COMMANDS_H
#include <string>
namespace blupi::scene {
enum class Layer : unsigned char
{
	Background,   // parallax decor image, scrolled at 2/3 of the camera
	World,        // positioned in world coordinates (screen = world - camera)
	Screen,       // fixed 640x480 overlay: HUD, texts, jauges, voyage icon
	Target,       // drawing into an offscreen surface (CHTEMP composition)
};

struct Coverage
{
	int left = 0, right = 0, top = 0, bottom = 0;
};

namespace commands {
template<class Point, class Rect, class Flag> struct Command
{
	enum class Op : unsigned char
	{
		SetClip,      // rect
		DrawPart,     // chDst, channel, pos, rect, mode, mask
		QuickIcon,    // channel, rank, pos
		DrawIcon,     // chDst, channel, rank, pos, mode, mask
		DrawText,     // DrawText(pixmap, pos, text, font) of src/text.h
		DrawTextCenter, // DrawTextCenter(pixmap, pos, text, font) of src/text.h
		JaugeDraw,    // snapshot of jauge index: mask = hidden, mode = type, rank = level,
		              // pos = m_pos, rect = its area (drawn like CJauge::Draw)
	};

	Op     op;
	int    chDst = -1, channel = 0, rank = 0, mode = 0, font = 0;
	Flag   mask = 0;
	Point  pos = {0, 0};    // position as passed to CPixmap (faithful 640x480)
	Rect   rect = {0, 0, 0, 0};
	std::string text;

};

template<class Item, class Point, class Pixmap, class Text, class Center>
void Issue(const Item& item, Point pos, Pixmap* pixmap, std::string& text,
		   int chBack, int chJauge, int dimX, int dimY, Text drawText, Center drawTextCenter)
{
	using Op = typename Item::Op;
	switch (item.op)
	{
	case Op::SetClip:   pixmap->SetClipping(item.rect); break;
	case Op::DrawPart:  pixmap->DrawPart(item.chDst, item.channel, pos, item.rect, item.mode, item.mask); break;
	case Op::QuickIcon: pixmap->QuickIcon(item.channel, item.rank, pos); break;
	case Op::DrawIcon:  pixmap->DrawIcon(item.chDst, item.channel, item.rank, pos, item.mode, item.mask); break;
	case Op::DrawText:
		text = item.text;  // the historical functions take char*
		drawText(pixmap, pos, &text[0], item.font);
		break;
	case Op::DrawTextCenter:
		text = item.text;
		drawTextCenter(pixmap, pos, &text[0], item.font);
		break;
	case Op::JaugeDraw:
	{
		// Drawing part of CJauge::Draw (src/jauge.cpp) from the snapshot.
		int  part;
		decltype(item.rect) rect;
		if (item.mask)  // bouton caché ?
		{
			pixmap->DrawPart(-1, chBack, pos, item.rect, 1);  // dessine le fond
			break;
		}
		part = (item.rank*(dimX-6-4))/100;
		rect.left   = 0;
		rect.right  = dimX;
		rect.top    = dimY*0;
		rect.bottom = dimY*1;
		pixmap->DrawPart(-1, chJauge, pos, rect);  // partie noire
		if (part > 0)
		{
			rect.left   = 0;
			rect.right  = 6+part;
			rect.top    = dimY*item.mode;
			rect.bottom = dimY*(item.mode+1);
			pixmap->DrawPart(-1, chJauge, pos, rect);  // partie colorée
		}
		break;
	}
	}
}

template<class Items, class IssueItem>
void Flatten(const Items& items, IssueItem issue)
{
	for (const auto& item : items) if (!item.extended) issue(item);
}
} // namespace commands
} // namespace blupi::scene
#endif
