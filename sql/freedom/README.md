### How to deploy Freedom DB changes
1. Execute all sql scripts under ../sql/freedom/base folder
2. Execute and deploy default TrinityCore's database data for respective databases mentioned in ../sql/freedom/base/freedom_base.sql script file
 * WoW Freedom uses TDB6.04 database version
 * WoW Freedom TC-side database is also updated up to 6.2.4/21742 release tag (https://github.com/TrinityCore/TrinityCore/releases/tag/6.2.4%2F21742)
3. Execute all sql scripts under ../sql/freedom/create folder for respective databases
4. Execute all sql scripts under ../sql/freedom/data folder for respective databases
5. Execute all sql scripts under ../sql/freedom/updates sequentally (by timestamps appended to filenames) for respective databses
6. Execute following scripts under ../sql/freedom folder:
 * freedom_commands.sql for "freedom" database
 * freedom_trinity_string.sql for "wod_world" database 
 * rbac_permissions.sql for "wod_auth" database
 
### Some script descriptions
* customize_charsections.sql : SQL script to unlock and globalize special skins, facials, hairstyle and etc. for character creation, what are mostly used for NPCs
* freedom_commands.sql : Frequently updated SQL script, what contains command help data to be displayed in mini-manager command list
* freedom_trinity_string.sql : Frequently updated SQL script, what contains custom wod_world.trinity_string table data for custom command (such as .freedom <subcommands>) help texts
* rbac_permissions.sql : Contains WoW Freedom-tailored rbac permission data for wod_auth rbac tables, what replaces TC's default permission data
