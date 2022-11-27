#include "dwm.h"
#include "helpers.h"
#include "keyboard.h"
#include "mouse.h"
#include "settings.h"

int get_root_ptr(int *x, int *y)
{
	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

void grab_buttons(client_t *c, int focused)
{
	update_num_lock_mask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, num_lock_mask, num_lock_mask|LockMask };
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if (!focused)
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
				BUTTON_MASK, GrabModeSync, GrabModeSync, None, None);
				
		for (i = 0; i < LENGTH(buttons); i++)
			if (buttons[i].click == ClkClientWin)
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabButton(dpy, buttons[i].button,
						buttons[i].mask | modifiers[j],
						c->win, False, BUTTON_MASK,
						GrabModeAsync, GrabModeSync, None, None);
	}
}