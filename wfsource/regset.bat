rem regset.bat: set registry entries from enviroment variables
bin\reg "SOFTWARE\World Foundry\GDK\WORLD_FOUNDRY_DIR" "%WF_DIR"
bin\reg "SOFTWARE\World Foundry\GDK\LEVELS_DIR" "%LEVEL_DIR"
bin\reg "SOFTWARE\World Foundry\GDK\OAD_DIR" "%OAD_DIR"
