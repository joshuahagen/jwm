/* See LICENSE file for copyright and license details. */

#include <X11/XF86keysym.h>
#include "src/layout.h"

/* appearance */
const unsigned int border_px             = 0;        /* border pixel of windows */
const unsigned int gap_px             	 = 5;	     /* gap between windows in tiling mode */
const unsigned int snap    	 	 = 32;       /* snap pixel */
const unsigned int bar_alpha     	 = 200;      /* alpha of bar 255 = no transparency */
const unsigned int border_alpha 	 = OPAQUE;
const int showbar          	 	 = 1;        /* 0 means no bar */
const int topbar           	 	 = 1;        /* 0 means bottom bar */
const int topbar_padding	  	 = 12;       /* default spacing around the bars font */
const int vert_pad			 = 6;		
const int side_pad		         = 10;
const int center_title		 	 = 0;        /* 0 means title is not centered */
const char *fonts[]               	 = { "Hack:size=12" };
const char dmenufont[]            	 = "Hack:size=12";
/* color definitions 			   hex code */
static const char col_gray1[]            = "#363636";
static const char col_gray2[]            = "#292929";
static const char col_gray3[]            = "#bbbbbb";
static const char col_gray5[]		 = "#2F2F2F";
static const char col_blue[]		 = "#3099A1";
static const char col_blue2[]            = "#192841";
static const char col_purple[]     	 = "#af7fcd";
static const char col_purple2[]          = "#221C35";
static const char col_yellow_white[]     = "#E7D7AD";
/* end color definitions */
const unsigned int icon_size      = 16;	    /* icon size */
const unsigned int icon_spacing   = 5;	    /* space between icon and the window title */

/* color schemes, modify this to change most colors */
const char* colors[][3] = {
	/*                 fg         	      bg           border    */
	[SchemeNorm]   = { col_yellow_white,  col_gray1,   col_gray2  },
	[SchemeSel]    = { col_purple, 	      col_gray1,   col_purple },
	[SchemeSelAlt] = { col_yellow_white,  col_gray1,   col_gray1  }
};

const unsigned int alphas[][3] = {
	/*                 fg      bg         border      */
	[SchemeNorm]   = { OPAQUE, bar_alpha, border_alpha },
	[SchemeSel]    = { OPAQUE, bar_alpha, border_alpha },
	[SchemeSelAlt] = { OPAQUE, bar_alpha, border_alpha }
};

/* tagging */
const char *tags[] = { "www", "dev", "chat", "sys", "vms", "stox" };

const rule_t rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class          	instance    title       tags mask     isfloating   monitor */
	{ "Code",         	NULL,	    NULL,	1 << 1,       0,           -1 },
	{ "firefox",  	  	NULL,       NULL,       1,	      0,           -1 },
	{ "java-lang-Thread",   NULL,	    NULL,	1 << 5,       0,           -1 },
	{ "Signal",       	NULL,       NULL,       1 << 2,	      0,	   -1 },
	{ "Virt-manager", 	NULL,	    NULL,       1 << 4,	      0,	   -1 },
};

/* layout(s) */
const float mfact             = 0.50;   /* factor of master area size [0.05..0.95] */
const int nmaster             = 1;      /* number of clients in master area */
const int resize_hints        = 0;      /* 1 means respect size hints in tiled resizals */
const int lock_full_screen    = 1;      /* 1 will force focus on the fullscreen window */

const layout_t layouts[] = {
	/* symbol       arrange function */
	{ "[]=",      tile    },    /* first entry is default */
	{ "><>",      NULL    },    /* no layout function means floating behavior */
	{ "[M]",      monocle },    /* monocle is a full size window that overlapps */
	{ "HHH",      grid    },    /* grid like layout, will make every window the same size */
	{ "[@]",      spiral  },    /* spiral windows inward */
	{ "[\\]",     dwindle },    /* dwindle windows similar to bspwm's default tiling mechanism */
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggle_view,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggle_tag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_blue2, "-sf", col_purple, NULL };
static const char *termcmd[]  = { "st", NULL };

/* keyboard hotkeys */
const _key_t keys[] = {
	/* modifier                     key			   function        	argument */
	{ MODKEY,                       XK_p,			   spawn,          	{.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return,		   spawn,          	{.v = termcmd } },
	{ MODKEY,                       XK_b,			   toggle_bar,      	{0} },
	{ MODKEY,                       XK_j,			   focus_stack,     	{.i = +1 } },
	{ MODKEY,                       XK_k,			   focus_stack,     	{.i = -1 } },
	{ MODKEY,                       XK_i,			   inc_n_master,     	{.i = +1 } },
	{ MODKEY,                       XK_d,			   inc_n_master,     	{.i = -1 } },
	{ MODKEY,                       XK_h,			   set_m_fact,       	{.f = -0.05} },
	{ MODKEY,                       XK_l,			   set_m_fact,       	{.f = +0.05} },
	{ MODKEY,                       XK_Return,		   zoom,           	{0} },
	{ MODKEY,                       XK_Tab,			   view,           	{0} },
	{ MODKEY|ShiftMask,             XK_c,			   kill_client,     	{0} },
	{ MODKEY,                       XK_t,			   set_layout,      	{.v = &layouts[0]} },
	{ MODKEY,                       XK_f,			   set_layout,      	{.v = &layouts[1]} },
	{ MODKEY,                       XK_m,			   set_layout,      	{.v = &layouts[2]} },
	{ MODKEY,			XK_g,			   set_layout,	   	{.v = &layouts[3]} },
	{ MODKEY|ShiftMask,	        XK_f,			   fullscreen,	   	{0} },
	{ MODKEY,                       XK_space,		   set_layout,      	{0} },
	{ MODKEY,			XK_r,			   set_layout,          {.v = &layouts[4]} },
	{ MODKEY|ShiftMask,	        XK_r,			   set_layout,		{.v = &layouts[5]} },
	{ MODKEY|ShiftMask,             XK_space,		   toggle_floating,	{0} },
	{ MODKEY,                       XK_0,			   view,            	{.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,			   tag,            	{.ui = ~0 } },
	{ MODKEY,                       XK_comma,		   focus_mon,       	{.i = -1 } },
	{ MODKEY,                       XK_period,		   focus_mon,       	{.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,		   tag_mon,         	{.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,		   tag_mon,         	{.i = +1 } },
	{ MODKEY|ShiftMask,             XK_F1,                     spawn,          	SHCMD("~/.local/bin/shutdown") },
        { MODKEY|ShiftMask,             XK_F5,                     spawn,          	SHCMD("~/.local/bin/dmenu_set_wallpaper") },
	{ MODKEY|ShiftMask,		XK_F6,			   spawn,               SHCMD("~/.local/bin/dmenu_dotfiles") },
	{ MODKEY|ShiftMask,		XK_F7,			   spawn,		SHCMD("~/.local/bin/dmenu_pass") },
	{ MODKEY|ShiftMask,		XK_F8,			   spawn,		SHCMD("~/.local/bin/copy_github_oath") },
	{ MODKEY|ShiftMask,             XK_w,                      spawn,		SHCMD("firefox-bin") },
	{ MODKEY|ShiftMask,		XK_h,			   spawn,		{.v = (const char*[]){ "st", "-e", "htop", NULL } } },
	{ MODKEY|ShiftMask,   		XK_l,			   spawn,		SHCMD("libreoffice") },
        { MODKEY|ShiftMask,		XK_r,			   spawn,		{.v = (const char*[]){ "st", "-e", "ranger", NULL } } },
	{ MODKEY|ShiftMask,             XK_s,     	           spawn,		SHCMD("signal-desktop") },
    	{ MODKEY|ShiftMask,             XK_t,                      spawn,		SHCMD("~/.local/bin/launch_tos") },
    	{ MODKEY,                       XK_Print,                  spawn,		SHCMD("~/.local/bin/dmenu_ss_all") },
    	{ MODKEY,                       XK_s,                      spawn,		SHCMD("~/.local/bin/dmenu_ss_region") },
	{ 0,                            XF86XK_AudioRaiseVolume,   spawn,		SHCMD("amixer set Master 3%+; kill -44 $(pidof dwmblocks)") },
        { 0,                            XF86XK_AudioLowerVolume,   spawn,		SHCMD("amixer set Master 3%-; kill -44 $(pidof dwmblocks)") },
	{ 0,				XF86XK_AudioMute,	   spawn,               SHCMD("amixer set Master toggle; kill -44 $(pidof dwmblocks)") },
	{ 0,                            XF86XK_AudioNext,          spawn,		SHCMD("playerctl next smplayer") },
        { 0,                            XF86XK_AudioPrev,          spawn,		SHCMD("playerctl previous smplayer") },
        { 0,                            XF86XK_AudioStop,          spawn,		SHCMD("playerctl pause smplayer") },
        { 0,                            XF86XK_AudioPlay,          spawn,		SHCMD("playerctl play smplayer") },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      		   quit,           	{0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
const button_t buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        set_layout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        set_layout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,            {0} },
	{ ClkStatusText,	0,		Button2,	spawn,		 {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        window_move_mouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        toggle_floating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        window_resize_mouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,            {0} },
	{ ClkTagBar,            0,              Button3,        toggle_view,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,             {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggle_tag,      {0} },
};
