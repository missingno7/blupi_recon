// Pointers to the protected members of the game's classes (CDecor, CEvent,
// CPixmap, ...), for port code that reads or writes their state without
// changing the historical headers.
//
//   #define DECOR_PARTS(X) X(m_posDecor) X(m_time)
//   BLUPI_MEMBER_ACCESS(DecorAccess, CDecor, DECOR_PARTS)
//   ... decor.*DecorAccess::m_posDecor ...
//
// A derived class makes the members public with using-declarations; a second
// struct holds pointers named like the members. (One struct holding both, the
// pointers named like the members they point to, is rejected by GCC 15: the
// name then denotes the pointer being declared.)
#ifndef BLUPI_GAME_MEMBER_ACCESS_H
#define BLUPI_GAME_MEMBER_ACCESS_H

#define BLUPI_MEMBER_USING_(m)   using Base_::m;
#define BLUPI_MEMBER_POINTER_(m) static constexpr auto m = &Members_::m;

#define BLUPI_MEMBER_ACCESS(Name, Class, LIST)                       \
	struct Name##Members_ : Class                                    \
	{                                                                \
		using Base_ = Class;                                         \
		LIST(BLUPI_MEMBER_USING_)                                    \
	};                                                               \
	struct Name                                                      \
	{                                                                \
		using Members_ = Name##Members_;                             \
		LIST(BLUPI_MEMBER_POINTER_)                                  \
	};

#endif
