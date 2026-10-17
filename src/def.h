// Def.h
//

#ifndef DEF_H
#define DEF_H

#pragma once

#include <Windows.h>

#define _DEMO			FALSE	// TRUE=demo, FALSE=complet
#define _INTRO			FALSE	// TRUE si images d'introduction
#define _EGAMES			FALSE	// TRUE version pour eGames
#define _SE				FALSE	// TRUE eGames Special Edition


#define REQUIRE_CD		TRUE
#define DREAM_IN_SB2	TRUE
#define MAXGAMER	8
#define MAXNETPLAYER	4
#define MAXTEAM	4
#define MAXNETMESSAGE	20
#define MAXSESSION	100
#define MAXMOVEOBJECT	200
#define MAXNOTIF	4
#define MAXFIFOPOS	10
#define MAXCHAT	6
#define MAXSAVE	6
#define MAXDEMO	10000
#define MAXINDEX	20
#define MAXMESSAGEPERPACKET 5
#define TRAJBYTEX	(MAXCELX - 1) / 8 + 1

#define LXIMAGE			640		// dimensions de la fenêtre de jeu
#define LYIMAGE			480

#define POSDRAWX		144		// surface de dessin
#define POSDRAWY		15
#define DIMDRAWX		480
#define DIMDRAWY		450

#define POSMAPX			8		// surface pour la carte
#define POSMAPY			15
#define DIMMAPX			128
#define DIMMAPY			128

#define MAXCELX			100		// nb max de cellules d'un monde
#define MAXCELY			100

#define DIMCELX			60		// dimensions d'une cellule (décor)
#define DIMCELY			30

#define DIMOBJX			64		// dimensions d'un objet
#define DIMOBJY			64

#define DIMBLUPIX		60		// dimensions de blupi
#define DIMBLUPIY		60
#define SHIFTBLUPIY		5		// petit décalage vers le haut

#define DIMBUTTONX		40		// dimensions d'un button
#define DIMBUTTONY		40

#define DIMJAUGEX		124		// dimensions de la jauge
#define DIMJAUGEY		22

#define POSSTATX		12		// statistiques
#define POSSTATY		220
#define DIMSTATX		60
#define DIMSTATY		30

#define DIMTEXTX		16		// dimensions max d'un caractère
#define DIMTEXTY		16

#define DIMLITTLEX		16		// dimensions max d'un petit caractère
#define DIMLITTLEY		12

#define CHBACK			0
#define CHFLOOR			1
#define CHOBJECT		2
#define CHOBJECTo		3
#define CHBLUPI			4
#define CHPIXMAP_CURSOR	10
#define CHHILI			5
#define CHFOG			6
#define CHMASK1			7
#define CHLITTLE		7
#define CHMAP			9
#define CHBUTTON		4
#define CHGROUND		11
#define CHJAUGE			5
#define CHTEXT			6
#define CHBIGNUM		14

#define FOGHIDE			4


// Directions :

#define DIRECT_E		(0*16)		// est
#define DIRECT_SE		(1*16)		// sud-est
#define DIRECT_S		(2*16)		// sud
#define DIRECT_SO		(3*16)		// sud-ouest
#define DIRECT_O		(4*16)		// ouest
#define DIRECT_NO		(5*16)		// nord-ouest
#define DIRECT_N		(6*16)		// nord
#define DIRECT_NE		(7*16)		// nord-est

//                  NO
//            O     |     N
//             \    |    / 
//               \  |  / 
//                 \|/
//        SO -------o------- NE
//                 /|\
//               /  |  \
//             /    |    \
//            S     |     E
//          (y)     SE    (x)


// Actions :

#define ACTION_STOP			1		// arrêt
#define ACTION_STOPf		1		// arrêt fatigué
#define ACTION_MARCHE		2		// marche
#define ACTION_MARCHEf		3		// marche fatigué
#define ACTION_BUILD		4		// construit
#define ACTION_PIOCHE		5		// pioche
#define ACTION_ENERGY		6		// prend de l'énergie
#define ACTION_TAKE			8		// fait sauter un objet sur la tête (est)
#define ACTION_DEPOSE		9		// repose l'objet sur la tête (est)
#define ACTION_SCIE			10		// scie du bois
#define ACTION_BRULE		11		// blupi crame !
#define ACTION_TCHAO		12		// blupi disparait !
#define ACTION_MANGE		13		// blupi mange
#define ACTION_NAISSANCE	14		// naissance
#define ACTION_SAUTE2		15		// saute par-dessus un obstacle
#define ACTION_SAUTE3		16		// saute par-dessus un obstacle
#define ACTION_SAUTE4		17		// saute par-dessus un obstacle
#define ACTION_SAUTE5		18		// saute par-dessus un obstacle
#define ACTION_PONT			19		// pousse un pont
#define ACTION_MISC1		20		// divers 1 (hausse les épaules)
#define ACTION_MISC2		21		// divers 2 (grat-grat)
#define ACTION_MISC3		22		// divers 3 (yoyo)
#define ACTION_MISC1f		23		// divers 1 fatigué (bof-bof)
#define ACTION_GLISSE		24		// glisse en marchant
#define ACTION_BOIT			25		// blupi boit
#define ACTION_LABO			26		// blupi travaille dans son laboratoire
#define ACTION_DYNAMITE		27		// blupi fait péter la dynamite
#define ACTION_DELAY		28		// blupi attend un frame
#define ACTION_CUEILLE1		29		// blupi cueille des fleurs
#define ACTION_CUEILLE2		30		// blupi cueille des fleurs
#define ACTION_MECHE		31		// blupi se bouche les oreilles
#define ACTION_STOPb		32		// arrêt en bateau
#define ACTION_MARCHEb		33		// avance en bateau
#define ACTION_STOPj		34		// arrêt en jeep
#define ACTION_MARCHEj		35		// avance en jeep
#define ACTION_ELECTRO		36		// blupi électrocuté
#define ACTION_GRILLE1		37		// blupi grille (phase 1)
#define ACTION_GRILLE2		38		// blupi grille (phase 2)
#define ACTION_GRILLE3		39		// blupi grille (phase 3)
#define ACTION_MISC4		40		// divers 4 (ferme les yeux)
#define ACTION_CONTENT		41		// blupi est content
#define ACTION_ARROSE		42		// blupi arrose
#define ACTION_BECHE		43		// blupi bèche
#define ACTION_CUEILLE3		44		// blupi cueille des fleurs
#define ACTION_BUILDBREF	45		// construit
#define ACTION_BUILDSEC		46		// construit
#define ACTION_BUILDSOURD	47		// construit
#define ACTION_BUILDPIERRE	48		// construit
#define ACTION_PIOCHEPIERRE	49		// pioche
#define ACTION_PIOCHESOURD	50		// pioche
#define ACTION_MISC5		51		// divers 5 (ohé)
#define ACTION_TELEPORTE1	52		// téléporte
#define ACTION_TELEPORTE2	53		// téléporte
#define ACTION_TELEPORTE3	54		// téléporte
#define ACTION_STOPa		55		// arrêt armure
#define ACTION_MARCHEa		56		// marche armure
#define ACTION_ARMUREOPEN	57		// ouvre armure
#define ACTION_ARMURECLOSE	58		// ferme armure
#define ACTION_SAUTE1		59		// saute dans la jeep
#define ACTION_MISC6		60		// divers 6 (diabolo)

#define ACTION_A_STOP		100		// araignée: arrêt
#define ACTION_A_MARCHE		101		// araignée: marche
#define ACTION_A_SAUT		102		// araignée: saute
#define ACTION_A_GRILLE		103		// araignée: grille dans rayon
#define ACTION_A_POISON		105		// araignée: empoisonée
#define ACTION_A_MORT1		106		// araignée: meurt
#define ACTION_A_MORT2		107		// araignée: meurt
#define ACTION_A_MORT3		108		// araignée: meurt

#define ACTION_V_STOP		200		// virus: arrêt
#define ACTION_V_MARCHE		201		// virus: marche
#define ACTION_V_GRILLE		202		// virus: grille dans rayon

#define ACTION_T_STOP		300		// tracks: arrêt
#define ACTION_T_MARCHE		301		// tracks: marche
#define ACTION_T_ECRASE		302		// tracks: écrase un objet

#define ACTION_R_STOP		400		// robot: arrêt
#define ACTION_R_MARCHE		401		// robot: marche
#define ACTION_R_APLAT		402		// robot: applatit
#define ACTION_R_BUILD		403		// robot: construit
#define ACTION_R_DELAY		404		// robot: construit
#define ACTION_R_CHARGE		405		// robot: recharge
#define ACTION_R_ECRASE		406		// robot: écrase un objet

#define ACTION_B_STOP		500		// bombe: arrêt
#define ACTION_B_MARCHE		501		// bombe: marche

#define ACTION_D_DELAY		600		// détonnateur: attend

#define ACTION_E_STOP		700		// électro: arrêt
#define ACTION_E_MARCHE		701		// électro: marche
#define ACTION_E_DEBUT		702		// électro: débute
#define ACTION_E_RAYON		703		// électro: rayonne

#define ACTION_D_STOP		800		// disciple: arrêt
#define ACTION_D_MARCHE		801		// disciple: marche
#define ACTION_D_BUILD		802		// disciple: construit
#define ACTION_D_PIOCHE		803		// disciple: pioche
#define ACTION_D_SCIE		804		// disciple: scie du bois
#define ACTION_D_TCHAO		805		// disciple: disparait !
#define ACTION_D_CUEILLE1	806		// disciple: cueille des fleurs
#define ACTION_D_CUEILLE2	807		// disciple: cueille des fleurs
#define ACTION_D_MECHE		808		// disciple: se bouche les oreilles
#define ACTION_D_ARROSE		809		// disciple: arrose
#define ACTION_D_BECHE		810		// disciple: bèche


// Sons :

#define SOUND_CLICK			0
#define SOUND_BOING			1
#define SOUND_OK1			2
#define SOUND_OK2			3
#define SOUND_OK3			4
#define SOUND_GO1			5
#define SOUND_GO2			6
#define SOUND_GO3			7
#define SOUND_TERM1			8
#define SOUND_TERM2			9
#define SOUND_TERM3			10
#define SOUND_COUPTERRE		11
#define SOUND_COUPTOC		12
#define SOUND_SAUT			13
#define SOUND_HOP			14
#define SOUND_SCIE			15
#define SOUND_FEU			16
#define SOUND_BRULE			17
#define SOUND_TCHAO			18
#define SOUND_MANGE			19
#define SOUND_NAISSANCE		20
#define SOUND_A_SAUT		21
#define SOUND_A_HIHI		22
#define SOUND_PLOUF			23
#define SOUND_BUT			24
#define SOUND_RAYON1		25
#define SOUND_RAYON2		26
#define SOUND_VIRUS			27
#define SOUND_GLISSE		28
#define SOUND_BOIT			29
#define SOUND_LABO			30
#define SOUND_DYNAMITE		31
#define SOUND_PORTE			32
#define SOUND_FLEUR			33
#define SOUND_T_MOTEUR		34
#define SOUND_T_ECRASE		35
#define SOUND_PIEGE			36
#define SOUND_AIE			37
#define SOUND_A_POISON		38
#define SOUND_R_MOTEUR		39
#define SOUND_R_APLAT		40
#define SOUND_R_ROTATE		41
#define SOUND_R_CHARGE		42
#define SOUND_B_SAUT		43
#define SOUND_BATEAU		44
#define SOUND_JEEP			45
#define SOUND_MINE			46
#define SOUND_USINE			47
#define SOUND_E_RAYON		48
#define SOUND_E_TOURNE		49
#define SOUND_ARROSE		50
#define SOUND_BECHE			51
#define SOUND_D_BOING		52
#define SOUND_D_OK			53
#define SOUND_D_GO			54
#define SOUND_D_TERM		55
#define SOUND_BOING1		56
#define SOUND_BOING2		57
#define SOUND_BOING3		58
#define SOUND_OK4			59
#define SOUND_OK5			60
#define SOUND_OK6			61
#define SOUND_OK1f			62
#define SOUND_OK2f			63
#define SOUND_OK3f			64
#define SOUND_OK1e			65
#define SOUND_OK2e			66
#define SOUND_OK3e			67
#define SOUND_GO4			68
#define SOUND_GO5			69
#define SOUND_GO6			70
#define SOUND_TERM4			71
#define SOUND_TERM5			72
#define SOUND_TERM6			73
#define SOUND_COUPSEC		74
#define SOUND_COUPPIERRE	75
#define SOUND_COUPSOURD		76
#define SOUND_COUPBREF		77
#define SOUND_OPEN			78
#define SOUND_CLOSE			79
#define SOUND_TELEPORTE		80
#define SOUND_ARMUREOPEN	81
#define SOUND_ARMURECLOSE	82
#define SOUND_WIN			83
#define SOUND_LOST			84
#define SOUND_MOVIE			99


// Boutons (play) :

#define MAXBUTTON			40

#define BUTTON_GO			0
#define BUTTON_STOP			1
#define BUTTON_MANGE		2
#define BUTTON_CARRY		3
#define BUTTON_DEPOSE		4
#define BUTTON_ABAT			5
#define BUTTON_ROC			6
#define BUTTON_CULTIVE		7
#define BUTTON_BUILD1		8
#define BUTTON_BUILD2		9
#define BUTTON_BUILD3		10
#define BUTTON_BUILD4		11
#define BUTTON_BUILD5		12
#define BUTTON_BUILD6		13
#define BUTTON_MUR			14
#define BUTTON_PALIS		15
#define BUTTON_ABATn		16
#define BUTTON_ROCn			17
#define BUTTON_PONT			18
#define BUTTON_TOUR			19
#define BUTTON_BOIT			20
#define BUTTON_LABO			21
#define BUTTON_FLEUR		22
#define BUTTON_FLEURn		23
#define BUTTON_DYNAMITE		24
#define BUTTON_BATEAU		25
#define BUTTON_DJEEP		26
#define BUTTON_DRAPEAU		27
#define BUTTON_EXTRAIT		28
#define BUTTON_FABJEEP		29
#define BUTTON_FABMINE		30
#define BUTTON_FABDISC		31
#define BUTTON_REPEAT		32
#define BUTTON_DARMURE		33
#define BUTTON_FABARMURE	34


// Erreurs :

#define ERROR_MISC			1
#define ERROR_GROUND		2
#define ERROR_FREE			3
#define ERROR_PONTOP		4
#define ERROR_PONTTERM		5
#define ERROR_TOURISOL		6
#define ERROR_TOUREAU		7
#define ERROR_TELE2			8


// Lutins pour la souris

#define SPRITE_ARROW		1
#define SPRITE_POINTER		2
#define SPRITE_MAP			3
#define SPRITE_ARROWU		4
#define SPRITE_ARROWD		5
#define SPRITE_ARROWL		6
#define SPRITE_ARROWR		7
#define SPRITE_ARROWUL		8
#define SPRITE_ARROWUR		9
#define SPRITE_ARROWDL		10
#define SPRITE_ARROWDR		11
#define SPRITE_WAIT			11
#define SPRITE_EMPTY		12
#define SPRITE_FILL			14


// User define message

#define WM_UPDATE				(WM_USER+1)

#define WM_DECOR1				(WM_USER+20)
#define WM_DECOR2				(WM_USER+21)
#define WM_DECOR3				(WM_USER+22)
#define WM_DECOR4				(WM_USER+23)
#define WM_DECOR5				(WM_USER+24)
#define WM_DECOR6				(WM_USER+25)
#define WM_DECOR7				(WM_USER+26)
#define WM_DECOR8				(WM_USER+27)

#define WM_ACTION_GO			(WM_USER+30)
#define WM_ACTION_ABAT1			(WM_USER+31)
#define WM_ACTION_ABAT2			(WM_USER+32)
#define WM_ACTION_ABAT3			(WM_USER+33)
#define WM_ACTION_ABAT4			(WM_USER+34)
#define WM_ACTION_ABAT5			(WM_USER+35)
#define WM_ACTION_ABAT6			(WM_USER+36)
#define WM_ACTION_BUILD1		(WM_USER+37)
#define WM_ACTION_BUILD2		(WM_USER+38)
#define WM_ACTION_BUILD3		(WM_USER+39)
#define WM_ACTION_BUILD4		(WM_USER+40)
#define WM_ACTION_BUILD5		(WM_USER+41)
#define WM_ACTION_BUILD6		(WM_USER+42)
#define WM_ACTION_STOP			(WM_USER+43)
#define WM_ACTION_CARRY			(WM_USER+44)
#define WM_ACTION_DEPOSE		(WM_USER+45)
#define WM_ACTION_ROC1			(WM_USER+46)
#define WM_ACTION_ROC2			(WM_USER+47)
#define WM_ACTION_ROC3			(WM_USER+48)
#define WM_ACTION_ROC4			(WM_USER+49)
#define WM_ACTION_ROC5			(WM_USER+50)
#define WM_ACTION_ROC6			(WM_USER+51)
#define WM_ACTION_ROC7			(WM_USER+52)
#define WM_ACTION_MUR			(WM_USER+53)
#define WM_ACTION_CULTIVE		(WM_USER+54)
#define WM_ACTION_CULTIVE2		(WM_USER+55)
#define WM_ACTION_MANGE			(WM_USER+56)
#define WM_ACTION_MAKE			(WM_USER+57)
#define WM_ACTION_BUILD			(WM_USER+58)
#define WM_ACTION_PALIS			(WM_USER+59)
#define WM_ACTION_NEWBLUPI		(WM_USER+60)
#define WM_ACTION_PONTE			(WM_USER+61)
#define WM_ACTION_PONTS			(WM_USER+62)
#define WM_ACTION_PONTO			(WM_USER+63)
#define WM_ACTION_PONTN			(WM_USER+64)
#define WM_ACTION_PONTEL		(WM_USER+65)
#define WM_ACTION_PONTSL		(WM_USER+66)
#define WM_ACTION_PONTOL		(WM_USER+67)
#define WM_ACTION_PONTNL		(WM_USER+68)
#define WM_ACTION_TOUR			(WM_USER+69)
#define WM_ACTION_CARRY2		(WM_USER+70)
#define WM_ACTION_DEPOSE2		(WM_USER+71)
#define WM_ACTION_MANGE2		(WM_USER+72)
#define WM_ACTION_BOIT			(WM_USER+73)
#define WM_ACTION_BOIT2			(WM_USER+74)
#define WM_ACTION_LABO			(WM_USER+75)
#define WM_ACTION_FLEUR1		(WM_USER+76)
#define WM_ACTION_FLEUR2		(WM_USER+77)
#define WM_ACTION_DYNAMITE		(WM_USER+78)
#define WM_ACTION_DYNAMITE2		(WM_USER+79)
#define WM_ACTION_T_DYNAMITE	(WM_USER+80)
#define WM_ACTION_FLEUR3		(WM_USER+81)
#define WM_ACTION_R_BUILD1		(WM_USER+82)
#define WM_ACTION_R_BUILD2		(WM_USER+83)
#define WM_ACTION_R_BUILD3		(WM_USER+84)
#define WM_ACTION_R_BUILD4		(WM_USER+85)
#define WM_ACTION_R_MAKE1		(WM_USER+86)
#define WM_ACTION_R_MAKE2		(WM_USER+87)
#define WM_ACTION_R_MAKE3		(WM_USER+88)
#define WM_ACTION_R_MAKE4		(WM_USER+89)
#define WM_ACTION_R_BUILD5		(WM_USER+90)
#define WM_ACTION_R_MAKE5		(WM_USER+91)
#define WM_ACTION_BATEAUE		(WM_USER+92)
#define WM_ACTION_BATEAUS		(WM_USER+93)
#define WM_ACTION_BATEAUO		(WM_USER+94)
#define WM_ACTION_BATEAUN		(WM_USER+95)
#define WM_ACTION_BATEAUDE		(WM_USER+96)
#define WM_ACTION_BATEAUDS		(WM_USER+97)
#define WM_ACTION_BATEAUDO		(WM_USER+98)
#define WM_ACTION_BATEAUDN		(WM_USER+99)
#define WM_ACTION_BATEAUAE		(WM_USER+100)
#define WM_ACTION_BATEAUAS		(WM_USER+101)
#define WM_ACTION_BATEAUAO		(WM_USER+102)
#define WM_ACTION_BATEAUAN		(WM_USER+103)
#define WM_ACTION_MJEEP			(WM_USER+104)
#define WM_ACTION_DJEEP			(WM_USER+105)
#define WM_ACTION_DRAPEAU		(WM_USER+106)
#define WM_ACTION_DRAPEAU2		(WM_USER+107)
#define WM_ACTION_DRAPEAU3		(WM_USER+108)
#define WM_ACTION_EXTRAIT		(WM_USER+109)
#define WM_ACTION_FABJEEP		(WM_USER+110)
#define WM_ACTION_FABMINE		(WM_USER+111)
#define WM_ACTION_MINE			(WM_USER+112)
#define WM_ACTION_MINE2			(WM_USER+113)
#define WM_ACTION_R_BUILD6		(WM_USER+114)
#define WM_ACTION_R_MAKE6		(WM_USER+115)
#define WM_ACTION_E_RAYON		(WM_USER+116)
#define WM_ACTION_ELECTRO		(WM_USER+117)
#define WM_ACTION_ELECTROm		(WM_USER+118)
#define WM_ACTION_GRILLE		(WM_USER+119)
#define WM_ACTION_MAISON		(WM_USER+120)
#define WM_ACTION_FABDISC		(WM_USER+121)
#define WM_ACTION_A_MORT		(WM_USER+122)
#define WM_ACTION_REPEAT		(WM_USER+123)
#define WM_ACTION_TELEPORTE00	(WM_USER+124)
#define WM_ACTION_TELEPORTE10	(WM_USER+125)
#define WM_ACTION_TELEPORTE01	(WM_USER+126)
#define WM_ACTION_TELEPORTE11	(WM_USER+127)
#define WM_ACTION_FABARMURE		(WM_USER+128)
#define WM_ACTION_MARMURE		(WM_USER+129)
#define WM_ACTION_DARMURE		(WM_USER+130)

#define WM_BUTTON0				(WM_USER+200)
#define WM_BUTTON1				(WM_USER+201)
#define WM_BUTTON2				(WM_USER+202)
#define WM_BUTTON3				(WM_USER+203)
#define WM_BUTTON4				(WM_USER+204)
#define WM_BUTTON5				(WM_USER+205)
#define WM_BUTTON6				(WM_USER+206)
#define WM_BUTTON7				(WM_USER+207)
#define WM_BUTTON8				(WM_USER+208)
#define WM_BUTTON9				(WM_USER+209)
#define WM_BUTTON10				(WM_USER+210)
#define WM_BUTTON11				(WM_USER+211)
#define WM_BUTTON12				(WM_USER+212)
#define WM_BUTTON13				(WM_USER+213)
#define WM_BUTTON14				(WM_USER+214)
#define WM_BUTTON15				(WM_USER+215)
#define WM_BUTTON16				(WM_USER+216)
#define WM_BUTTON17				(WM_USER+217)
#define WM_BUTTON18				(WM_USER+218)
#define WM_BUTTON19				(WM_USER+219)
#define WM_BUTTON20				(WM_USER+220)
#define WM_BUTTON21				(WM_USER+221)
#define WM_BUTTON22				(WM_USER+222)
#define WM_BUTTON23				(WM_USER+223)
#define WM_BUTTON24				(WM_USER+224)
#define WM_BUTTON25				(WM_USER+225)
#define WM_BUTTON26				(WM_USER+226)
#define WM_BUTTON27				(WM_USER+227)
#define WM_BUTTON28				(WM_USER+228)
#define WM_BUTTON29				(WM_USER+229)
#define WM_BUTTON30				(WM_USER+230)
#define WM_BUTTON31				(WM_USER+231)
#define WM_BUTTON32				(WM_USER+232)
#define WM_BUTTON33				(WM_USER+233)
#define WM_BUTTON34				(WM_USER+234)
#define WM_BUTTON35				(WM_USER+235)
#define WM_BUTTON36				(WM_USER+236)
#define WM_BUTTON37				(WM_USER+237)
#define WM_BUTTON38				(WM_USER+238)
#define WM_BUTTON39				(WM_USER+239)

#define WM_READ0				(WM_USER+300)
#define WM_READ1				(WM_USER+301)
#define WM_READ2				(WM_USER+302)
#define WM_READ3				(WM_USER+303)
#define WM_READ4				(WM_USER+304)
#define WM_READ5				(WM_USER+305)
#define WM_READ6				(WM_USER+306)
#define WM_READ7				(WM_USER+307)
#define WM_READ8				(WM_USER+308)
#define WM_READ9				(WM_USER+309)

#define WM_WRITE0				(WM_USER+310)
#define WM_WRITE1				(WM_USER+311)
#define WM_WRITE2				(WM_USER+312)
#define WM_WRITE3				(WM_USER+313)
#define WM_WRITE4				(WM_USER+314)
#define WM_WRITE5				(WM_USER+315)
#define WM_WRITE6				(WM_USER+316)
#define WM_WRITE7				(WM_USER+317)
#define WM_WRITE8				(WM_USER+318)
#define WM_WRITE9				(WM_USER+319)

#define WM_PHASE_INIT			(WM_USER+500)
#define WM_PHASE_PLAY			(WM_USER+501)
#define WM_PHASE_BUILD			(WM_USER+502)
#define WM_PHASE_READ			(WM_USER+503)
#define WM_PHASE_WRITE			(WM_USER+504)
#define WM_PHASE_INFO			(WM_USER+505)
#define WM_PHASE_BUTTON			(WM_USER+506)
#define WM_PHASE_TERM			(WM_USER+507)
#define WM_PHASE_WIN			(WM_USER+508)
#define WM_PHASE_LOST			(WM_USER+509)
#define WM_PHASE_STOP			(WM_USER+510)
#define WM_PHASE_SETUP			(WM_USER+511)
#define WM_PHASE_MUSIC			(WM_USER+512)
#define WM_PHASE_PLAYMOVIE		(WM_USER+513)
#define WM_PHASE_WINMOVIE		(WM_USER+514)
#define WM_PHASE_SCHOOL			(WM_USER+515)
#define WM_PHASE_MISSION		(WM_USER+516)
#define WM_PHASE_LASTWIN		(WM_USER+517)
#define WM_PHASE_WRITEp			(WM_USER+518)
#define WM_PHASE_SETUPp			(WM_USER+519)
#define WM_PHASE_REGION			(WM_USER+520)
#define WM_PHASE_INSERT			(WM_USER+521)
#define WM_PHASE_HISTORY0		(WM_USER+522)
#define WM_PHASE_HISTORY1		(WM_USER+523)
#define WM_PHASE_HELP			(WM_USER+524)
#define WM_PHASE_H0MOVIE		(WM_USER+525)
#define WM_PHASE_H1MOVIE		(WM_USER+526)
#define WM_PHASE_H2MOVIE		(WM_USER+527)
#define WM_PHASE_TESTCD			(WM_USER+528)
#define WM_PHASE_MANUEL			(WM_USER+529)
#define WM_PHASE_PRIVATE		(WM_USER+530)
#define WM_PHASE_UNDO			(WM_USER+531)
#define WM_PHASE_BYE			(WM_USER+532)
#define WM_PHASE_SKILL1			(WM_USER+533)
#define WM_PHASE_SKILL2			(WM_USER+534)
#define WM_PHASE_DEMO			(WM_USER+535)
#define WM_PHASE_INTRO1			(WM_USER+536)
#define WM_PHASE_INTRO2			(WM_USER+537)
/* SB1 phase values beyond the Planet Blupi base set. */
#define WM_PHASE_NAMEGAMER 1527
#define WM_PHASE_CLEARGAMER 1528
#define WM_PHASE_PLAYTEST 1530
#define WM_PHASE_DOPLAY 1539
#define WM_PHASE_QUITPLAYTEST 1542
#define WM_PHASE_GAMER 1543
#define WM_PHASE_WINDESIGN 1544
#define WM_PHASE_LOSTDESIGN 1545
#define WM_PHASE_WINMOVIEDESIGN 1546
#define WM_PHASE_LOSTMULTI 1547
#define WM_PHASE_WINMULTI 1548
#define WM_PHASE_WINMOVIEMULTI 1549
#define WM_PHASE_NAMEDESIGN 1555
#define WM_PHASE_WRITEDESIGN 1556
#define WM_PHASE_DONAMEGAMER 1557
#define WM_PHASE_DOCLEARGAMER 1558
#define WM_PHASE_DONAMEDESIGN 1559
#define WM_PHASE_DOWRITEDESIGN 1560
#define WM_PHASE_DOREADDESIGN 1561
#define WM_PHASE_DOCLEARDESIGN 1562
#define WM_PHASE_SERVICE 1563
#define WM_PHASE_DP_DOSERVICE 1564
#define WM_PHASE_DP_CANCELSERVICE 1565
#define WM_PHASE_SESSION 1566
#define WM_PHASE_DP_JOIN 1567
#define WM_PHASE_DP_CREATELOBBY 1568
#define WM_PHASE_DP_REFRESH 1569
#define WM_PHASE_DP_CANCELSESSION 1570
#define WM_PHASE_MULTI 1571
#define WM_PHASE_DP_STARTMULTI 1572
#define WM_PHASE_DP_CANCELMULTI 1573
#define WM_PHASE_CREATE 1574
#define WM_PHASE_DP_DOCREATE 1575
#define WM_PHASE_DP_CANCELCREATE 1576
#define WM_PHASE_GAMEWRITE 1578
#define WM_PHASE_GAMEREAD 1580
#define WM_PHASE_QUITPLAY 1581
#define WM_PHASE_DOQUIT 1582


#define WM_PREV					(WM_USER+600)
#define WM_NEXT					(WM_USER+601)
#define WM_MOVIE				(WM_USER+602)



// Types de gestion de la souris.

#define MOUSETYPEGRA			1
#define MOUSETYPEWIN			2
#define MOUSETYPEWINPOS			3



// Conditions pour gagner.

typedef struct
{
	short	bHachBlupi;		// blupi sur dalle hachurée
	short	bHachPlanche;	// planches sur dalle hachurée
	short	bStopFire;		// feu éteint
	short	nbMinBlupi;		// nb de blupi nécessaires
	short	nbMaxBlupi;		// nb de blupi nécessaires
	short	bHomeBlupi;		// blupi à la maison
	short	bKillRobots;	// plus d'ennemis
	short	bHachTomate;	// tomates sur dalle hachurée
	short	bHachMetal;		// métal sur dalle hachurée
	short	bHachRobot;		// robot sur dalle hachurée
	short	reserve[14];
}
Term;


/* SB1 engine names absent from the earlier Planet Blupi header. */
#define CHDECOR	3
#define CHEXPLO	9
#define CHELEMENT	10
#define CHBLUPI1	11
#define CHBLUPI2	12
#define CHBLUPI3	13
#define CHTEMP	14
#define CHMAX	14
#define PK_MOVE	1
#define PK_ABANDON	8
#define PK_LOST	9
#define PK_DIE	10
#define PK_PAUSE	12
#define PK_OBJECTSTART	20
#define PK_OBJECTDELETE	21
#define PK_MODIFDECOR	30
#define PK_PLAYSOUND	40
#define PK_STOPSOUND	41
#define PK_ASCENSEURSTART	50
#define PK_ASCENSEUREND	51
#define PK_STOPCLOUD	60
#define MESS_CLOSE	1
#define MESS_JOIN	2
#define MESS_LEAVE	3
#define MESS_LOBBY	4
#define MESS_TEAM	5
#define MESS_READY	6
#define MESS_START	7
#define MESS_CHAT	11
#define DIR_LEFT	1
#define DIR_RIGHT	2
#define SEC_SHIELD	1
#define SEC_POWER	2
#define SEC_CLOUD	3
#define SEC_HIDE	4
#define STEP_STOPSTART	1
#define STEP_ADVANCE	2
#define STEP_STOPEND	3
#define STEP_RECEDE	4
#define DECOR_EXPLO1	1
#define DECOR_EXPLO2	2
#define DECOR_EXPLO3	3
#define DECOR_EXPLO4	4
#define DECOR_BALLOON	5
#define KEY_NONE	0
#define KEY_LEFT	1
#define KEY_RIGHT	2
#define KEY_UP	4
#define KEY_DOWN	8
#define KEY_JUMP	16
#define KEY_FIRE	32
#define WM_DECOR9	1052
#define WM_DECOR10	1053
#define WM_DECOR11	1054
#define WM_DIMS1	1265
#define WM_DIMS2	1266
#define WM_DIMS3	1267
#define WM_DIMS4	1268
#define WM_PHASE_1531	1531
#define WM_PHASE_1538	1538
#define WM_PHASE_READDESIGN	1560
#define WM_PHASE_CLEARDESIGN	1561
#define WM_PHASE_GAMEREADp	1585
#define IC_BT_TECH	0
#define IC_BT_SPECIALTECH	1
#define IC_BT_TRIANGULARTECH	2
#define IC_BT_9	3
#define IC_BT_10	4
#define IC_BT_BLUPI	5
#define IC_BT_CANCEL	6
#define IC_BT_TREASURE	7
#define IC_BT_BOMB	8
#define IC_BT_BOMBUP	9
#define IC_BT_EGG	10
#define IC_BT_GOAL	11
#define IC_BT_CANCEL2	12
#define IC_BT_HORIZONTAL	13
#define IC_BT_VERTICAL	14
#define IC_BT_HORIZONTALVERTICAL	15
#define IC_BT_BULLDOZER	16
#define IC_BT_CASE	17
#define IC_BT_ACCEPT	18
#define IC_BT_LIFTFAST	19
#define IC_BT_LIFTNORMAL	20
#define IC_BT_LIFTSLOW	21
#define IC_BT_ROCK	22
#define IC_BT_WOODENWALL	23
#define IC_BT_HELICOPTER	24
#define IC_BT_DREAM	25
#define IC_BT_LAVA	26
#define IC_BT_SECRETTECH	27
#define IC_BT_LIFTFASTSLOW	28
#define IC_BT_PALMTREE	29
#define IC_BT_SUPPORT	30
#define IC_BT_TREE	31
#define IC_BT_VEGETATION	32
#define IC_BT_HOUSE	33
#define IC_BT_MECH	34
#define IC_BT_SPECIALMECH	35
#define IC_BT_DELETE	36
#define IC_BT_BIGMECH	37
#define IC_BT_WATER	38
#define IC_BT_GRASS	39
#define IC_BT_STOP	40
#define IC_BT_WIND	41
#define IC_BT_FAN	42
#define IC_BT_CREATE	43
#define IC_BT_MUSIC	44
#define IC_BT_JOIN	45
#define IC_BT_NAME	46
#define IC_BT_SETUP	47
#define IC_BT_PLAY	48
#define IC_BT_DESIGN	49
#define IC_BT_ARROWLEFT	50
#define IC_BT_ARROWRIGHT	51
#define IC_BT_READ	52
#define IC_BT_WRITE	53
#define IC_BT_MOVINGBOMB	54
#define IC_BT_FISH	55
#define IC_BT_BAR	56
#define IC_BT_JEEP	57
#define IC_BT_FURNITURE	58
#define IC_BT_TRIANGULARROCK	59
#define IC_BT_BIRD	60
#define IC_BT_TINY	61
#define IC_BT_LEVELSTART	62
#define IC_BT_DOOR	63
#define IC_BT_KEY	64
#define IC_BT_FACADE	65
#define IC_BT_ROOF	66
#define IC_BT_SECRETMECH	67
#define IC_BT_SECRETROCK	68
#define IC_BT_MARINE	69
#define IC_BT_SPRING	70
#define IC_BT_SKATEBOARD	71
#define IC_BT_ARROWUP	72
#define IC_BT_ARROWDOWN	73
#define IC_BT_SHIELD	74
#define IC_BT_LOLLIPOP	75
#define IC_BT_SECRETCASE	76
#define IC_BT_CONTINUE	77
#define IC_BT_HALT	78
#define IC_BT_INVISIBLEBLOCK	79
#define IC_BT_KIDSSTUFF	80
#define IC_BT_SPECIALKIDSSTUFF	81
#define IC_BT_BIGKIDSSTUFF	82
#define IC_BT_MULTI	83
#define IC_BT_DEMO	84
#define IC_BT_TANK	85
#define IC_BT_HELP	86
#define IC_BT_DYNAMITE	87
#define IC_BT_GLUE	88
#define IC_BT_DRINK	89
#define IC_BT_NOTREADY	90
#define IC_BT_READY	91
#define IC_BT_PERSO	92
#define IC_BT_CHARGER	93
#define IC_BT_SEND	94
#define IC_BT_HELICOENEMY	95
#define IC_BT_BLUPI1	96
#define IC_BT_BLUPI2	97
#define IC_BT_BLUPI3	98
#define IC_BT_TANKENEMY	99
#define IC_BT_CHEESE	100
#define IC_BT_INVERTER	101
#define IC_BT_PERSO0	102
#define IC_BT_PERSO1	103
#define IC_BT_PERSO2	104
#define IC_BT_PERSO3	105
#define IC_BT_PIPES	106
#define IC_BT_HOMINGBOMB	107
#define IC_BT_TEAM1	108
#define IC_BT_TEAM2	109
#define IC_BT_TEAM3	110
#define IC_BT_TEAM4	111
#define IC_BT_BRICKS	112
#define IC_BT_WASP	113
#define IC_BT_TEAMLITTLE1	114
#define IC_BT_TEAMLITTLE2	115
#define IC_BT_TEAMLITTLE3	116
#define IC_BT_TEAMLITTLE4	117
#define IC_BT_CAVE	118
#define IC_BT_HOVERCRAFT	119
#define IC_BT_LIGHTNING	120
#define IC_BT_CONVEYOR	121
#define IC_BT_CRUSHER	122
#define IC_BT_VANISH	123
#define IC_BT_TELEPORTER	124
#define IC_BT_KEYCOLOR	125
#define IC_BT_DOORCOLOR	126
#define IC_BT_SECRETCAVE	127
#define IC_BT_SECRETCHEESE	128
#define IC_BT_SLIME	129
#define IC_BT_BIGSLIME	130
#define IC_BT_SLIMEFLOOR	131
#define IC_BT_SAW	132
#define IC_BT_LOWCOLORBACK	133
#define IC_BT_TRUECOLORBACK	134
#define IC_BT_LOWCOLORDECOR	135
#define IC_BT_TRUECOLORDECOR	136
#define IC_BT_PALACE	137
#define IC_BT_PALACEELEMENT	138
#define IC_BT_BIGPALACE	139
#define IC_BT_SLIMECREATURE	140
#define IC_BT_BRIDGE	141
#define IC_BT_SPIDERWEB	142
#define IC_BT_SLIMECEILING	143
#define IC_BT_TREASUREDOOR	144
#define _VALUE	FALSE
#define DIMEXPLOX	128
#define DIMEXPLOY	128
#define DIMDECORX	640
#define DIMDECORY	480
#define ACTION_MARCH	2
#define ACTION_TURN	3
#define ACTION_JUMP	4
#define ACTION_AIR	5
#define ACTION_DOWN	6
#define ACTION_UP	7
#define ACTION_VERTIGO	8
#define ACTION_RECEDE	9
#define ACTION_ADVANCE	10
#define ACTION_CLEAR1	11
#define ACTION_SET	12
#define ACTION_WIN	13
#define ACTION_PUSH	14
#define ACTION_STOPHELICO	15
#define ACTION_MARCHHELICO	16
#define ACTION_TURNHELICO	17
#define ACTION_STOPNAGE	18
#define ACTION_MARCHNAGE	19
#define ACTION_TURNNAGE	20
#define ACTION_STOPSURF	21
#define ACTION_MARCHSURF	22
#define ACTION_TURNSURF	23
#define ACTION_DROWN	24
#define ACTION_STOPJEEP	25
#define ACTION_MARCHJEEP	26
#define ACTION_TURNJEEP	27
#define ACTION_STOPPOP	28
#define ACTION_POP	29
#define ACTION_BYE	30
#define ACTION_STOPSUSPEND	31
#define ACTION_MARCHSUSPEND	32
#define ACTION_TURNSUSPEND	33
#define ACTION_JUMPSUSPEND	34
#define ACTION_HIDE	35
#define ACTION_JUMPAIE	36
#define ACTION_STOPSKATE	37
#define ACTION_MARCHSKATE	38
#define ACTION_TURNSKATE	39
#define ACTION_JUMPSKATE	40
#define ACTION_AIRSKATE	41
#define ACTION_TAKESKATE	42
#define ACTION_DEPOSESKATE	43
#define ACTION_OUF1a	44
#define ACTION_OUF1b	45
#define ACTION_OUF2	46
#define ACTION_OUF3	47
#define ACTION_OUF4	48
#define ACTION_SUCETTE	49
#define ACTION_STOPTANK	50
#define ACTION_MARCHTANK	51
#define ACTION_TURNTANK	52
#define ACTION_FIRETANK	53
#define ACTION_GLU	54
#define ACTION_DRINK	55
#define ACTION_CHARGE	56
#define ACTION_HELICOGLU	58
#define ACTION_TURNAIR	59
#define ACTION_STOPMARCH	60
#define ACTION_STOPJUMP	61
#define ACTION_STOPJUMPh	62
#define ACTION_MOCKERY	63
#define ACTION_MOCKERYi	64
#define ACTION_OUF5	65
#define ACTION_BALLOON	66
#define ACTION_STOPOVER	67
#define ACTION_MARCHOVER	68
#define ACTION_TURNOVER	69
#define ACTION_RECEDEq	70
#define ACTION_ADVANCEq	71
#define ACTION_STOPECRASE	72
#define ACTION_MARCHECRASE	73
#define ACTION_TELEPORTE	74
#define ACTION_CLEAR2	75
#define ACTION_CLEAR3	76
#define ACTION_CLEAR4	77
#define ACTION_CLEAR5	78
#define ACTION_CLEAR6	79
#define ACTION_CLEAR7	80
#define ACTION_CLEAR8	81
#define ACTION_SWITCH	82
#define ACTION_MOCKERYp	83
#define ACTION_NON	84
#define ACTION_SLOWDOWNSKATE	85
#define ACTION_TAKEDYNAMITE	86
#define ACTION_PUTDYNAMITE	87
#define TYPE_ASCENSEUR	1
#define TYPE_BOMBEDOWN	2
#define TYPE_BOMBEUP	3
#define TYPE_BULLDOZER	4
#define TYPE_TRESOR	5
#define TYPE_EGG	6
#define TYPE_GOAL	7
#define TYPE_EXPLO1	8
#define TYPE_EXPLO2	9
#define TYPE_EXPLO3	10
#define TYPE_EXPLO4	11
#define TYPE_CAISSE	12
#define TYPE_HELICO	13
#define TYPE_PLOUF	14
#define TYPE_BLUP	15
#define TYPE_BOMBEMOVE	16
#define TYPE_POISSON	17
#define TYPE_TOMATES	18
#define TYPE_JEEP	19
#define TYPE_OISEAU	20
#define TYPE_CLE	21
#define TYPE_DOOR	22
#define TYPE_BALLE	23
#define TYPE_SKATE	24
#define TYPE_SHIELD	25
#define TYPE_POWER	26
#define TYPE_MAGICTRACK	27
#define TYPE_TANK	28
#define TYPE_BULLET	29
#define TYPE_DRINK	30
#define TYPE_CHARGE	31
#define TYPE_BLUPIHELICO	32
#define TYPE_BLUPITANK	33
#define TYPE_GLU	34
#define TYPE_TIPLOUF	35
#define TYPE_POLLUTION	36
#define TYPE_CLEAR	37
#define TYPE_ELECTRO	38
#define TYPE_TRESORTRACK	39
#define TYPE_INVERT	40
#define TYPE_INVERTSTART	41
#define TYPE_INVERTSTOP	42
#define TYPE_INVERTSPIN	43
#define TYPE_GUEPE	44
#define TYPE_OVER	46
#define TYPE_ASCENSEURs	47
#define TYPE_ASCENSEURsi	48
#define TYPE_CLE1	49
#define TYPE_CLE2	50
#define TYPE_CLE3	51
#define TYPE_BRIDGE	52
#define TYPE_TENTACULE	53
#define TYPE_CREATURE	54
#define TYPE_DYNAMITE	55
#define TYPE_DYNAMITEf	56
#define TYPE_SHIELDTRACK	57
#define TYPE_HIDETRACK	58
#define TYPE_EXPLO5	90
#define TYPE_EXPLO6	91
#define TYPE_EXPLO7	92
#define TYPE_EXPLO8	93
#define TYPE_EXPLO9	94
#define TYPE_EXPLO10	95
#define TYPE_BOMBEFOLLOW1	96
#define TYPE_BOMBEFOLLOW2	97
#define TYPE_SPLOUTCH1	98
#define TYPE_SPLOUTCH2	99
#define TYPE_SPLOUTCH3	100
#define TYPE_BOMBEPERSO1	200
#define TYPE_BOMBEPERSO2	201
#define TYPE_BOMBEPERSO3	202
#define TYPE_BOMBEPERSO4	203
#define SOUND_JUMP1	1
#define SOUND_JUMP2	2
#define SOUND_JUMPEND	3
#define SOUND_JUMPTOC	4
#define SOUND_TURN	5
#define SOUND_VERTIGO	6
#define SOUND_DOWN	7
#define SOUND_FALL	8
#define SOUND_NEW	9
#define SOUND_BOUM	10
#define SOUND_TRESOR	11
#define SOUND_EGG	12
#define SOUND_ENDKO	13
#define SOUND_ENDOK	14
#define SOUND_HELICOSTART	15
#define SOUND_HELICOHIGH	16
#define SOUND_HELICOSTOP	17
#define SOUND_HELICOLOW	18
#define SOUND_LASTTRESOR	19
#define SOUND_UP	20
#define SOUND_LOOKUP	21
#define SOUND_JUMP0	22
#define SOUND_BLUP	24
#define SOUND_SURF	25
#define SOUND_DROWN	26
#define SOUND_ERROR	27
#define SOUND_JEEPSTART	28
#define SOUND_JEEPHIGH	29
#define SOUND_JEEPSTOP	30
#define SOUND_JEEPLOW	31
#define SOUND_BYE	32
#define SOUND_DOOR	33
#define SOUND_SUSPENDTOC	34
#define SOUND_SUSPENDJUMP	35
#define SOUND_SINGE	36
#define SOUND_PATIENT	37
#define SOUND_PUSH	38
#define SOUND_POP	39
#define SOUND_JUMPAIE	40
#define SOUND_RESSORT	41
#define SOUND_STARTSHIELD	42
#define SOUND_STOPSHIELD	43
#define SOUND_STARTPOWER	44
#define SOUND_STOPPOWER	45
#define SOUND_OUF1	46
#define SOUND_OUF2	47
#define SOUND_OUF3	48
#define SOUND_OUF4	49
#define SOUND_SUCETTE	50
#define SOUND_GLU	51
#define SOUND_FIREOK	52
#define SOUND_FIREKO	53
#define SOUND_TAKEGLU	54
#define SOUND_STARTCLOUD	55
#define SOUND_STOPCLOUD	56
#define SOUND_DRINK	57
#define SOUND_CHARGE	58
#define SOUND_ELECTRO	59
#define SOUND_PERSOTAKE	60
#define SOUND_PERSOPOSE	61
#define SOUND_STARTHIDE	62
#define SOUND_STOPHIDE	63
#define SOUND_TIPLOUF	64
#define SOUND_MOCKERY	65
#define SOUND_INVERTSTART	66
#define SOUND_INVERTSTOP	67
#define SOUND_OVERSTOP	68
#define SOUND_BLITZ	69
#define SOUND_ECRASE	70
#define SOUND_BRIDGE1	72
#define SOUND_BRIDGE2	73
#define SOUND_ANGEL	74
#define SOUND_SWITCHOFF	76
#define SOUND_SWITCHON	77
#define SOUND_JUMPENDb	78
#define SOUND_JUMPTOCb	79
#define SOUND_JUMPENDm	80
#define SOUND_JUMPTOCm	81
#define SOUND_JUMPENDg	82
#define SOUND_JUMPTOCg	83
#define SOUND_JUMPENDo	84
#define SOUND_JUMPTOCo	85
#define SOUND_JUMPENDk	86
#define SOUND_JUMPTOCk	87
#define SOUND_JUMPENDf	88
#define SOUND_JUMPTOCf	89
#define SOUND_JUMPENDh	90
#define SOUND_JUMPTOCh	91
#define SOUND_FOLLOW	92
#define SPRITE_DISABLE	13
#define CLE_RED	(1 << 0)
#define CLE_GREEN	(1 << 1)
#define CLE_BLUE	(1 << 2)
#define JAUGE_AIR	0
#define JAUGE_POWER	1

namespace Object {
	enum Object {
		Empty = -1,
		Hili,
		TechSquare_1,
		TechSquare_2,
		TechSquare_3,
		TechSquare_4,
		TechSquare_5,
		TechSquare_6,
		TechSquare_7,
		TechSquare_8,
		TechSquare_9,
		TechSquare_10,
		TechSquare_11,
		TechSquare_12,
		TechSquare_13,
		TechSquare_14,
		TechTriRight_1,
		TechTriRight_2, // 16
		TechTriLeft_1,
		TechTriLeft_2,
		TechSpecial_1,
		TechSpecial_2,
		TechSpecial_3,
		TechSecret_1,
		TechSpecial_4,
		TechSpecial_5,
		TechSpecial_6,
		TechSpecial_7,
		TechSpecial_8,
		TechSpecial_9,
		TechSpecial_10,
		LiftPoint_1,
		LiftPoint_2,
		Caisse_1, // 32
		Caisse_2,
		Caisse_3,
		RockSquare_1,
		RockSquare_2,
		RockSquare_3,
		RockSquare_4,
		RockSquare_5,
		RockSquare_6,
		WoodWall_1,
		WoodWall_2,
		WoodWall_3,
		WoodWall_4,
		WoodWall_5,
		WoodWall_6,
		WoodWall_7,
		Dream_1, // 48
		Dream_2,
		Dream_3,
		Dream_4,
		Dream_5,
		Dream_6,
		Dream_7,
		Dream_8,
		Dream_9,
		Dream_10,
		Dream_11,
		Dream_12,
		Dream_13,
		Dream_14,
		Dream_15,
		Dream_16,
		Dream_17,  // 64
		Dream_18,
		Dream_19,
		Dream_20,
		Lava_1,
		Lava_2,
		Lava_3,
		Lava_4,
		Lava_5,
		TechSecret_2,
		RockTriLeft_1,
		RockTriRight_1,
		Support_1,
		Support_2,
		MechSquare_1,
		MechSquare_2,
		MechSquare_3, // 80
		MechSquare_4,
		MechSquare_5,
		MechSquare_6,
		MechSquare_7,
		MechSecret_1,
		MechSecret_2,
		MechSpecial_1,
		MechSpecial_2,
		MechSpecial_3,
		MechSpecial_4,
		WaterSquare_1,
		WaterSurf_1,
		WaterSurf_2,
		WaterSurf_3,
		WaterSurf_4,
		WaterSquare_2, // 96
		WaterSquare_3,
		WaterSquare_4,
		Splash_1,
		Splash_2,
		Splash_3,
		Splash_4,
		Bubble_1,
		Bubble_2,
		Bubble_3,
		Bubble_4,
		GrassSquare_1,
		GrassLeft,
		GrassRight,
		WindLeft_1,
		WindLeft_2, // 112
		WindLeft_3,
		WindLeft_4,
		WindRight_1,
		WindRight_2,
		WindRight_3,
		WindRight_4,
		WindUp_1,
		WindUp_2,
		WindUp_3,
		WindUp_4,
		WindDown_1,
		WindDown_2,
		WindDown_3,
		WindDown_4,
		FanLeft_1,
		FanLeft_2, // 128
		FanLeft_3,
		FanRight_1,
		FanRight_2,
		FanRight_3,
		FanUp_1,
		FanUp_2,
		FanUp_3,
		FanDown_1,
		FanDown_2,
		FanDown_3,
		Bar_1,
		Bookshelf_1,
		Bookshelf_2,
		Bookshelf_3,
		Bookshelf_4,
		Bookshelf_5,
		RockSquare_7, // 144
		RockSquare_8,
		RockSquare_9,
		RockSquare_10,
		RockSquare_11,
		RockSquare_12,
		RockSquare_13,
		RockSquare_14,
		RockSquare_15,
		RockSquare_16,
		RockTriLeft_2,
		RockTriRight_2,
		RockSquare_17,
		GrassSquare_2,
		World_1,
		World_2,
		World_3, // 160
		World_4,
		World_5,
		World_6,
		World_7,
		World_8,
		WorldDone_1,
		WorldDone_2,
		WorldDone_3,
		WorldDone_4,
		WorldDone_5,
		WorldDone_6,
		WorldDone_7,
		WorldDone_8,
		Level_1,
		Level_2,
		Level_3, // 176
		Level_4,
		Level_5,
		Level_6,
		Level_7,
		Level_8,
		DoorLevel,
		DoorCourse,
		WorldBack,
		RockSecret,
		Facade_1,
		Facade_2,
		Facade_3,
		Facade_4,
		Facade_5,
		Facade_6,
		Facade_7, // 192
		Roof_1,
		Roof_2,
		Roof_3,
		Roof_4,
		Roof_5,
		Support_3,
		Support_4,
		Support_5,
		Support_6,
		Bar_2,
		Marine_1,
		Marine_2,
		Marine_3,
		Marine_4,
		Marine_5,
		Marine_6, // 208
		Spring_1,
		Spring_2,
		Spring_3,
		Spring_4,
		Spring_5,
		Invisible,
		KidsSquare_1,
		KidsSquare_2,
		KidsSquare_3,
		KidsSquare_4,
		KidsSquare_5,
		KidsSquare_6,
		KidsSquare_7,
		KidsSquare_8,
		KidsSpecial_1,
		KidsSpecial_2, // 224
		KidsSpecial_3,
		KidsSpecial_4,
		KidsSpecial_5,
		KidsSpecial_6,
		KidsSpecial_7,
		KidsSquare_9,
		KidsSquare_10,
		KidsSpecial_8,
		KidsSquare_11,
		KidsSquare_12,
		Support_7,
		Support_8,
		DitherMask,
		Charger_1,
		Charger_2,
		Charger_3, // 240
		Charger_4,
		Charger_5,
		Charger_6,
		Splash_5,
		Unk_F5,
		CheeseSquare_1,
		CheeseSquare_2,
		CheeseSquare_3,
		CheeseSquare_4,
		Pipes_1,
		Pipes_2,
		Pipes_3,
		Pipes_4,
		Pipes_5,
		Pipes_6,
		Pipes_7, // 256
		Pipes_8,
		Pipes_9,
		Pipes_10,
		Pipes_11,
		Bricks_1,
		Bricks_2,
		Bricks_3,
		CheeseSupport_1,
		CheeseSupport_2,
		CheeseSupport_3,
		CheeseSupport_4,
		CheeseSupport_5,
		CheeseSupport_6,
		CheeseSupport_7,
		CheeseSupport_8,
		CheeseSupport_9, // 272
		CheeseSupport_10,
		CheeseSupport_11,
		CheeseSupport_12,
		CheeseSupport_13,
		CheeseSupport_14,
		CheeseSupport_15,
		CheeseSupport_16,
		CheeseSupport_17,
		CheeseSupport_18,
		CheeseSupport_19,
		Bricks_4,
		Cave_1,
		CaveSupport_1,
		CaveSupport_2,
		CaveSupport_3,
		CaveSupport_4, // 288
		CaveSupport_5,
		CaveSupport_6,
		CaveSupport_7,
		CaveSupport_8,
		CaveSupport_9,
		CaveSupport_10,
		CaveSupport_11,
		CaveSupport_12,
		CaveSupport_13,
		CaveSupport_14,
		CaveSupport_15,
		CaveSupport_16,
		Cave_2,
		CaveSupport_17,
		CaveSupport_18,
		LightningMachine, // 304
		Lightning_1,
		Lightning_2,
		Lightning_3,
		Lightning_4,
		World_9,
		WorldDone_9,
		Conveyor_1,
		Conveyor_2,
		Conveyor_3,
		Conveyor_4,
		Conveyor_5,
		Conveyor_6,
		Crusher_1,
		Crusher_2,
		Crusher_3,
		Crusher_4, // 320
		Crusher_5,
		Crusher_6,
		Crusher_7,
		Vanish_1,
		Vanish_2,
		Vanish_3,
		Vanish_4,
		Vanish_5,
		Vanish_6,
		Teleporter_1,
		Teleporter_2,
		Teleporter_3,
		Teleporter_4,
		DoorRed,
		DoorGreen,
		DoorBlue, // 336
		CaveSecret,
		Cave_3,
		CheeseSecret,
		CheeseSquare_5,
		Slime_1,
		Slime_2,
		Slime_3,
		Slime_4,
		Slime_5,
		Slime_6,
		Slime_7,
		Slime_8,
		Slime_9,
		Slime_10,
		Slime_11,
		Slime_12, // 352
		Slime_13,
		Slime_14,
		Slime_15,
		Slime_16,
		Slime_17,
		Slime_18,
		Slime_19,
		Slime_20,
		Slime_21,
		Slime_22,
		Slime_23,
		FragileBridge_1,
		FragileBridge_2,
		FragileBridge_3,
		FragileBridge_4,
		FragileBridge_5, // 368
		FragileBridge_6,
		FragileBridge_7,
		FragileBridge_8,
		FragileBridge_9,
		SlimeTrapFloor_1,
		SlimeTrapFloor_2,
		Support_9,
		Support_10,
		Support_11,
		Saw_1,
		Saw_2,
		Saw_3,
		Saw_4,
		Saw_5,
		Saw_6,
		SawSwitchOn, // 384
		SawSwitchOff,
		Palace_1,
		Palace_2,
		Palace_3,
		Palace_4,
		Palace_5,
		Palace_6,
		Palace_7,
		Palace_8,
		Palace_9,
		Palace_10,
		Palace_11,
		Palace_12,
		PalaceElement_1,
		PalaceElement_2,
		PalaceElement_3, // 400
		SpiderWeb_1,
		SpiderWeb_2,
		SpiderWeb_3,
		SlimeTrapCeiling_1,
		SlimeTrapCeiling_2,
		SlimeTrapCeiling_3,
		SlimeTrapCeiling_4,
		SlimeTrapCeiling_5,
		SlimeTrapCeiling_6,
		SlimeTrapCeiling_7,
		World_10,
		World_11,
		World_12,
		World_13,
		World_14,
		WorldDone_10, // 416
		WorldDone_11,
		WorldDone_12,
		WorldDone_13,
		WorldDone_14,
		DoorTreasure_1,
		DoorTreasure_2,
		DoorTreasure_3,
		DoorTreasure_4,
		DoorTreasure_5,
		DoorTreasure_6,
		DoorTreasure_7,
		DoorTreasure_8,
		DoorTreasure_9,
		DoorTreasure_10,
		DoorTreasure_11,
		DoorTreasure_12,	// 432
		DoorTreasure_13,
		DoorTreasure_14,
		DoorTreasure_15,
		DoorTreasure_16,
		DoorTreasure_17,
		DoorTreasure_18,
		DoorTreasure_19,
		DoorTreasure_20
	};
}

// hack to mimic array literals in old c++
#if __cplusplus < 199711
#define RECT(A,B,C,D) MAKE_RECT(A,B,C,D)
#define POINT(X,Y) MAKE_POINT(X,Y)

inline RECT MAKE_RECT(int left, int top, int right, int bottom)
{
	RECT r = { left, top, right, bottom };
	return r;
};

inline POINT MAKE_POINT(int x, int y)
{
	POINT p = { x, y };
	return p;
};
#else
#define RECT(A,B,C,D) RECT{A,B,C,D}
#define POINT(X,Y) POINT{X,Y}
#endif

// helpers for POINT

POINT operator+(POINT p, const POINT& a)
{
	return POINT(a.x + p.x, a.y + p.y);
}

POINT operator-(POINT p, const POINT& a)
{
	return POINT(p.x - a.x, p.y - a.y);
}

POINT operator*(POINT p, const POINT& a)
{
	return POINT(a.x * p.x, a.y * p.y);
}

POINT operator/(POINT p, const POINT& a)
{
	return POINT(p.x / a.x, p.y / a.y);
}

POINT operator*(POINT p, const int& a)
{
	return POINT(p.x * a, p.y * a);
}

POINT operator/(POINT p, const int& a)
{
	return POINT(p.x / a, p.y / a);
}

POINT& operator+=(POINT& p, const POINT& a)
{
	p.x += a.x;
	p.y += a.y;
	return p;
}

POINT& operator-=(POINT& p, const POINT& a)
{
	p.x -= a.x;
	p.y -= a.y;
	return p;
}

POINT& operator*=(POINT& p, const POINT& a)
{
	p.x *= a.x;
	p.y *= a.y;
	return p;
}

POINT& operator/=(POINT& p, const POINT& a)
{
	p.x /= a.x;
	p.y /= a.y;
	return p;
}

POINT& operator*=(POINT& p, const int& a)
{
	p.x *= a;
	p.y *= a;
	return p;
}

POINT& operator/=(POINT& p, const int& a)
{
	p.x /= a;
	p.y /= a;
	return p;
}

bool operator!=(POINT a, const POINT& b)
{
	return a.x != b.x || a.y != b.y;
}

bool operator==(POINT a, const POINT& b)
{
	return a.x == b.x && a.y == b.y;
}

#ifdef SB1_EVENT_API
/* The event TU uses the phase sequence confirmed against the SB1 phase table. */
#undef WM_PHASE_INIT
#define WM_PHASE_INIT 1524
#undef WM_PHASE_PLAY
#define WM_PHASE_PLAY 1525
#undef WM_PHASE_BUILD
#define WM_PHASE_BUILD 1526
#undef WM_PHASE_NAMEGAMER
#define WM_PHASE_NAMEGAMER 1527
#undef WM_PHASE_CLEARGAMER
#define WM_PHASE_CLEARGAMER 1528
#undef WM_PHASE_INFO
#define WM_PHASE_INFO 1529
#undef WM_PHASE_PLAYTEST
#define WM_PHASE_PLAYTEST 1530
#undef WM_PHASE_1531
#define WM_PHASE_1531 1531
#undef WM_PHASE_WIN
#define WM_PHASE_WIN 1532
#undef WM_PHASE_LOST
#define WM_PHASE_LOST 1533
#undef WM_PHASE_SETUP
#define WM_PHASE_SETUP 1534
#undef WM_PHASE_MUSIC
#define WM_PHASE_MUSIC 1535
#undef WM_PHASE_PLAYMOVIE
#define WM_PHASE_PLAYMOVIE 1536
#undef WM_PHASE_WINMOVIE
#define WM_PHASE_WINMOVIE 1537
#undef WM_PHASE_1538
#define WM_PHASE_1538 1538
#undef WM_PHASE_DOPLAY
#define WM_PHASE_DOPLAY 1539
#undef WM_PHASE_LASTWIN
#define WM_PHASE_LASTWIN 1540
#undef WM_PHASE_SETUPp
#define WM_PHASE_SETUPp 1541
#undef WM_PHASE_REGION
#define WM_PHASE_REGION 1542
#undef WM_PHASE_INSERT
#define WM_PHASE_INSERT 1543
#undef WM_PHASE_QUITPLAYTEST
#define WM_PHASE_QUITPLAYTEST 1544
#undef WM_PHASE_GAMER
#define WM_PHASE_GAMER 1545
#undef WM_PHASE_WINDESIGN
#define WM_PHASE_WINDESIGN 1546
#undef WM_PHASE_LOSTDESIGN
#define WM_PHASE_LOSTDESIGN 1547
#undef WM_PHASE_WINMOVIEDESIGN
#define WM_PHASE_WINMOVIEDESIGN 1548
#undef WM_PHASE_LOSTMULTI
#define WM_PHASE_LOSTMULTI 1549
#undef WM_PHASE_WINMULTI
#define WM_PHASE_WINMULTI 1550
#undef WM_PHASE_WINMOVIEMULTI
#define WM_PHASE_WINMOVIEMULTI 1551
#undef WM_PHASE_TESTCD
#define WM_PHASE_TESTCD 1552
#undef WM_PHASE_MANUEL
#define WM_PHASE_MANUEL 1553
#undef WM_PHASE_PRIVATE
#define WM_PHASE_PRIVATE 1554
#undef WM_PHASE_UNDO
#define WM_PHASE_UNDO 1555
#undef WM_PHASE_BYE
#define WM_PHASE_BYE 1556
#undef WM_PHASE_DEMO
#define WM_PHASE_DEMO 1557
#undef WM_PHASE_NAMEDESIGN
#define WM_PHASE_NAMEDESIGN 1558
#undef WM_PHASE_WRITEDESIGN
#define WM_PHASE_WRITEDESIGN 1559
#undef WM_PHASE_READDESIGN
#define WM_PHASE_READDESIGN 1560
#undef WM_PHASE_CLEARDESIGN
#define WM_PHASE_CLEARDESIGN 1561
#undef WM_PHASE_DONAMEGAMER
#define WM_PHASE_DONAMEGAMER 1562
#undef WM_PHASE_DOCLEARGAMER
#define WM_PHASE_DOCLEARGAMER 1563
#undef WM_PHASE_DONAMEDESIGN
#define WM_PHASE_DONAMEDESIGN 1564
#undef WM_PHASE_DOWRITEDESIGN
#define WM_PHASE_DOWRITEDESIGN 1565
#undef WM_PHASE_DOREADDESIGN
#define WM_PHASE_DOREADDESIGN 1566
#undef WM_PHASE_DOCLEARDESIGN
#define WM_PHASE_DOCLEARDESIGN 1567
#undef WM_PHASE_SERVICE
#define WM_PHASE_SERVICE 1568
#undef WM_PHASE_DP_DOSERVICE
#define WM_PHASE_DP_DOSERVICE 1569
#undef WM_PHASE_DP_CANCELSERVICE
#define WM_PHASE_DP_CANCELSERVICE 1570
#undef WM_PHASE_SESSION
#define WM_PHASE_SESSION 1571
#undef WM_PHASE_DP_JOIN
#define WM_PHASE_DP_JOIN 1572
#undef WM_PHASE_DP_CREATELOBBY
#define WM_PHASE_DP_CREATELOBBY 1573
#undef WM_PHASE_DP_REFRESH
#define WM_PHASE_DP_REFRESH 1574
#undef WM_PHASE_DP_CANCELSESSION
#define WM_PHASE_DP_CANCELSESSION 1575
#undef WM_PHASE_MULTI
#define WM_PHASE_MULTI 1576
#undef WM_PHASE_DP_STARTMULTI
#define WM_PHASE_DP_STARTMULTI 1577
#undef WM_PHASE_DP_CANCELMULTI
#define WM_PHASE_DP_CANCELMULTI 1578
#undef WM_PHASE_CREATE
#define WM_PHASE_CREATE 1579
#undef WM_PHASE_DP_DOCREATE
#define WM_PHASE_DP_DOCREATE 1580
#undef WM_PHASE_DP_CANCELCREATE
#define WM_PHASE_DP_CANCELCREATE 1581
#undef WM_PHASE_STOP
#define WM_PHASE_STOP 1582
#undef WM_PHASE_HELP
#define WM_PHASE_HELP 1583
#undef WM_PHASE_GAMEWRITE
#define WM_PHASE_GAMEWRITE 1584
#undef WM_PHASE_GAMEREADp
#define WM_PHASE_GAMEREADp 1585
#undef WM_PHASE_GAMEREAD
#define WM_PHASE_GAMEREAD 1586
#undef WM_PHASE_QUITPLAY
#define WM_PHASE_QUITPLAY 1587
#undef WM_PHASE_DOQUIT
#define WM_PHASE_DOQUIT 1588
#undef WM_PHASE_INTRO1
#define WM_PHASE_INTRO1 1589
#undef WM_PHASE_INTRO2
#define WM_PHASE_INTRO2 1590
#endif

#endif /* DEF_H */

