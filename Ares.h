#ifndef ARES_H
#define ARES_H

#include <hash_map>
#include <YRPP.h>
#include <MacroHelpers.h>

class Ares
{
public:
	//Global Options
	static HANDLE	hInstance;

	static bool		bNoLogo;
	static bool		bNoCD;

	static bool		bLog;
	static FILE*	pLogFile;

	static DWORD readLength;
	static char readBuffer[BUFLEN];
	static char readDelims[4];
	static char readDefval[4];

	//Callbacks
	static eMouseEventFlags __stdcall MouseEvent(Point2D*,eMouseEventFlags);
	static void __stdcall CmdLineParse(char**,int);

	static void __stdcall ExeRun();
	static void __stdcall ExeTerminate();

	static void __stdcall PostGameInit();

	static void __stdcall RegisterCommands();

	//General functions
	static void SendPDPlane(
		HouseClass* pOwner,
		CellClass* pDestination,
		AircraftTypeClass* pPlaneType,
		TypeList<TechnoTypeClass*>* pTypes,
		TypeList<int>* pNums);
};

#endif
