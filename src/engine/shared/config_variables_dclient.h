
// All DClient variables are defined here

MACRO_CONFIG_INT(DcShowFlags, dc_show_flags, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Tee stats (Deep/Jetpack/etc)")
MACRO_CONFIG_INT(DcShowFlagsSize, dc_show_flags_size, 30, -50, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Size of tee stat indicators")
MACRO_CONFIG_INT(DcGrenadePath, dc_grenade_path, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Grenade path prediction")
MACRO_CONFIG_INT(DcLaserPath, dc_laser_path, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Laser path prediction")
MACRO_CONFIG_INT(DcShowDJ, dc_show_jumps, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show remaining double jumps of a tee")
MACRO_CONFIG_INT(DcShowJumpsSize, dc_show_jumps_size, 30, -50, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Size of double jump indicators")
MACRO_CONFIG_INT(DcFreeMouse, dc_free_mouse, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Free mouse mode (WIP)")

MACRO_CONFIG_INT(DcUnlockZoom, dc_unlock_zoom, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Disable camera zoom lock")
MACRO_CONFIG_INT(DcFogOfWar, dc_fow, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Don't render anything beyond possible vision range when zoom is unlocked")
MACRO_CONFIG_COL(DcFogOfWarColor, dc_fow_color, 0x7F000000, CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_COLALPHA | CFGFLAG_INSENSITIVE, "Fog of war color")
