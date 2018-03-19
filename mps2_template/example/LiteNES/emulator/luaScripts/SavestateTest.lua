
TESTER = savestate.create();

savestate.save(TESTER);

--FCEU.frameadvance();									

savestate.save(TESTER);

FCEU.frameadvance();

--savestate.load(TESTER);


