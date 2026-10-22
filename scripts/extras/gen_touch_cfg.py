"""Writes touch.cfg, the default layout of the touch buttons of Day of Defeat for Xash3D.
The buttons are squares for a 16:9 screen (the engine keeps them square with the aspect value of touch_addbutton).
usage: gen_touch_cfg.py <extras dir>"""
import sys, os

H = 16.0 / 9.0
SQ = 0.08            # the width of a small button

# name, texture, command, x, y, width, flags
BUTTONS = [
    # weapons
    ('slot1', 'knife', 'slot1', 0.00, 0.00, SQ, 0),
    ('slot2', 'pistol', 'slot2', 0.08, 0.00, SQ, 0),
    ('slot3', 'rifle', 'slot3', 0.16, 0.00, SQ, 0),
    ('slot5', 'grenade', 'slot5', 0.24, 0.00, SQ, 0),
    ('invprev', 'prev', 'invprev', 0.32, 0.00, SQ, 0),
    ('invnext', 'next', 'invnext', 0.40, 0.00, SQ, 0),
    # team, class, chat, scores
    ('changeclass', 'changeclass', 'changeclass', 0.52, 0.00, SQ, 0),
    ('changeteam', 'changeteam', 'changeteam', 0.60, 0.00, SQ, 0),
    ('signals', 'signals', 'voice_menu1', 0.68, 0.00, SQ, 0),
    ('chat', 'chat', 'messagemode', 0.76, 0.00, SQ, 0),
    ('chat_team', 'chat_team', 'messagemode2', 0.84, 0.00, SQ, 0),
    ('scores', 'score', '+showscores', 0.92, 0.00, SQ, 0),
    # map and menus
    ('map', 'map', '+map', 0.64, 0.16, SQ, 0),
    ('mapzoom', 'mapzoom', '+mapzoom', 0.72, 0.16, SQ, 0),
    ('commandmenu', 'cmdmenu', '+commandmenu', 0.80, 0.16, SQ, 0),
    # right hand
    ('jump', 'jump', '+jump', 0.88, 0.16, 0.12, 0),
    ('use', 'use', '+use', 0.88, 0.40, 0.12, 0),
    ('prone', 'prone', 'prone', 0.92, 0.64, SQ, 0),
    ('duck', 'duck', '+duck', 0.87, 0.785, 0.11, 0),
    ('attack', 'attack', '+attack', 0.72, 0.60, 0.14, 0),
    ('attack2', 'attack2', '+attack2', 0.78, 0.40, 0.10, 0),
    ('reload', 'reload', '+reload', 0.66, 0.40, 0.10, 0),
    # the left bottom corner
    ('menu', 'menu', 'escape', 0.00, 0.85, SQ, 0),
    ('edit', 'settings', 'touch_enableedit', 0.08, 0.85, SQ, 32),
    ('voice', 'voice', '+voicerecord', 0.16, 0.85, SQ, 0),
    ('drop', 'drop', 'drop', 0.24, 0.85, SQ, 0),
]

HEADER = '''//=======================================================================
//	Day of Defeat client, touchscreen config
//	made by scripts/extras/gen_touch_cfg.py, the textures are in touch/gfx
//=======================================================================

touch_config_file "touch.cfg"

// touch cvars

// sensitivity settings
touch_pitch "90.000000"
touch_yaw "120.000000"
touch_forwardzone "0.060000"
touch_sidezone "0.060000"
touch_nonlinear_look "0"
touch_pow_factor "1.300000"
touch_pow_mult "400.000000"
touch_exp_mult "0.000000"

// grid settings
touch_grid_count "50"
touch_grid_enable "1"

// global overstroke (width, r, g, b, a)
touch_set_stroke 1 255 255 255 255

// highlight when pressed
touch_highlight_r "1.000000"
touch_highlight_g "1.000000"
touch_highlight_b "1.000000"
touch_highlight_a "1.000000"

// _joy and _dpad options
touch_dpad_radius "1.000000"
touch_joy_radius "1.000000"

// how much slowdown when Precise Look button pressed
touch_precise_amount "0.500000"

// enable/disable move indicator
touch_move_indicator "0.000000"

// reset menu state when execing config
touch_setclientonly 0

// touch buttons
touch_removeall
touch_addbutton "move" "" "_move" 0.000000 0.106667 0.500000 0.995556 255 255 255 150 0
touch_addbutton "look" "" "_look" 0.500000 0.106667 1.000000 0.995556 255 255 255 150 0
'''


def main():
    root = sys.argv[1]
    lines = [HEADER.rstrip('\n')]
    rects = []
    for name, tex, cmd, x, y, w, flags in BUTTONS:
        h = w * H
        x2, y2 = x + w, y + h
        assert x2 <= 1.0001 and y2 <= 1.0001, (name, x2, y2)
        assert os.path.exists(os.path.join(root, 'touch', 'gfx', tex + '.png')), tex
        for n2, a in rects:
            assert x2 <= a[0] + 1e-6 or x >= a[2] - 1e-6 or y2 <= a[1] + 1e-6 or y >= a[3] - 1e-6, 'overlap %s %s' % (name, n2)
        rects.append((name, (x, y, x2, y2)))
        lines.append('touch_addbutton "%s" "touch/gfx/%s" "%s" %.6f %.6f %.6f %.6f 255 255 255 150 %d 1' % (name, tex, cmd, x, y, x2, y2, flags))
    open(os.path.join(root, 'touch.cfg'), 'w', newline='\n').write('\n'.join(lines) + '\n')
    print('buttons:', len(BUTTONS))


if __name__ == '__main__':
    main()
