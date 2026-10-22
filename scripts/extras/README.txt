Tools that made the pictures and the touch config of 3rdparty/dod13client-extras (Python 3, no packages). The extras are packed
into extras.pk3 by CMake and waf, these scripts are not part of the pack.

fixpal.py <in.bmp> <out.bmp>
    Makes palette index 0 of an 8 bpp bmp the black one (swaps the entries and the pixel indices, the picture is the same).
    btns_main.bmp had the black color at index 238 and a light gray at index 0. The main menu of Xash3D builds an atlas of
    the buttons and fills the free space of it with index 0, so every button got light frames. Run it on any 8 bpp bmp that
    is used this way.

gen_buttons.py <extras>/gfx/shell [preview.png]
    The buttons that the Day of Defeat atlas does not have:
      - new game, hazard course, load game, save game go into the empty cells of btns_main.bmp (atlas cells 0, 2, 4, 12, 13);
      - btn_touch*, btn_gamepad, btn_gyro, btn_favorite, btn_unfavorite, new in Xash3D, are separate 156x78 files.
    The letters are cut out of the labels that exist (wonbtn.py): the advance of every letter is fitted from the known
    texts, the cuts are snapped to the valleys of the column profile. The dim state is 0.4 of the normal one, the pressed
    state is the normal one plus a Gaussian glow, both measured on the original labels. The letter b does not exist in
    the atlas, it is the mirrored d.

gen_heads.py <extras>/gfx/shell
    The banners head_touch, head_touch_buttons, head_touch_options, head_gamepad, head_gyro (headlib.py, the same method
    with the fraktur font of the banners).

gen_touch_icons.py <extras> [sheet.png]
    The textures of the touch buttons, touch/gfx/*.png (128x128, RGBA): a dark olive disc with a light icon. The icons are
    drawn from discs, rings, strokes and polygons, nothing is taken from another project.

gen_touch_cfg.py <extras>
    touch.cfg: the layout of the buttons (a table in the script, the commands of Day of Defeat). It checks that the
    textures exist, that nothing leaves the screen and that the buttons do not overlap.

To add a label: put the text into FILES or ATLAS_CELLS of gen_buttons.py (or into FILES of gen_heads.py) and run it.
The texts of the existing labels are listed in glyphs.py (TEXTS, by atlas cell) and in headlib.py (HEAD_TEXTS).
