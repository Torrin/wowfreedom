// This is where scripts' loading functions should be declared:
void AddSC_fnpc_commandscript();
void AddSC_fgobject_commandscript();
void AddSC_fgoto_commandscript();
void AddSC_freedom_commandscript();
void AddSC_fgm_commandscript();
void AddSC_flist_commandscript();
void AddSC_flookup_commandscript();
void AddSC_fmisc_commandscript();
void AddSC_fmodify_commandscript();

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddFreedomScripts()
{
    AddSC_fnpc_commandscript();
    AddSC_fgobject_commandscript();
    AddSC_fgoto_commandscript();
    AddSC_freedom_commandscript();
    AddSC_fgm_commandscript();
    AddSC_flist_commandscript();
    AddSC_flookup_commandscript();
    AddSC_fmisc_commandscript();
    AddSC_fmodify_commandscript();
}
