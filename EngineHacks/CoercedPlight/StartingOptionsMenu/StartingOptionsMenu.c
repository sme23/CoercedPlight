
#include "StartingOptionsMenu.h"

// lovingly borrowed from circles' self-rando
static const ProcCode RandomOptionsProc[] = {
  PROC_SET_NAME("StartingOptions"),
  PROC_CALL_ROUTINE(LockGameLogic),
  PROC_END_ALL(0x8a20b1c),
  // PROC_END_ALL(0x8a206a8), //savemenu drawing

  PROC_CALL_ROUTINE(StartingOptionsSetup),
  // PROC_CALL_ROUTINE(0x80b1a09), //original config drawing

    PROC_CALL_ROUTINE_ARG(NewFadeIn, 8),
    PROC_WHILE_ROUTINE(FadeInExists),
    PROC_SLEEP(1),

  PROC_LOOP_ROUTINE(StartingOptionsLoop), //wait for B button
  
  PROC_CALL_ROUTINE_ARG(NewFadeOut, 0x10),
  PROC_WHILE_ROUTINE(FadeOutExists),
  PROC_SLEEP(10),

  PROC_END
};

static const ProcCode SpinProc[] = {
  PROC_SET_NAME("SpinnyBoi"),
  PROC_SET_MARK(0xD),
  PROC_CALL_ROUTINE(&SpinRoutine1),
  PROC_LOOP_ROUTINE(&SpinRoutine2),
  PROC_END
};

static const ProcCode NewGameDifficultySelect[] = {
  PROC_SET_NAME("DifficultySelect"),

  PROC_SET_DESTRUCTOR(0x80ac078+1),
  PROC_CALL_ROUTINE(0x80ad5b4+1),
  PROC_YIELD,
  PROC_CALL_ROUTINE(0x80ac1a8+1),
  PROC_SLEEP(1),
  PROC_CALL_ROUTINE(EnableAllGfx),//EnableAllGfx
  PROC_CALL_ROUTINE_ARG(NewFadeIn, 8),
  PROC_WHILE_ROUTINE(FadeInExists),
  PROC_LABEL(0),
  PROC_LOOP_ROUTINE(0x80ac288+1),
  PROC_LABEL(1),
    PROC_CALL_ROUTINE_ARG(NewFadeOut, 8),
    PROC_WHILE_ROUTINE(FadeOutExists),
    PROC_SLEEP(10),
    
      PROC_NEW_CHILD(SpinProc), //one spinny boi

    PROC_NEW_CHILD_BLOCKING(RandomOptionsProc),

    // PROC_NEW_CHILD_BLOCKING(0x8a2ece0), //config proc
    PROC_SLEEP(10),

  PROC_LABEL(2),
  PROC_CALL_ROUTINE_ARG(NewFadeOut, 8),
  PROC_WHILE_ROUTINE(FadeOutExists),
  PROC_CALL_ROUTINE(nullsub_64),

  PROC_YIELD,

  PROC_CALL_ROUTINE(0x80a8c2c+1),
  PROC_YIELD,
  PROC_CALL_ROUTINE(0x80a8cd4+1),
  PROC_CALL_ROUTINE(0x80a8f04+1),
  PROC_YIELD,
  PROC_CALL_ROUTINE(UnlockGameLogic),
  PROC_END
};


static const LocationTable CursorLocationTable[] = {
  {10, 0x18},
  {10, 0x28},
  {10, 0x38},
  {10, 0x48},
  {10, 0x58},
  {10, 0x68},
  {10, 0x78} //,
  // {10, 0x88} //leave room for a description?
};

//POIN to this at $a20164
void NewDifficultySelectFunc(ProcState* input){
  ProcStartBlocking(NewGameDifficultySelect, input);
};

void StartingOptionsSetup(OptionsProc* CurrentProc){
	//set up bg graphics
	ClearBG0BG1();
	EnableBgSyncByIndex(0);
	CpuSet(0x859ED70, (0x020228A8 + 16 * 0x20), 0x20); //ui palette


	CpuSet(0x8b1754c+0x20, (0x020228A8 + 8 * 0x20), 0x20); //bg palette

	VBlankIntrWait();
	LZ77UnCompVram(0x8b12db4, 0x6003000); //bg (changed from 0x6008000)
	GenerateBGTsa((u16*)BG1Buffer, 0x280, 8, 0x180); //was BG1Buffer
	FillBgMap(0x6006000, 0); //clear bg0 tilemap
	FillBgMap(0x6006800, 0); //clear bg1 tilemap
	VBlankIntrWait();
	*gColorSpecialEffectsSelectionBuffer = 0xA44; //blending set
	*gBg1ControlBuffer = 0xD03; //priority set

	//Load fonts
	// SetupDebugFontForBG(2, 0);
	// SetupDebugFontForOBJ(-1, 14);
	// SetupDebugFontForOBJ(0x6017800, 14);
	InitDefaultFont();
	// InitText(0, 0);

	//set up cursor
	CurrentProc->CursorIndex = 0;
	CurrentProc->Page = 1;
	CurrentProc->CasualMode = 0;
	CurrentProc->FixedGrowths = 0;
	CurrentProc->RN = 0;

	updateOptionsPage(CurrentProc);
};

void GenerateBGTsa(u16 *MapOffset, u32 NumberOfTiles, u8 PaletteId, u16 baseTile) {
  for(u16 i = baseTile; i < (baseTile+NumberOfTiles/2)+1; i++) {
    MapOffset[i-baseTile] = (i | (PaletteId << 12));
    MapOffset[NumberOfTiles-(i-baseTile)] = (i | (PaletteId << 12) | (3<<10)); //v and h flipped
  }
}

void updateOptionsPage(OptionsProc* CurrentProc) {

	//clear bg font buffers
	Text_ResetTileAllocation();
	// ClearBG0BG1();
	FillBgMap(BG0Buffer, 0);
	EnableBgSyncByIndex(0);
	//Print Headings
	DrawTextInline(0, BGLoc(BG0Buffer, 2, 0), 4, 0, 8, "Settings");
	//DrawTextInline(0, BGLoc(BG0Buffer, 19, 0), 4, 0, 8, "<< L/R >>");

	if (thisPage == 1){
		//option names
		DrawTextInline(0, BGLoc(BG0Buffer, 2, 3), 3, 0, 7, "Casual Mode:");
		DrawTextInline(0, BGLoc(BG0Buffer, 2, 5), 3, 0, 8, "Fixed Growths:");
		DrawTextInline(0, BGLoc(BG0Buffer, 2, 7), 3, 0, 7, "Hit RNG:");
		// DrawTextInline(0, BGLoc(BG0Buffer, 2, 9), 3, 0, 10, "Peak/Water Units:");
		// DrawTextInline(0, BGLoc(BG0Buffer, 2, 11), 3, 0, 8, "Weapon Stats:");
		// DrawTextInline(0, BGLoc(BG0Buffer, 2, 13), 3, 0, 8, "Random Items:");
		// DrawTextInline(0, BGLoc(BG0Buffer, 2, 15), 3, 0, 5, "Mode:");
		// DrawTextInline(0, BGLoc(BG0Buffer, 2, 17), 3, 0, 7, "Map Music:");


		//option values
		
		if (CurrentProc->CasualMode == 0) { 
			DrawTextInline(0, BGLoc(BG0Buffer, 15, 3), 2, 0, 5, "Off"); 
			UnsetEventId(CasualModeFlagLink);
		}
		else if (CurrentProc->CasualMode == 1) { 
			DrawTextInline(0, BGLoc(BG0Buffer, 15, 3), 2, 0, 5, "On"); 
			SetEventId(CasualModeFlagLink);
		}
		
		
		if (CurrentProc->FixedGrowths == 0) { 
			DrawTextInline(0, BGLoc(BG0Buffer, 15, 5), 2, 0, 10, "Off"); 
			UnsetEventId(FixedGrowthsFlagLink);
		}
		else if (CurrentProc->FixedGrowths == 1) { 
			DrawTextInline(0, BGLoc(BG0Buffer, 15, 5), 2, 0, 10, "On"); 
			SetEventId(FixedGrowthsFlagLink);
		}
		
		switch (CurrentProc->RN) {

			case 0:
			DrawTextInline(0, BGLoc(BG0Buffer, 15, 7), 2, 0, 10, "True Hit");
			UnsetEventId(OneRNFlagLink);
			UnsetEventId(FatesRNFlagLink);
			UnsetEventId(EvilRNFlagLink);
			break;
			
			case 1:
			DrawTextInline(0, BGLoc(BG0Buffer, 15, 7), 2, 0, 10, "Display Hit");
			SetEventId(OneRNFlagLink);
			UnsetEventId(FatesRNFlagLink);
			UnsetEventId(EvilRNFlagLink);
			break;
			
			case 2:
			DrawTextInline(0, BGLoc(BG0Buffer, 15, 7), 2, 0, 10, "Fates Hit");
			UnsetEventId(OneRNFlagLink);
			SetEventId(FatesRNFlagLink);
			UnsetEventId(EvilRNFlagLink);
			break;
			
			case 3:
			DrawTextInline(0, BGLoc(BG0Buffer, 15, 7), 2, 0, 10, "False Hit");
			UnsetEventId(OneRNFlagLink);
			UnsetEventId(FatesRNFlagLink);
			SetEventId(EvilRNFlagLink);
			break;
			
			default:
			break;

		}
	}
	
 };


void StartingOptionsLoop(OptionsProc* CurrentProc){

	//make snowflakes white
	*(u32*) 0x5000262 = 0x739eFFFF; //fill in the only obj palette colour that matters lol
	
	// UpdateBG3HOffset();
	UpdateHandCursor(CursorLocationTable[CurrentProc->CursorIndex].x, CursorLocationTable[CurrentProc->CursorIndex].y);	
	if (((newInput & InputStart) != 0)|((newInput & InputA) != 0)) { //press A or Start to continue
		BreakProcLoop((Proc *)CurrentProc);
		m4aSongNumStart(0x6B); 
	};


  if (thisPage == 1) {
    if ((newInput & InputDown) != 0) {
      if (CurrentProc->CursorIndex < PAGE1MAXINDEX) CurrentProc->CursorIndex++;
      else CurrentProc->CursorIndex = 0;
    };
    if ((newInput & InputUp) != 0) {
      if (CurrentProc->CursorIndex != 0) CurrentProc->CursorIndex--;
      else CurrentProc->CursorIndex = PAGE1MAXINDEX;
    };
    //CasualMode
    if (CurrentProc->CursorIndex == 0) {
      if (((newInput & InputLeft) != 0)|((newInput & InputRight) != 0)) {
		  if (CurrentProc->CasualMode == 0) CurrentProc->CasualMode = 1;
		  else CurrentProc->CasualMode = 0;
		updateOptionsPage(CurrentProc);
	}}
	//FixedGrowths
	if (CurrentProc->CursorIndex == 1) {
      if (((newInput & InputLeft) != 0)|((newInput & InputRight) != 0)) {
		  if (CurrentProc->FixedGrowths == 0) CurrentProc->FixedGrowths = 1;
		  else CurrentProc->FixedGrowths = 0;
		updateOptionsPage(CurrentProc);
	}};
		
	//RN
	if (CurrentProc->CursorIndex == 2) {
		if ((newInput & InputLeft) != 0) {
			if (CurrentProc->RN == 0) CurrentProc->RN = 3;
			else CurrentProc->RN --;
			updateOptionsPage(CurrentProc);
		}
		if ((newInput & InputRight) != 0) {
			if (CurrentProc->RN == 3) CurrentProc->RN = 0;
			else CurrentProc->RN ++;
			updateOptionsPage(CurrentProc);
		}}
		
		
	};

};


