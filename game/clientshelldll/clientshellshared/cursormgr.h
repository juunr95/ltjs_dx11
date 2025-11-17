// ----------------------------------------------------------------------- //
//
// MODULE  : CursorMgr.h
//
// PURPOSE : Manage all mouse cursor related functionality
//
// CREATED : 12/3/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CURSOR_MGR_H__
#define __CURSOR_MGR_H__

#include "iltcursor.h"
#include "screenspritemgr.h"


class CCursorMgr;
extern CCursorMgr* g_pCursorMgr;

class CCursorMgr
{
public:
	CCursorMgr();
	~CCursorMgr();

	LTBOOL		Init();
	void		Term();

	void		ScheduleReinit(float fDelay);
	void		CheckForReinit();
	
	void		UseHardwareCursor(LTBOOL bUseHardwareCursor, bool bForce = false);
    void		UseCursor(LTBOOL bUseCursor, LTBOOL bLockCursorToCenter = LTFALSE);
	void		Update();

	void		SetCenter(int x, int y);

	// TODO at some point in the future, we can allow multiple sprites and other FX here.

private:

	LTBOOL		m_bInitialized;
    LTBOOL      m_bUseCursor;
    LTBOOL      m_bUseHardwareCursor;

	LTIntPt		m_CursorCenter;

	HTEXTURE    m_hTexCursor;
	LTPoly_GT4  m_PolyCursor;
};

#endif // __CURSOR_MGR_H__