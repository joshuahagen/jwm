#include "dwm.h"
#include "helpers.h"
#include "keyboard.h"
#include "settings.h"

unsigned int num_lock_mask = 0;

void grab_keys(void)
{
	update_num_lock_mask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, num_lock_mask, num_lock_mask|LockMask };
		KeyCode code;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		for (i = 0; i < LENGTH(keys); i++)
			if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
						True, GrabModeAsync, GrabModeAsync);
	}
}

void update_num_lock_mask(void)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	num_lock_mask = 0;
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(dpy, XK_Num_Lock))
				num_lock_mask = (1 << i);
	XFreeModifiermap(modmap);
}