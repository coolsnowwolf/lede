// SPDX-License-Identifier: GPL-2.0+
/*
 *  inputbox.c -- implements the input box
 *
 *  ORIGINAL AUTHOR: Savio Lam (lam836@cs.cuhk.hk)
 *  MODIFIED FOR LINUX KERNEL CONFIG BY: William Roadcap (roadcap@cfw.com)
 */

#include "dialog.h"

char dialog_input_result[MAX_LEN + 1];

/*
 *  Print the termination buttons
 */
static void print_buttons(WINDOW * dialog, int height, int width, int selected)
{
	int x = width / 2 - 11;
	int y = height - 2;

	print_button(dialog, "  Ok  ", y, x, selected == 0);
	print_button(dialog, " Help ", y, x + 14, selected == 1);

	wmove(dialog, y, x + 1 + 14 * selected);
	wrefresh(dialog);
}

/*
 * Display a dialog box for inputing a string
 */
int dialog_inputbox(const char *title, const char *prompt, int height, int width,
		    const char *init)
{
	int i, x, y, box_y, box_x, box_width;
	int input_x = 0, key = 0, button = -1;
	int show_x, len, pos;
	char *instr = dialog_input_result;
	WINDOW *dialog;

	if (!init)
		instr[0] = '\0';
	else
		strcpy(instr, init);

do_resize:
	if (getmaxy(stdscr) <= (height - INPUTBOX_HEIGTH_MIN))
		return -ERRDISPLAYTOOSMALL;
	if (getmaxx(stdscr) <= (width - INPUTBOX_WIDTH_MIN))
		return -ERRDISPLAYTOOSMALL;

	/* center dialog box on screen */
	x = (getmaxx(stdscr) - width) / 2;
	y = (getmaxy(stdscr) - height) / 2;

	draw_shadow(stdscr, y, x, height, width);

	dialog = newwin(height, width, y, x);
	keypad(dialog, TRUE);

	draw_box(dialog, 0, 0, height, width,
		 dlg.dialog.atr, dlg.border.atr);
	wattrset(dialog, dlg.border.atr);
	mvwaddch(dialog, height - 3, 0, ACS_LTEE);
	for (i = 0; i < width - 2; i++)
		waddch(dialog, ACS_HLINE);
	wattrset(dialog, dlg.dialog.atr);
	waddch(dialog, ACS_RTEE);

	print_title(dialog, title, width);

	wattrset(dialog, dlg.dialog.atr);
	print_autowrap(dialog, prompt, width - 2, 1, 3);

	/* Draw the input field box */
	box_width = width - 6;
	getyx(dialog, y, x);
	box_y = y + 2;
	box_x = (width - box_width) / 2;
	draw_box(dialog, y + 1, box_x - 1, 3, box_width + 2,
		 dlg.dialog.atr, dlg.border.atr);

	print_buttons(dialog, height, width, 0);

	/* Set up the initial value */
	wmove(dialog, box_y, box_x);
	wattrset(dialog, dlg.inputbox.atr);

	len = strlen(instr);
	pos = len;

	if (len >= box_width) {
		show_x = len - box_width + 1;
		input_x = box_width - 1;
		for (i = 0; i < box_width - 1; i++)
			waddch(dialog, instr[show_x + i]);
	} else {
		show_x = 0;
		input_x = len;
		waddstr(dialog, instr);
	}

	wmove(dialog, box_y, box_x + input_x);

	wrefresh(dialog);

	while (key != KEY_ESC) {
		key = wgetch(dialog);

		if (button == -1) {	/* Input box selected */
			switch (key) {
			case TAB:
			case KEY_UP:
			case KEY_DOWN:
				break;
			case KEY_BACKSPACE:
			case 8:   /* ^H */
			case 127: /* ^? */
				if (pos) {
					wattrset(dialog, dlg.inputbox.atr);
					if (input_x == 0) {
						show_x--;
					} else
						input_x--;

					if (pos < len) {
						for (i = pos - 1; i < len; i++) {
							instr[i] = instr[i+1];
						}
					}

					pos--;
					len--;
					instr[len] = '\0';
					wmove(dialog, box_y, box_x);
					for (i = 0; i < box_width; i++) {
						if (!instr[show_x + i]) {
							waddch(dialog, ' ');
							break;
						}
						waddch(dialog, instr[show_x + i]);
					}
					wmove(dialog, box_y, input_x + box_x);
					wrefresh(dialog);
				}
				continue;
			case KEY_LEFT:
				if (pos > 0) {
					if (input_x > 0) {
						wmove(dialog, box_y, --input_x + box_x);
					} else if (input_x == 0) {
						show_x--;
						wmove(dialog, box_y, box_x);
						for (i = 0; i < box_width; i++) {
							if (!instr[show_x + i]) {
								waddch(dialog, ' ');
								break;
							}
							waddch(dialog, instr[show_x + i]);
						}
						wmove(dialog, box_y, box_x);
					}
					pos--;
				}
				continue;
			case KEY_RIGHT:
				if (pos < len) {
					if (input_x < box_width - 1) {
						wmove(dialog, box_y, ++input_x + box_x);
					} else if (input_x == box_width - 1) {
						show_x++;
						wmove(dialog, box_y, box_x);
						for (i = 0; i < box_width; i++) {
							if (!instr[show_x + i]) {
								waddch(dialog, ' ');
								break;
							}
							waddch(dialog, instr[show_x + i]);
						}
						wmove(dialog, box_y, input_x + box_x);
					}
					pos++;
				}
				continue;
			default:
				if (key < 0x100 && isprint(key)) {
					if (len < MAX_LEN) {
						wattrset(dialog, dlg.inputbox.atr);
						if (pos < len) {
							for (i = len; i > pos; i--)
								instr[i] = instr[i-1];
							instr[pos] = key;
						} else {
							instr[len] = key;
						}
						pos++;
						len++;
						instr[len] = '\0';

						if (input_x == box_width - 1) {
							show_x++;
						} else {
							input_x++;
						}

						wmove(dialog, box_y, box_x);
						for (i = 0; i < box_width; i++) {
							if (!instr[show_x + i]) {
								waddch(dialog, ' ');
								break;
							}
							waddch(dialog, instr[show_x + i]);
						}
						wmove(dialog, box_y, input_x + box_x);
						wrefresh(dialog);
					} else
						flash();	/* Alarm user about overflow */
					continue;
				}
			}
		}
		switch (key) {
		case 'O':
		case 'o':
			delwin(dialog);
			return 0;
		case 'H':
		case 'h':
			delwin(dialog);
			return 1;
		case KEY_UP:
		case KEY_LEFT:
			switch (button) {
			case -1:
				button = 1;	/* Indicates "Help" button is selected */
				print_buttons(dialog, height, width, 1);
				break;
			case 0:
				button = -1;	/* Indicates input box is selected */
				print_buttons(dialog, height, width, 0);
				wmove(dialog, box_y, box_x + input_x);
				wrefresh(dialog);
				break;
			case 1:
				button = 0;	/* Indicates "OK" button is selected */
				print_buttons(dialog, height, width, 0);
				break;
			}
			break;
		case TAB:
		case KEY_DOWN:
		case KEY_RIGHT:
			switch (button) {
			case -1:
				button = 0;	/* Indicates "OK" button is selected */
				print_buttons(dialog, height, width, 0);
				break;
			case 0:
				button = 1;	/* Indicates "Help" button is selected */
				print_buttons(dialog, height, width, 1);
				break;
			case 1:
				button = -1;	/* Indicates input box is selected */
				print_buttons(dialog, height, width, 0);
				wmove(dialog, box_y, box_x + input_x);
				wrefresh(dialog);
				break;
			}
			break;
		case ' ':
		case '\n':
			delwin(dialog);
			return (button == -1 ? 0 : button);
		case 'X':
		case 'x':
			key = KEY_ESC;
			break;
		case KEY_ESC:
			key = on_key_esc(dialog);
			break;
		case KEY_RESIZE:
			delwin(dialog);
			on_key_resize();
			goto do_resize;
		}
	}

	delwin(dialog);
	return KEY_ESC;		/* ESC pressed */
}
