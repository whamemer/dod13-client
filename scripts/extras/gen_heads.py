"""Makes the banners that the Xash3D menus have and the Day of Defeat set does not (touch, gamepad, gyroscope)."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from headlib import HeadLibrary
from wonbtn import write_gray_bmp

shell = sys.argv[1]
FILES = {'head_touch': 'touch', 'head_touch_buttons': 'touch buttons', 'head_touch_options': 'touch options',
         'head_gamepad': 'gamepad', 'head_gyro': 'gyroscope'}
lib = HeadLibrary(shell)
for name, text in FILES.items():
    write_gray_bmp(os.path.join(shell, name + '.bmp'), lib.compose(text))
print('done:', len(FILES), 'banners')
