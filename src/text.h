#ifndef TEXT_H
#define TEXT_H
// Text.h

#pragma once

#define FONTWHITE	0
#define FONTRED		1
#define FONTSLIM	2
#define FONTLITTLE	10

#ifdef SB1_EVENT_API
#define FONTGOLD 1
#define FONTSELECTED 2
#endif

extern
void DrawText(CPixmap *pPixmap, POINT pos, char *pText, int font=0);

extern
void DrawChar(CPixmap* pPixmap, POINT* pos, char c, int font);

extern
void DrawCharSingle(CPixmap *pPixmap, POINT pos, int rank, int font);

extern
void DrawTextLeft(CPixmap *pPixmap, POINT pos, char *pText, int font);

extern
void DrawTextPente(CPixmap *pPixmap, POINT pos, char *pText,
				   int pente, int font=0);

extern
void DrawTextRect(CPixmap *pPixmap, POINT pos, char *pText,
				  int pente, int font=0, int part=-1);

extern
void DrawTextCenter(CPixmap *pPixmap, POINT pos, char *pText, int font=0);

extern
int GetTextHeight(char *pText, int font=0, int part=-1);

extern
int GetTextWidth(char *pText, int font=0);

extern
void DrawBignum(CPixmap *pPixmap, POINT pos, int num);

extern
int GetBignumWidth(int num);


#endif /* TEXT_H */
