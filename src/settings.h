#ifndef SETTINGS_H
#define SETTINGS_H

/* this file contains all of the configuration options that will be used internally
    without having to access config.h options directly...
     
    whenever there is a new configuration option added to config.h, it must be
    added here aswell so internal functions can see it     
*/

extern const unsigned int border_px;
extern const unsigned int gap_px;
extern const unsigned int snap;     
extern const int showbar;              
extern const int topbar;                
extern int bh;
extern const int center_title;
extern const unsigned int sys_tray_pinning;
extern const unsigned int sys_tray_on_left;
extern const unsigned int sys_tray_spacing;
extern const int sys_tray_pinning_fail_first;
extern const int show_sys_tray;
extern const unsigned int icon_size;
extern const unsigned int icon_spacing;
extern const float mfact;           
extern const int nmaster;   
extern const int resize_hints;

extern const char *colors[2][3];
extern const char* tags[30];
extern const rule_t rules[12];
extern const layout_t layouts[12];
extern const _key_t keys[100];
extern const button_t buttons[12];

#endif
